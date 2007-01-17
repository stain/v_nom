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
* Portions created by the Initial Developer are Copyright (C) 2005-2006
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

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSMEMMGR
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/* For nomToken etc. */
#include <nom.h>
#include "nomtk.h"
#include "nomobj.h"
#include "nomcls.h"
#include <nomclassmanager.h>
/* Garbage collector */
#include <gc.h>

/* Undef if you want debugging msg from somEnvironmentNew() */
#define DEBUG_NOMENVNEW

/********************************************************/
/********************************************************/
PNOM_ENV pGlobalNomEnv;
/* Global class manager object */
NOMClassMgr* NOMClassMgrObject=NULLHANDLE; /* Referenced from different files */

/********************************************************/
/*   Toolkit functions, exported                        */
/********************************************************/


/*
  Some of these functions should be moved to other source files...
 */

/*
  This function asks the NOMClassMgrObject to search in the internal lists
  for the mtab of the given object. If that mtab is found the pointer points to a
  valid object.
 */
NOMEXTERN gboolean NOMLINK nomIsObj(gpointer nomObj)
{
  if(NOMClassMgrObject)
    return NOMClassMgr_nomIsObject(NOMClassMgrObject, (PNOMObject)nomObj, NULLHANDLE);

  if(!nomObj)
    return FALSE;

  /* We assume that here... */
  return TRUE;
}

NOMEXTERN int NOMLINK nomPrintf(string chrFormat, ...)
{
  long rc;

  va_list arg_ptr;

  va_start (arg_ptr, chrFormat);
  rc=vprintf( chrFormat, arg_ptr);
  va_end (arg_ptr);
  return rc;
}



/*
  This function is called to initialize the NOM runtime. 
   It creates NOMObject, NOMClass and NOMClassMgr classes and the global
   NOMClassMgrObject. 
*/
NOMEXTERN NOMClassMgr * NOMLINK nomEnvironmentNew (void)
{
  NOMClassPriv* ncPriv;
  NOMClass* nomCls;
#if 0
  NOMObject *nomObj;
  NOMTest* nomTst;
  NOMTest* nomTstObj;
  NOMTest* nomTst2Obj;
#endif

#ifdef DEBUG_NOMENVNEW
  nomPrintf("Entering %s to initialize NOM runtime.\n\n", __FUNCTION__);
  nomPrintf("**** Building NOMObject class...\n");
#endif
  nomCls=NOMObjectNewClass(NOMObject_MajorVersion, NOMObject_MinorVersion);

#ifdef DEBUG_NOMENVNEW
  nomPrintf("NOMObject class: %x\n", nomCls);
  nomPrintf("\n**** Building NOMClass class...\n");
#endif

  nomCls=NOMClassNewClass(NOMClass_MajorVersion, NOMClass_MinorVersion);
#ifdef DEBUG_NOMENVNEW
  nomPrintf("NOMClass class: %x\n", nomCls);
  nomPrintf("\n**** Building NOMClassMgr class and NOMClassMgrObject...\n");
#endif
  NOMClassMgrObject=NOMClassMgrNew();
  if(!NOMClassMgrObject)
    g_error("Can't create the NOMClassMgr class object!\n");

#ifdef DEBUG_NOMENVNEW
  nomPrintf("%s: NOMClassMgrObject: %x \n", __FUNCTION__, NOMClassMgrObject);
#endif

  /* Now register the classes we already have */
  //  _nomRegisterClass(NOMClassMgrObject,   pGlobalNomEnv->defaultMetaClass->mtab, NULLHANDLE); //NOMClass
  _nomClassReady(pGlobalNomEnv->defaultMetaClass, NULLHANDLE); //NOMClass
  //_nomRegisterClass(NOMClassMgrObject,   NOMClassMgrObject->mtab, NULLHANDLE); //NOMClassMgr
  _nomClassReady(  _NOMClassMgr, NULLHANDLE); //NOMClassMgr
  ncPriv=(NOMClassPriv*)pGlobalNomEnv->nomObjectMetaClass->mtab->nomClsInfo;

  /* Do not register the NOMObject metaclass here. It's already registered because it's 
     NOMClass in fact. */
  //_nomRegisterClass(NOMClassMgrObject, pGlobalNomEnv->nomObjectMetaClass->mtab, NULLHANDLE); //NOMObject
  _nomClassReady(_NOMObject, NULLHANDLE); //NOMObject


#if 0
  nomPrintf("\n**** Building NOMTest2 class...\n");
  nomPrintf("\nNow building a NOMTest2 object...\n");
  nomTst2Obj=    NOMTest2New();
  nomPrintf("NOMTest2 object: %x\n", nomTst2Obj);

  nomPrintf("\nCalling _nomTestFunc_NOMTest2() 1\n", nomTst2Obj);
  _nomTestFunc_NOMTest2(nomTst2Obj, NULLHANDLE);
  nomPrintf("\nCalling _nomTestFuncString_NOMTest2() 1\n", nomTst2Obj);
  nomPrintf("--> %s\n",_nomTestFuncString_NOMTest2(nomTst2Obj, NULLHANDLE));
  nomPrintf("\nCalling _nomTestFunc_NOMTest2() 2\n", nomTst2Obj);
  _nomTestFunc_NOMTest2(nomTst2Obj, NULLHANDLE);

  nomPrintf("\nCalling _nomTestFunc() with NOMTest2 object: %x\n", nomTst2Obj);
  _nomTestFunc(nomTst2Obj, NULLHANDLE);
  nomPrintf("\nCalling _nomTestFuncString() with NOMTest2: %x\n", nomTst2Obj);
  nomPrintf("--> %s\n",_nomTestFuncString(nomTst2Obj, NULLHANDLE));
  nomPrintf("\n");
  _nomTestFunc(nomTst2Obj, NULLHANDLE);
  _nomTestFunc_NOMTest2(nomTst2Obj, NULLHANDLE);
#endif

  return NOMClassMgrObject;
}


NOMEXTERN PNOM_ENV NOMLINK nomTkInit(void)
{
  PVOID memPtr;
  //PVOID memPool;

  nomPrintf("Entering %s...\n", __FUNCTION__);

  memPtr=g_malloc(sizeof(NOM_ENV)); /* g_malloc() can't fail... */

  nomPrintf("%s: Got root memory: %x\n", __FUNCTION__, memPtr);

  /* Now init the structure */
  /* GC memory is zeroed... */
  ((PNOM_ENV)memPtr)->cbSize=sizeof(NOM_ENV);
  pGlobalNomEnv=(PNOM_ENV)memPtr;

#if 0
  if(NO_ERROR!=DosCreateMutexSem(NULL, &((PNOM_ENV)memPtr)->hmtx, DC_SEM_SHARED, FALSE))
    {
      g_free(memPtr);
      return NULL;
    }
#endif

  return (PNOM_ENV)memPtr;
}











