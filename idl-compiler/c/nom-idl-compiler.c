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
  {"output", 'o', 0, G_OPTION_ARG_FILENAME, &chrOutputName, "Output name", NULL},
  {NULL}
};

static char* chrOutputFileName="";

/* The pointer array holding the interfaces we found */
GPtrArray* pInterfaceArray;

/* Symbols defined for our IDL language.
   Keep this in synch with the defined enums! */
const SYMBOL idlSymbols[]={
  {"interface", IDL_SYMBOL_INTERFACE, KIND_UNKNOWN}, /* 71 */
  {"NOMCLASSVERSION", IDL_SYMBOL_CLSVERSION, KIND_UNKNOWN},
  {"NOMINSTANCEVAR", IDL_SYMBOL_INSTANCEVAR, KIND_UNKNOWN},
  {"NOMOVERRIDE", IDL_SYMBOL_OVERRIDE, KIND_UNKNOWN},
  {"gulong", IDL_SYMBOL_GULONG, KIND_TYPESPEC},
  {"gint", IDL_SYMBOL_GINT, KIND_TYPESPEC},
  {"gpointer", IDL_SYMBOL_GPOINTER, KIND_TYPESPEC},
  {"gboolean", IDL_SYMBOL_GBOOLEAN, KIND_TYPESPEC},
  {"in", IDL_SYMBOL_IN, KIND_DIRECTION},
  {"out", IDL_SYMBOL_OUT, KIND_DIRECTION},
  {"inout", IDL_SYMBOL_INOUT, KIND_DIRECTION},
  {"define", IDL_SYMBOL_DEFINE, KIND_UNKNOWN},
  {"ifdef", IDL_SYMBOL_IFDEF, KIND_UNKNOWN},
  {"endif", IDL_SYMBOL_ENDIF, KIND_UNKNOWN},
  {NULL, 0, KIND_UNKNOWN}
},*pSymbols=idlSymbols;

GScanner *gScanner;
GTokenType curToken=G_TOKEN_EOF;
PINTERFACE pCurInterface=NULL;

/* Holding info about current token. Referenced by gScanner. */
SYMBOLINFO curSymbol;


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


gchar* getTypeSpecStringFromCurToken(void)
{
  GTokenValue value;

  if(G_TOKEN_IDENTIFIER==curToken)
    return g_strdup(value.v_identifier);
  else
    {
      /* It's one of our symbols. */
      PSYMBOLINFO psi;

      if(curToken<=G_TOKEN_LAST)
        g_scanner_unexp_token(gScanner,
                              G_TOKEN_SYMBOL,
                              NULL,
                              NULL,
                              NULL,
                              "Error in NOMINSTANCEVAR()",
                              TRUE); /* is_error */

      psi=(PSYMBOLINFO)gScanner->user_data;
      return psi->pSymbols[curToken-G_TOKEN_LAST-1].chrSymbolName;
    }
}


/**
   This is the root parse function. Here starts the fun...
 */
void parseIt(void)
{
  while(g_scanner_peek_next_token(gScanner) != G_TOKEN_EOF) {
    GTokenType token;

    curToken=g_scanner_get_next_token(gScanner);
    token=curToken;
    GTokenValue value=gScanner->value;
    
    switch(curToken)
      {
      case IDL_SYMBOL_INTERFACE:
        parseInterface(token);
        break;
      case '#':
        parseHash();
        break;

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
        printToken(curToken);
        //  g_message("Token: %d (---)\t\t\t%c (LINE %d)", token, token, g_scanner_cur_line(gScanner));
        break;
      }
  }
}

/* Show help.
   gContext must be valid.
*/
static void outputCompilerHelp(GOptionContext *gContext, gchar* chrExeName)
{
  GError *gError = NULL;
  int argc2=2;
  char *helpCmd[]={"","--help"};
  char** argv2=helpCmd;
  helpCmd[0]=chrExeName;
  
  g_option_context_parse (gContext, &argc2, &argv2, &gError); 
}

/*

 */
int main(int argc, char **argv)
{
  int a;
  int fd;
  int idScope=0;
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

#if 0
  if(!a){
    g_printf("An emitter must be specified.\n\n");
    outputCompilerHelp(gContext, argv[0]);
  }
  if(a>1){
    g_printf("Only one emitter must be specified.\n\n");
    outputCompilerHelp(gContext, argv[0]);
  }
#endif
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

  /* Create output file name */

  fd=open(argv[1], O_RDONLY);
  
  if(-1==fd)
    {
      g_message("Can't open input file %s", argv[1]);
      exit(1);
    }
  
  g_printf("\n");

  gScanner=g_scanner_new(NULL);
  gScanner->user_data=(gpointer)&curSymbol;
  curSymbol.pSymbols=idlSymbols;

  pInterfaceArray=g_ptr_array_new();

  g_scanner_input_file(gScanner, fd);
  /* No single line comments */
  gScanner->config->skip_comment_single=FALSE;
  gScanner->config->cpair_comment_single="";
  /* This string is used in error messages of the parser */
  gScanner->input_name=IDL_COMPILER_STRING;

  g_scanner_set_scope(gScanner, idScope);
  /* Load our own symbols into the scanner. We use the default scope for now. */
  while(pSymbols->chrSymbolName)
    {
      g_scanner_scope_add_symbol(gScanner, idScope, pSymbols->chrSymbolName, GINT_TO_POINTER(pSymbols->uiSymbolToken));
      pSymbols++;
    }
  gScanner->config->symbol_2_token=TRUE;

  parseIt();

  //if(pInterfaceArray->len)
  //  printInterface();

  g_scanner_destroy(gScanner);
  close(fd);

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
