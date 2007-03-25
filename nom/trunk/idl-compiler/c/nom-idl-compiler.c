/* ***** BEGIN LICENSE BLOCK *****
* Version: CDDL 1.0/LGPL 2.1
*
* The contents of this file are subject to the COMMON DEVELOPMENT AND
* DISTRIBUTION LICENSE (CDDL) Version 1.0 (the "License"); you may not use
* this file except in compliance with the License. You may obtain a copy of
* the License at http://www.sun.com/cddl/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is "NOM" Netlabs Object Model
*
* The Initial Developer of the Original Code is
* netlabs.org: Chris Wohlgemuth <cinc-ml@netlabs.org>.
* Portions created by the Initial Developer are Copyright (C) 2007
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
* case the provisions of the LGPL are applicable instead of those above. If
* you wish to allow use of your version of this file only under the terms of
* the LGPL, and not to allow others to use your version of this file under
* the terms of the CDDL, indicate your decision by deleting the provisions
* above and replace them with the notice and other provisions required by the
* LGPL. If you do not delete the provisions above, a recipient may use your
* version of this file under the terms of any one of the CDDL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */
#define INCL_DOSPROCESS
#define INCL_DOS
#define INCL_DOSPROFILE
#define INCL_DOSERRORS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

#include <glib.h> 
#include <glib/gprintf.h>

#include "nom.h"
#include "nomtk.h"

#include "parser.h"

static gchar* chrOutputDir="";
static gchar* chrOutputName="";
static gboolean fOptionEmitH=FALSE;
static gboolean fOptionEmitIH=FALSE;
static gboolean fOptionEmitC=FALSE;

/* Command line options */
static GOptionEntry gOptionEntries[] = 
{
  {"directory", 'd', 0, G_OPTION_ARG_FILENAME, &chrOutputDir, "Output directory", NULL},
  {"emit-h", 0, 0, G_OPTION_ARG_NONE, &fOptionEmitH, "Emmit a header file (*.h)", NULL},
  {"emit-ih", 0, 0, G_OPTION_ARG_NONE, &fOptionEmitIH, "Emmit an include header (*.ih)", NULL},
  {"emit-c", 0, 0, G_OPTION_ARG_NONE, &fOptionEmitC, "Emmit an implementation template (*.c)", NULL},
  {"output", 'o', 0, G_OPTION_ARG_FILENAME, &chrOutputName, "Output name. Must not be omitted.", NULL},
  {NULL}
};


/* The pointer array holding the interfaces we found */
GPtrArray* pInterfaceArray;

/* Symbols defined for our IDL language.
   Keep this in synch with the defined enums! */
SYMBOL idlSymbols[]={
  {"interface", IDL_SYMBOL_INTERFACE, KIND_UNKNOWN}, /* 71 */
  {"NOMCLASSVERSION", IDL_SYMBOL_CLSVERSION, KIND_UNKNOWN},
  {"NOMINSTANCEVAR", IDL_SYMBOL_INSTANCEVAR, KIND_UNKNOWN},
  {"NOMOVERRIDE", IDL_SYMBOL_OVERRIDE, KIND_UNKNOWN},
  {"NOMREGISTEREDIFACE", IDL_SYMBOL_REGINTERFACE, KIND_TYPESPEC},
  {"NOMCLASSNAME", IDL_SYMBOL_CLSNAME, KIND_UNKNOWN},
  {"NOMMETACLASS", IDL_SYMBOL_OLDMETACLASS, KIND_UNKNOWN},
  {"MetaClass", IDL_SYMBOL_METACLASS, KIND_UNKNOWN},
  {"native", IDL_SYMBOL_NATIVE, KIND_UNKNOWN},
  {"gulong", IDL_SYMBOL_GULONG, KIND_TYPESPEC},
  {"gint", IDL_SYMBOL_GINT, KIND_TYPESPEC},
  {"gpointer", IDL_SYMBOL_GPOINTER, KIND_TYPESPEC},
  {"gboolean", IDL_SYMBOL_GBOOLEAN, KIND_TYPESPEC},
  {"gchar", IDL_SYMBOL_GCHAR, KIND_TYPESPEC},
  {"void", IDL_SYMBOL_VOID, KIND_TYPESPEC},

  {"boolean", IDL_SYMBOL_BOOLEAN, KIND_TYPESPEC},
  {"string", IDL_SYMBOL_STRING, KIND_TYPESPEC},
  {"long", IDL_SYMBOL_LONG, KIND_TYPESPEC},
  {"unsigned", IDL_SYMBOL_LONG, KIND_TYPESPEC},

  {"in", IDL_SYMBOL_IN, KIND_DIRECTION},
  {"out", IDL_SYMBOL_OUT, KIND_DIRECTION},
  {"inout", IDL_SYMBOL_INOUT, KIND_DIRECTION},
  {"define", IDL_SYMBOL_DEFINE, KIND_UNKNOWN},
  {"ifdef", IDL_SYMBOL_IFDEF, KIND_UNKNOWN},
  {"endif", IDL_SYMBOL_ENDIF, KIND_UNKNOWN},
  {NULL, 0, KIND_UNKNOWN}
},*pSymbols=idlSymbols;

