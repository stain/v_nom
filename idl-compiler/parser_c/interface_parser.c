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
#include <os2.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include <glib/gprintf.h> 
#include "parser.h"

extern GScanner *gScanner;
/* The pointer array holding the interfaces we found */
extern GPtrArray* pInterfaceArray;
extern PPARSEINFO pParseInfo;

static void registerInterface(void)
{
  PSYMBOL pNewSymbol=g_malloc0(sizeof(SYMBOL));

  if(!strcmp(pParseInfo->chrRootSourceFile, pParseInfo->pCurInterface->chrSourceFileName))
    pParseInfo->pCurInterface->fIsInRootFile=TRUE;

  g_ptr_array_add(pInterfaceArray, (gpointer) pParseInfo->pCurInterface);

  /* Any found interface is registered as a new type so it can be
     used in other classes. */
  pNewSymbol->chrSymbolName=g_strdup(pParseInfo->pCurInterface->chrName); /* We create a copy here because
                                                                             when cleaning up the symbol space
                                                                             the string will be freed. */
  pNewSymbol->uiKind=KIND_TYPESPEC;
  pNewSymbol->uiSymbolToken=IDL_SYMBOL_REGINTERFACE;
  g_tree_insert(pParseInfo->pSymbolTree, pNewSymbol, pNewSymbol->chrSymbolName);
  g_scanner_scope_add_symbol(gScanner, ID_SCOPE, pNewSymbol->chrSymbolName,
                             pNewSymbol);
  /* For legacy support and convenience we automatically register a pointer type
     to the interface. */
  pNewSymbol=g_malloc0(sizeof(SYMBOL));
  pNewSymbol->uiKind=KIND_TYPESPEC;
  pNewSymbol->uiSymbolToken=IDL_SYMBOL_REGINTERFACE;
  pNewSymbol->chrSymbolName=g_strconcat("P", pParseInfo->pCurInterface->chrName, NULL);
  g_tree_insert(pParseInfo->pSymbolTree, pNewSymbol, pNewSymbol->chrSymbolName);
  g_scanner_scope_add_symbol(gScanner, ID_SCOPE, pNewSymbol->chrSymbolName,
                             pNewSymbol);
  //g_message("%s: %s", __FUNCTION__, pNewSymbol->chrSymbolName);
}

static PINTERFACE createInterfaceStruct()
{
  PINTERFACE pInterface;

  pInterface=(PINTERFACE)g_malloc0(sizeof(INTERFACE));
  pInterface->pInstanceVarArray=g_ptr_array_new();
  pInterface->pMethodArray=g_ptr_array_new();
  pInterface->pOverrideArray=g_ptr_array_new();

  return pInterface;
}

/*
  Function to parse the body of an interface declaration.

  IB:= CV                                             // NOMCLASSVERSION()
     | IV                                             // NOMINSTANCEVAR()
     |  M                                             // Method
     | OV                                             // Overriden method
 */
void parseIBody(void)
{
  /* Current token is '{' */

  do{
    //  g_printf("%d: ", __LINE__);
    //  printToken(curToken);

    /* Typespec check must be first */
    if(matchNextKind(KIND_TYPESPEC)) /* Be aware that we don't compare types here */
      {
        parseMethod();
      }
    else if(matchNext('#'))
      parseHash();
    else if(matchNext(G_TOKEN_SYMBOL))
      {
        PSYMBOL pCurSymbol;
        GTokenValue value;
        
        value=gScanner->value;
        pCurSymbol=value.v_symbol;
        switch(pCurSymbol->uiSymbolToken)
          {
          case IDL_SYMBOL_CLSVERSION:
            parseClassVersion();
            break;
          case IDL_SYMBOL_OVERRIDE:
            parseOverrideMethod();
            break;
          case IDL_SYMBOL_INSTANCEVAR:
            parseInstanceVar();
            break;
          case IDL_SYMBOL_METACLASS:
            parseMetaClass();
            break;
          default:
            g_scanner_unexp_token(gScanner,
                                  G_TOKEN_SYMBOL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  "Trying to parse interface body.",
                                  TRUE); /* is_error */
            exit(1);
          }/* switch */
      }
    else
      {
        getNextToken();
        g_scanner_unexp_token(gScanner,
                              G_TOKEN_IDENTIFIER,
                              NULL,
                              NULL,
                              NULL,
                              "Trying to parse interface body.",
                              TRUE); /* is_error */
        exit(1);
      }
    }while(g_scanner_peek_next_token(gScanner)!='}');
}


