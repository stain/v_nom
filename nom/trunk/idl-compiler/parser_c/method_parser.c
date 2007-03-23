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

#include <glib.h> 
#include "parser.h"

extern GScanner *gScanner;
extern GTokenType curToken;
extern PINTERFACE pCurInterface;

static PMETHOD createMethodStruct()
{
  PMETHOD pMethod=g_malloc0(sizeof(METHOD));

  pMethod->pParamArray=g_ptr_array_new();

  return pMethod;
}

/*
  Parse one or more method parameters. Note that the check for '(' and ')' is
  done in the calling function.

  MPARAMS:= DIRECTION TS G_TOKEN_IDENTIFIER
          | MPARAMS ',' MPARAMS 
 */
static void parseMethodParams(PMETHOD pMethod)
{
  GTokenValue value;
  PMETHODPARAM pParam;

  do{ 
    pParam=g_malloc0(sizeof(METHODPARAM));

    /* Direction */
    if(!matchNextKind(KIND_DIRECTION)) /* Be aware that we don't compare types here */
      {
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              G_TOKEN_IDENTIFIER,
                              NULL,
                              NULL,
                              NULL,
                              "Error in method declaration, direction (in|out|inout) is not specified.",
                              TRUE); /* is_error */
        exit(1);
      }

    switch(curToken)
      {
      case IDL_SYMBOL_IN:
        pParam->uiDirection=PARM_DIRECTION_IN;
        break;
      case IDL_SYMBOL_OUT:
        pParam->uiDirection=PARM_DIRECTION_OUT;
        break;
      case IDL_SYMBOL_INOUT:
        pParam->uiDirection=PARM_DIRECTION_INOUT;
        break;
      default:
        break;
      }

    /* Typespec */
    if(matchNextKind(KIND_TYPESPEC)) /* Be aware that we don't compare types here */
      parseTypeSpec(pParam);
    else
      {
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              G_TOKEN_IDENTIFIER,
                              NULL,
                              NULL,
                              NULL,
                              "Error in method declaration, Unknown type specification.",
                              TRUE); /* is_error */
        exit(1);
      }

    //pParam->chrType=getTypeSpecStringFromCurToken();

    if(!matchNext(G_TOKEN_IDENTIFIER))
      {
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              G_TOKEN_IDENTIFIER,
                              NULL,
                              NULL,
                              NULL,
                              "Error in method declaration, parameter name is wrong.",
                              TRUE); /* is_error */
        exit(1);
      }
    value=gScanner->value;
    pParam->chrName=g_strdup(value.v_identifier);

    g_ptr_array_add(pMethod->pParamArray , (gpointer) pParam);
  }while(matchNext(','));
}


/*
  Current token is the typespec.

  TS:= TYPE_SPEC              //typespec, see elsewhere
    |  TYPE_SPEC '*'

  M:= TS G_TOKEN_IDENTIFIER '(' ')' ';'        // method
    | TS G_TOKEN_IDENTIFIER '(' MPARMS ')' ';' // method
 */
void parseMethod(void)
{
  GTokenValue value;
  PMETHOD pMethod=createMethodStruct();

  //g_printf("%d: ", __LINE__);
  //printToken(curToken);

  /* Do type spec */
  parseTypeSpec(&pMethod->mpReturn);

  /* Method name */
  if(!matchNext(G_TOKEN_IDENTIFIER))
    {
      getNextToken(); /* Make sure error references the correct token */
      printToken(curToken);
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_IDENTIFIER,
                            NULL,
                            NULL,
                            NULL,
                            "Error in method declaration",
                            TRUE); /* is_error */
      exit(1);
    }
  value=gScanner->value;
  pMethod->chrName=g_strdup(value.v_identifier);

  /* Handle parameters  if any */
  if(!matchNext('('))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            '(',
                            NULL,
                            NULL,
                            NULL,
                            "Error in method declaration",
                            TRUE); /* is_error */
      exit(1);
    }

  /* No parameters */
  if(matchNext(')'))
    {
      if(!matchNext(';'))
        {
          getNextToken(); /* Make sure error references the correct token */
          g_scanner_unexp_token(gScanner,
                                ';',
                                NULL,
                                NULL,
                                NULL,
                                "Missing semicolon.",
                                TRUE); /* is_error */
          exit(1);
        }
      g_ptr_array_add( pCurInterface->pMethodArray, (gpointer) pMethod);
      return;
    }

  /* This parses all the parameters */
  parseMethodParams(pMethod);

  if(!matchNext(')'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ')',
                            NULL,
                            NULL,
                            NULL,
                            "Error in method declaration",
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
                            "Error in method declaration, Missing semicolon",
                            TRUE); /* is_error */
      exit(1);
    }

  g_ptr_array_add( pCurInterface->pMethodArray, (gpointer) pMethod);
}
