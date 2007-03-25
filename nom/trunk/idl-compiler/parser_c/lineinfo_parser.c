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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include "parser.h"

extern GScanner *gScanner;
extern PARSEINFO parseInfo;

/*
  Current token is the first integer.

  LI:= INT STRING INT     // Line info from the preprocessor
 */
void parsePreprocLineInfo(void)
{
  GTokenValue value;

  /* Line number */
  value=gScanner->value;
  parseInfo.uiLineCorrection=g_scanner_cur_line(gScanner)-value.v_int+1;

  if(!matchNext(G_TOKEN_STRING))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_STRING,
                            NULL,
                            NULL,
                            NULL,
                            "Trying to parse preprocessor line information.",
                            TRUE); /* is_error */
      exit(1);
    }

  /* Current source file */
  if(parseInfo.chrCurrentSourceFile)
    g_free(parseInfo.chrCurrentSourceFile);

  value=gScanner->value;
  parseInfo.chrCurrentSourceFile=g_strdup(value.v_string);

  /* Trailing file include level info isn't used for now. Note that for the root
     level no trailing int is following. */
  matchNext(G_TOKEN_INT);
#if 0
  if(!matchNext(G_TOKEN_INT))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_INT,
                            NULL,
                            NULL,
                            NULL,
                            "Trying to parse preprocessor line information.",
                            TRUE); /* is_error */
      exit(1);
    }
#endif
}