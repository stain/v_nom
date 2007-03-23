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
* Portions created by the Initial Developer are Copyright (C) 2005-2007
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
#include <gtk/gtk.h> 
#include "nom.h"

#include "parser.h"

extern GScanner *gScanner;
extern GTokenType curToken;

/*
  We need this information during parsing to decide if an identifier
  is e.g. a typespec or just a var name.
 */
void setSymbolInfo(GTokenType token)
{
  PSYMBOLINFO psi;

  psi=(PSYMBOLINFO)gScanner->user_data;

  switch(token)
    {
    case G_TOKEN_IDENTIFIER:
      /* Here we have to check identifiers if they are for example types, e.g. int */
      break;
    default:
      psi->uiCurSymbolKind=KIND_UNKNOWN;
      break;
    }
  if(token>G_TOKEN_LAST)
    {
      psi->uiCurSymbolKind=psi->pSymbols[token-G_TOKEN_LAST-1].uiKind;
    }
}

guint queryCurTokensKind(void)
{
  PSYMBOLINFO psi;

  psi=(PSYMBOLINFO)gScanner->user_data;
  return psi->uiCurSymbolKind;
}

static guint getKindFromTokenType(GTokenType token)
{
  if(token>G_TOKEN_LAST)
    {
      PSYMBOLINFO psi;
      
      psi=(PSYMBOLINFO)gScanner->user_data;
      return psi->pSymbols[token-G_TOKEN_LAST-1].uiKind;
    }
  switch(token)
    {
    case G_TOKEN_IDENTIFIER:
      {
        /* Compare strings here. Yes, that's slow... */
        break;
      }
    default:
      break;
    }

  return KIND_UNKNOWN;
}

guint queryNextTokensKind(void)
{
  return getKindFromTokenType(gScanner->next_token);
}


/* Well, the name says all... */
void getNextToken(void)
{
  curToken = g_scanner_get_next_token(gScanner);
  setSymbolInfo(curToken);
}

gboolean matchNextKind(guint uiKind)
{
  GTokenType token;

  token=g_scanner_peek_next_token(gScanner);
  if(uiKind==getKindFromTokenType(token))
    {
      getNextToken();
      return TRUE;
    }
  return FALSE;
}

/* Well, the name says all... */
gboolean matchCur(GTokenType token)
{
  if(token==curToken)
    {
      return TRUE;
    }
  return FALSE;
}

/* Well, the name says all...

   Note that this function advances to the next token if the
   tokens match.
*/
gboolean matchNext(GTokenType token)
{
  if(token==g_scanner_peek_next_token(gScanner))
    {
      getNextToken();
      return TRUE;
    }
  return FALSE;
}

/*
  Print current token info.
 */
void printToken(GTokenType token)
{
  GTokenValue value=gScanner->value;

  switch(token)
    {
    case IDL_SYMBOL_INTERFACE:
      g_message("Token: %d (IDL_SYMBOL_INTERFACE)\tParsing...", token);
      break;
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
      g_message("Token: %d (semicolon)\t\t\t; (LINE %d)", token, g_scanner_cur_line(gScanner));
      break;
    case '#':
      g_message("Token: %d (hash)\t\t\t#", token);
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
    default:
      g_message("Token: %d (---)\t\t\t (LINE %d)", token, g_scanner_cur_line(gScanner));
      break;
    }
}