GScanner *gScanner;

/* Holding info about current token. Referenced by gScanner. */
//SYMBOLINFO curSymbol;

/* Holding the current state of parsing and pointers to necessary lists. */
PARSEINFO parseInfo={0};
PPARSEINFO pParseInfo=&parseInfo;

/**
   Helper function which scans the array of known interfaces and returns the interface
   structure for the given name. 

   \PARAM chrName Name of the interface.
   \Returns If no interface with that name can be found NULL is returned otherwise a
   pointer to the interface structure..
 */
PINTERFACE findInterfaceFromName(gchar* chrName)
{
  int a;

  for(a=0;a<pInterfaceArray->len;a++)
    {
      PINTERFACE pif=g_ptr_array_index(pInterfaceArray, a);
      if(!strcmp(chrName, pif->chrName))
        return pif;
    }

  return NULL;
}

/**
   Helper function which returns a copy of the typespec string of the current token.
   That is e.g. 'gint' or 'gpointer'. Note that this function is only called when the
   current token is indeed a type specification in the IDL file.
 */
gchar* getTypeSpecStringFromCurToken(void)
{
  GTokenValue value;

  value=gScanner->value;

  switch(gScanner->token)
    {
    case G_TOKEN_IDENTIFIER:
      return g_strdup(value.v_identifier);
      break;
    case G_TOKEN_SYMBOL:
      {
        /* It's one of our symbols. */
        PSYMBOL pCurSymbol;

        pCurSymbol=value.v_symbol;
        return g_strdup(pCurSymbol->chrSymbolName);
        break;
      }
    default:
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_SYMBOL,
                            NULL,
                            NULL,
                            NULL,
                            "",
                            TRUE); /* is_error */

      break;
    }
  return "unknown";
}

/**
   This function is only for removing the NOMCLASSNAME() definition from
   the input stream. When everything is moved to the new IDL compiler
   those statements will be removed and this parsing function eventually
   removed.

   The current token is the NOMCLASSNAME keyword.

   CN:= G_TOKEN_SYMBOL '(' INDENT ')' ';'
 */
static void parseClassName(void)
{

  if(!matchNext('('))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            '(',
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMCLASSNAME()",
                            TRUE); /* is_error */
      exit(1);
    }

  /* Identifier. We discard it. */
  if(!matchNext(G_TOKEN_IDENTIFIER))
    {
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_IDENTIFIER,
                            NULL,
                            NULL,
                            NULL,
                            "Class name is not a valid identifier.",
                            TRUE); /* is_error */
      exit(1);
    }

  if(!matchNext(')'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ')',
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMCLASSNAME().",
                            TRUE); /* is_error */
      exit(1);
    }

    if(!matchNext(';'))
      {
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              ';',
                              NULL,
                              NULL,
                              NULL,
                              "Error in NOMCLASSNAME() definition, Missing semicolon at the end.",
                              TRUE); /* is_error */
        exit(1);
      }
}

/**
   This function is only for removing the NOMMETACLASS() definition from
   the input stream. When everything is moved to the new IDL compiler
   those statements will be removed and this parsing function eventually
   removed.

   The current token is the NOMMETACLASS keyword.

   CN:= G_TOKEN_SYMBOL '(' INDENT ')' ';'
 */
