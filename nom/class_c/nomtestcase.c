/*
 * This file was generated by the NOM IDL compiler for Voyager - DO NOT EDIT!
 *
 *
 * And remember, phase 3 is near...
 */
/*
 * Built from /Users/cinc/svn-sources/nom/trunk/nom/idl/nomtestcase.idl
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
