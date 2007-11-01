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
#include "nomgc.h"
#include "aclass.h"

#define ULONG_TESTVALUE_1  0xffeeddcc
#define ULONG_TESTVALUE_2  0x55aa1122

AClass*  createAClassObject()
{
  AClass*  aClass;
  
  aClass=AClassNew();
  
  if(nomIsObj(aClass))
    g_message("AClass creation\t\t\t\tOK\n");
  else
    g_message("AClass creation\t\t\t\t\tFAILED\n");
  return aClass;
}

/**
   Main entry point for the idl compiler.
 */
int main(int argc, char **argv)
{
  NOMClassMgr *NOMClassMgrObject;
  HREGDLL hReg=NULLHANDLE;
  AClass*  aObject;
  ULONG ulRC;

#if 0
  /* Preload the DLL otherwise it won't be found by the GC registering function */
  if((rc=DosLoadModule(uchrError, sizeof(uchrError),"nobjtk.dll", &hModuleGC))!=NO_ERROR)
    {
      printf("DosLoadmodule for nobjtk.dll failed with rc=0x%x because of module %s.\n", (int)rc, uchrError);
      return 1;
    };
  fprintf(stderr, "DLL handle for nobjtk.dll is: 0x%x\n", (int)hModuleGC);
#endif
  nomInitGarbageCollection(NULL);

  /* Register DLLs with the garbage collector */
  hReg=nomBeginRegisterDLLWithGC();
  if(NULLHANDLE==hReg)
    return 1;

#if 0
  //g_assert(nomRegisterDLLByName(hReg, "GLIB2.DLL" ));
  //g_assert(nomRegisterDLLByName(hReg, "GOBJECT2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GMODULE2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GDK2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GDKPIX2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GTK2.DLL" ));
  g_assert(nomRegisterDLLByName(hReg, "ATK.DLL" ));
#endif
  g_assert(nomRegisterDLLByName(hReg, "NOBJTK.DLL"));

  nomEndRegisterDLLWithGC(hReg);

  g_message("NOM test application started.");

  /* Init NOM */
  NOMClassMgrObject=nomEnvironmentNew();

  /* Try to create an object */
  aObject=createAClassObject();
  g_assert(aObject);

  /* -- Call methods on the object --- */
  g_message("================================================================");
  g_message("===== Testing init values of instance variables. Must be 0 =====");
  g_message("================================================================");

  ulRC=_tstQueryUlongVar1(aObject, NULLHANDLE);
  g_message("Calling tstQueryUlongVar1():\t%ld\t\t%s", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);

  ulRC=_tstQueryUlongVar2(aObject, NULLHANDLE);
  g_message("Calling tstQueryUlongVar2():\t%ld\t\t%s\n", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);

  g_message("=================================================");
  g_message("===== Testing setting of instance variables =====");
  g_message("=================================================");
  _tstSetUlongVar1(aObject, ULONG_TESTVALUE_1, NULLHANDLE);
  ulRC=_tstQueryUlongVar1(aObject, NULLHANDLE);
  g_message("Calling tstQueryUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_1==ulRC);

  ulRC=_tstQueryUlongVar2(aObject, NULLHANDLE);
  g_message("Calling tstQueryUlongVar2():\t0x%lx\t\t%s\n", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);

  _tstSetUlongVar2(aObject, ULONG_TESTVALUE_2, NULLHANDLE);
  ulRC=_tstQueryUlongVar1(aObject, NULLHANDLE);
  g_message("Calling tstQueryUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_1==ulRC);

  ulRC=_tstQueryUlongVar2(aObject, NULLHANDLE);
  g_message("Calling tstQueryUlongVar2():\t0x%lx\t\t%s\n", ulRC, (ULONG_TESTVALUE_2!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_2==ulRC);

  return 0;
};