static void parseOldMetaClass(void)
{

  if(!matchNext('('))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            '(',
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMMETACLASS()",
                            TRUE); /* is_error */
      exit(1);
    }

  /* Identifier. We discard it. */
  if(!matchNext(G_TOKEN_IDENTIFIER))
    {
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_IDENTIFIER,
                            NULL,
                            NULL,
                            NULL,
                            "Class name is not a valid identifier.",
                            TRUE); /* is_error */
      exit(1);
    }

  if(!matchNext(')'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ')',
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMMETACLASS().",
                            TRUE); /* is_error */
      exit(1);
    }

    if(!matchNext(';'))
      {
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              ';',
                              NULL,
                              NULL,
                              NULL,
                              "Error in NOMMETACLASS() definition, Missing semicolon at the end.",
                              TRUE); /* is_error */
        exit(1);
      }
}

/**
   Parse the declaration of a new type using the 'native' keyword.

  The 'native' keyword is used to introduce new types. That's coming
  from the Corba spec.

  \Remarks  The current token is the 'native' keyword.

  N:= G_TOKEN_SYMBOL IDENT ';'
 */
static void parseNative(void)
{
  GTokenValue value;
  PSYMBOL pCurSymbol=g_malloc0(sizeof(SYMBOL));

  if(!matchNext(G_TOKEN_IDENTIFIER))
    {
      PSYMBOL pSymbol;

      /* Check if it's a symbol. The following 'identifier' (word) is maybe alread
       registered as a symbol. */
      if(!matchNext(G_TOKEN_SYMBOL))
        {
          g_scanner_unexp_token(gScanner,
                                G_TOKEN_SYMBOL,
                                NULL,
                                NULL,
                                NULL,
                                "'native' statement is not followed by a valid identifier.",
                                TRUE); /* is_error */
          exit(1);
        }
      /* It's a symbol. Check if it's a typespec. */
      value=gScanner->value;
      pSymbol=value.v_symbol;
      if(!pSymbol || pSymbol->uiKind!=KIND_TYPESPEC)
        {
          g_scanner_unexp_token(gScanner,
                                G_TOKEN_SYMBOL,
                                NULL,
                                NULL,
                                NULL,
                                "'native' statement is not followed by a valid symbol.",
                                TRUE); /* is_error */
          exit(1);
        }
    }

    value=gScanner->value;
    pCurSymbol->chrSymbolName=g_strdup(value.v_identifier);
    pCurSymbol->uiKind=KIND_TYPESPEC;
    pCurSymbol->uiSymbolToken=G_TOKEN_NONE;
    g_tree_insert(parseInfo.pSymbolTree, pCurSymbol, pCurSymbol->chrSymbolName);
    g_scanner_scope_add_symbol(gScanner, ID_SCOPE, pCurSymbol->chrSymbolName,
                             pCurSymbol);

    if(!matchNext(';'))
      {
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              ';',
                              NULL,
                              NULL,
                              NULL,
                              "Error in 'native' definition , Missing semicolon",
                              TRUE); /* is_error */
        exit(1);
      }

}

/**
   This is the root parse function. Here starts the fun. When a token is found in the
   input stream which matches one of the known token types the respective parsing function
   is called for further processing. In case of an error the parsing function in question 
   prints an error which describes the problem and exits the application.

   This function scans the input until EOF is hit.
 */