/*
  Parse the interface name.
  Note that the current token is the 'interface' keyword.

  I:= IDL_SYMBOL_INTERFACE G_TOKEN_INDENTIFIER
 */
static void parseIFace(GTokenType token)
{
  if(!matchNext(G_TOKEN_IDENTIFIER))
    {
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_IDENTIFIER,
                            NULL,
                            NULL,
                            NULL,
                            "Keyword 'interface' must be followed by an identifier",
                            TRUE); /* is_error */
      exit(1);
    }
  /* Save interface info */
  GTokenValue value=gScanner->value;
  pParseInfo->pCurInterface->chrName=g_strdup(value.v_identifier);
}

/*
  Current token is '{'.

  IB2:= '{' IB '}'
      | '{' IB '}' ';'

*/
static void parseIFaceBody(void)
{
  parseIBody();
  if(!matchNext('}'))
    {
      g_scanner_unexp_token(gScanner,
                            '}',
                            NULL,
                            NULL,
                            NULL,
                            "No closing of 'interface' section.",
                            TRUE); /* is_error */
      exit(1);
    }
  /* Remove a terminating ';' from the input if present. */
  matchNext(';');
}

/*
  Parse an interface which is subclassed. This includes checking if the parent
  interface is already defined.

  IS:= G_TOKEN_SYMBOL IB2

  It's G_TOKEN_SYMBOL here because every found interface is registered
  as a new symbol with GScanner.
 */
static void parseSubclassedIFace()
{
  PSYMBOL pCurSymbol;

  /* Parent interface */
  if(!matchNext(G_TOKEN_SYMBOL))
    {
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_SYMBOL,
                            NULL,
                            NULL,
                            NULL,
                            "Parent interface name is missing or unknown.",
                            TRUE); /* is_error */
      exit(1);
    }
  GTokenValue value=gScanner->value;
  /* Make sure it's the correct symbol */
  pCurSymbol=value.v_symbol;

  if(IDL_SYMBOL_REGINTERFACE!=pCurSymbol->uiSymbolToken)
    {
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_SYMBOL,
                            NULL,
                            NULL,
                            NULL,
                            "Parent interface name is unknown.",
                            TRUE); /* is_error */
      exit(1);
    }
  pParseInfo->pCurInterface->chrParent=g_strdup(pCurSymbol->chrSymbolName);

  /* Check if the parent interface is known. */
  if(!findInterfaceFromName(pParseInfo->pCurInterface->chrParent))
  {
    g_scanner_unexp_token(gScanner,
                          G_TOKEN_IDENTIFIER,
                          NULL,
                          NULL,
                          NULL,
                          "Parent interface in definition is unknown.",
                          TRUE); /* is_error */
    exit(1);
  }

  if(!matchNext('{'))
    {
      g_scanner_unexp_token(gScanner,
                            '{',
                            NULL,
                            NULL,
                            NULL,
                            "No opening brace in interface definition.",
                            TRUE); /* is_error */
      exit(1);
      
    }
  parseIFaceBody();
}

/*
  Parse an interface declaration. The current token is the 'interface' keyword.

  interface:= I ';'                       // Forward declaration
            | I IB2
            | I ':' IS                    // Subclassed interface

  This translates into:

  interface:= I ';'                       // Forward declaration
            | I '{' IB '}'
            | I ':' G_TOKEN_SYMBOL '{' IB '}'

            It's G_TOKEN_SYMBOL here because every found interface is registered
            as a new symbol with GScanner.
 */
void parseInterface(GTokenType token)
{
  pParseInfo->pCurInterface=createInterfaceStruct();

  /* Get the interface name */
  parseIFace(token);
  pParseInfo->pCurInterface->chrSourceFileName=g_strdup(pParseInfo->chrCurrentSourceFile);

  /* It's save to register the interface right here even if the struct is almost empty. 
     If anything goes wrong later we will exit anyway. */
  registerInterface();  

  if(matchNext(';'))
    {
      pParseInfo->pCurInterface->fIsForwardDeclaration=TRUE;
    }
  else if(matchNext(':'))
    {
      parseSubclassedIFace();
    }
  else if(matchNext('{'))
    {
      parseIFaceBody();
    }
  else
    {
      g_message("Line %d: Error in interface declaration",  g_scanner_cur_line(gScanner));
      exit(0);
    }
}

