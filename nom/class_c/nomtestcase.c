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
 * Portions created by the Initial Developer are Copyright (C) 2008
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
/*
 * And remember, phase 3 is near...
 */
#ifndef NOM_NOMTestCase_IMPLEMENTATION_FILE
#define NOM_NOMTestCase_IMPLEMENTATION_FILE
#endif

#if __OS2__
#define INCL_DOS
#include <os2.h>
#endif /* __OS2__ */

#include <nom.h>
#include <nomtk.h>

#include "nomarray.h"
#include "nomtestcase.ih"

NOMDLLEXPORT NOM_Scope void NOMLINK impl_NOMTestCase_setUp(NOMTestCase* nomSelf,
                                                           CORBA_Environment *ev)
{
  /* NOMTestCaseData* nomThis = NOMTestCaseGetData(nomSelf); */

}

NOMDLLEXPORT NOM_Scope void NOMLINK impl_NOMTestCase_tearDown(NOMTestCase* nomSelf,
                                                              CORBA_Environment *ev)
{
  /* NOMTestCaseData* nomThis = NOMTestCaseGetData(nomSelf); */

}

NOMDLLEXPORT NOM_Scope NOMArray* NOMLINK impl_NOMTestCase_runTests(NOMTestCase* nomSelf,
                                                              CORBA_Environment *ev)
{
  NOMArray* resultArray=NOMArrayNew();
  
  /* NOMTestCaseData* nomThis = NOMTestCaseGetData(nomSelf); */

  _setUp(nomSelf, NULL);
  
  _tearDown(nomSelf, NULL);
  return resultArray;	
}

NOMDLLEXPORT NOM_Scope void NOMLINK impl_NOMTestCase_runSingleTest(NOMTestCase* nomSelf,
                                                                   const CORBA_char* chrTestName,
                                                                   CORBA_Environment *ev)
{
  /* NOMTestCaseData* nomThis = NOMTestCaseGetData(nomSelf); */

  _setUp(nomSelf, NULL);
  
  
  _tearDown(nomSelf, NULL);  
}