void parseIt(void)
{
  while(g_scanner_peek_next_token(gScanner) != G_TOKEN_EOF) {
    GTokenType token;

    g_scanner_get_next_token(gScanner);
    token=gScanner->token;
    GTokenValue value=gScanner->value;
    
    switch(token)
      {
      case '#':
        parseHash();
        break;
      case G_TOKEN_SYMBOL:
        {
          PSYMBOL pCurSymbol=value.v_symbol;
          switch(pCurSymbol->uiSymbolToken)
            {
            case IDL_SYMBOL_INTERFACE:
              parseInterface(token);
              break;
            case IDL_SYMBOL_NATIVE:
              parseNative();
              break;
            case IDL_SYMBOL_CLSNAME:
              parseClassName();
              break;
            case IDL_SYMBOL_OLDMETACLASS:
              parseOldMetaClass();
              break;
            default:
              break;
            }
          break;
        }
#if 0
      case G_TOKEN_IDENTIFIER:
        g_message("Token: %d (G_TOKEN_IDENTIFIER)\t\t%s", token, value.v_identifier);
        break;
      case G_TOKEN_STRING:
        g_message("Token: %d (G_TOKEN_STRING)\t\t\t%s", token, value.v_string);
        break;
      case G_TOKEN_LEFT_PAREN:
        g_message("Token: %d (G_TOKEN_LEFT_PAREN)\t\t(", token);
        break;
      case G_TOKEN_RIGHT_PAREN:
        g_message("Token: %d (G_TOKEN_RIGHT_PAREN)\t\t)", token);
        break;
      case ':':
        g_message("Token: %d (colon)\t\t:", token);
        break;
      case ';':
        g_message("Token: %d (semicolon)\t\t\t;", token);
        break;
      case '/':
        g_message("Token: %d (slash)\t\t\t/ %s", token, value.v_comment);
        break;
      case G_TOKEN_COMMA:
        g_message("Token: %d (G_TOKEN_COMMA)\t\t\t,", token);
        break;
      case G_TOKEN_INT:
        g_message("Token: %d (G_TOKEN_INT)\t\t\t%ld", token, value.v_int);
        break;
      case IDL_SYMBOL_DEFINE:
        g_message("Token: %d (IDL_SYMBOL_DEFINE)\t\t\t", token);
        break;
      case IDL_SYMBOL_IFDEF:
        g_message("Token: %d (IDL_SYMBOL_IFDEF)\t\t\t", token);
        break;
      case IDL_SYMBOL_ENDIF:
        g_message("Token: %d (IDL_SYMBOL_ENDIF)\t\t\t", token);
        break;
#endif
      default:
        printToken(token);
        break;
      }
  }
}

/**
   Support function to show help for the IDL compiler. gContext must be valid.
*/
static void outputCompilerHelp(GOptionContext *gContext, gchar* chrExeName)
{
  GError *gError = NULL;
  int argc2=2;
  char *helpCmd[]={"","--help"};
  char** argv2=helpCmd;
  helpCmd[0]=chrExeName;

  g_printf("An output filename must always be specified. If the name is an absolute path\n\
it will be used unmodified. Otherwise the output name is built from the given\n\
name and the directory specification.\n\n\
-If no directory is specified the output name is built from the current directory\n\
 path and the given filename.\n\
-If the directory is a relative path the output name is built from the current\n\
 directory path, the given directory name (or path) and the filename.\n\
-If the directory is a full path the output name is built from the directory\n\
 path and the given filename.\n\n\
Note that an emitter specific extension will always be appended to the output\n\
filename\n\n");

  /* This prints the standard option help to screen. */  
  g_option_context_parse (gContext, &argc2, &argv2, &gError);
}

/*
  Compare function for the tree holding our private symbols.
 */
static gint funcSymbolCompare(gconstpointer a, gconstpointer b)
{
  if(a==b)
    return 0;

  if(a<b)
    return -1;

  return 1;
};

/**
   Message output handler for the scanner. The default handler isn't used because the preprocessor
   mangles all include files together and thus the line numbers are not as expected by the user.
   This function prints the error messages with corrected linenumbers and the source file name
   in which to find the problem.
   
 */
void funcMsgHandler(GScanner *gScanner, gchar *message, gboolean error)
{
  g_printf("%s:%d: error: %s (%d %d)\n", parseInfo.chrCurrentSourceFile,
           g_scanner_cur_line(gScanner)-parseInfo.uiLineCorrection, message,
           g_scanner_cur_line(gScanner), parseInfo.uiLineCorrection);
}

/**
   Main entry point for the idl compiler.
 */
int main(int argc, char **argv)
{
  int a;
  int fd;
  /* Vars for filename building */
  char* chrOutputFileName="";
  char* chrTemp;

  GError *gError = NULL;
  GOptionContext* gContext;

  /* Parse command line options */
  gContext = g_option_context_new ("file");
  g_option_context_add_main_entries (gContext, gOptionEntries, NULL);

  if(!g_option_context_parse (gContext, &argc, &argv, &gError))
    {
      outputCompilerHelp(gContext, argv[0]);
    }

  /* Correct emitter options? Exactly one emitter must be specified. */
  a=0;
  if(fOptionEmitH)
    a++;
  if(fOptionEmitIH)
    a++;
  if(fOptionEmitC)
    a++;

  if(!a){
    g_printf("An emitter must be specified.\n\n");
    outputCompilerHelp(gContext, argv[0]);
  }
  if(a>1){
    g_printf("Only one emitter must be specified.\n\n");
    outputCompilerHelp(gContext, argv[0]);
  }

  if(strlen(chrOutputName)==0)
    {
      g_printf("No output file name given.\n\n");
      outputCompilerHelp(gContext, argv[0]);
    }
  g_option_context_free(gContext);


  if(argc<2)
    {
      g_printf("No input file name given.\n\nUse %s --help for options.\n\n", argv[0]);
      return 1;
    }

  for(a=0; a<argc; a++)
    {
      g_message("arg %d: %s", a, argv[a]);
    }

  
  /*** Create output file name ****/
  if(!g_path_is_absolute(chrOutputName))
    {
      if(g_path_is_absolute(chrOutputDir))
        chrOutputFileName=g_build_filename(chrOutputDir, chrOutputName, NULL);
      else
        {
          /* Yes this is a memory leak but I don't care */
          chrOutputFileName=g_build_filename(g_get_current_dir(), chrOutputDir, chrOutputName, NULL);
        }
    }
  else
    chrOutputFileName=chrOutputName;

  /* Add emitter extension */
  if(fOptionEmitH)
    chrTemp=g_strconcat(chrOutputFileName, ".h", NULL);
  else if(fOptionEmitIH)
    chrTemp=g_strconcat(chrOutputFileName, ".ih", NULL);
  else if(fOptionEmitC)
    chrTemp=g_strconcat(chrOutputFileName, ".c", NULL);
  g_free(chrOutputFileName);
  chrOutputFileName=chrTemp;

  g_message("Output file: %s", chrOutputFileName);

  /* Open input */
  if(!strcmp(argv[1], "-"))
    fd=0; /* Read from stdin */
  else
    fd=open(argv[1], O_RDONLY);
  
  if(-1==fd)
    {
      g_message("Can't open input file %s", argv[1]);
      exit(1);
    }
  
  /* Open output */
  parseInfo.outFile=fopen(chrOutputFileName, "w");

  g_printf("\n");

  gScanner=g_scanner_new(NULL);
  //gScanner->user_data=(gpointer)&curSymbol;

  gScanner->msg_handler=funcMsgHandler;
  pInterfaceArray=g_ptr_array_new();

  g_scanner_input_file(gScanner, fd);
  gScanner->config->case_sensitive=TRUE;
  /* No single line comments */
  gScanner->config->skip_comment_single=FALSE;
  gScanner->config->cpair_comment_single="";
  /* This string is used in error messages of the parser */
  gScanner->input_name=IDL_COMPILER_STRING;

  g_scanner_set_scope(gScanner, ID_SCOPE);
  /* Load our own symbols into the scanner. We use the default scope for now. */
  parseInfo.pSymbolTree=g_tree_new((GCompareFunc) funcSymbolCompare);
  while(pSymbols->chrSymbolName)
    {
#warning !!! Create a copy here so it is the same as with new symbols added later.
      g_scanner_scope_add_symbol(gScanner, ID_SCOPE, pSymbols->chrSymbolName,
                                 pSymbols);
      g_tree_insert(parseInfo.pSymbolTree, pSymbols, pSymbols->chrSymbolName);
      pSymbols++;
    }

  parseIt();

  /* Write the output file */
  if(fOptionEmitH)
    emitHFile(pInterfaceArray);

#if 0
  else if(fOptionEmitIH)

  else if(fOptionEmitC)
    a++;

  if(pInterfaceArray->len)
    printInterface();
#endif

  g_scanner_destroy(gScanner);
  close(fd);
  fclose(parseInfo.outFile);
  return 0;
}

#if 0
  /* We are a folder somwhere in the chain */
  nomRetval=WPFileSystem_wpQueryFileName((WPFileSystem*)wpParent, bFullPath, NULLHANDLE);

  nomRetval=wpParent.wpQueryFileName(bFullPath, NULLHANDLE);

  nomPath=NOMPathNew();
  nomPath= (PNOMPath) NOMPath_assignCString(nomPath, _pszFullPath, ev);

  return (PNOMPath)NOMPath_appendPath(nomRetval, nomPath, NULLHANDLE);

#endif









