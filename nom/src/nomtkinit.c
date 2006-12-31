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
/* #include "nomtst.h"
   #include "nomtst2.h" */
#include <nomclassmanager.h>
/* Garbage collector */
#include <gc.h>

/* Undef if you want debugging msg from somEnvironmentNew() */
#define DEBUG_NOMENVNEW

/********************************************************/
/********************************************************/
PNOM_ENV pGlobalNomEnv;
/* Global class manager object */
NOMClassMgr* NOMClassMgrObject; /* Referenced from different files */

/********************************************************/
/*   Toolkit functions, exported                        */
/********************************************************/


/*

 */
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
  NOMObject *nomObj;
#if 0
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
  //_nomRegisterClass(NOMClassMgrObject, pGlobalNomEnv->ncpNOMObject->mtab, NULLHANDLE); //NOMObject
  _nomClassReady(_NOMObject, NULLHANDLE); //NOMObject


#if 0
  nomPrintf("\n**** Building NOMTest class...\n");
  nomTst=NOMTestNewClass(NOMTest_MajorVersion, NOMTest_MinorVersion);
  nomPrintf("NOMTest class: %x\n", nomTst);

  nomPrintf("Now building a NOMTest object from %x...\n", _NOMTest);
  nomTstObj=    NOMTestNew();
  nomPrintf("NOMTest object: %x\n", nomTstObj);
#endif

#if 0
  nomPrintf("Calling _nomTestFunc() 1\n", nomTstObj);
  _nomTestFunc(nomTstObj, NULLHANDLE);
  nomPrintf("Calling _nomTestFuncString() 1\n", nomTstObj);
  nomPrintf("--> %s\n",_nomTestFuncString(nomTstObj, NULLHANDLE));

  nomPrintf("Calling _nomTestFunc() 2\n", nomTstObj);
  _nomTestFunc(nomTstObj, NULLHANDLE);
  nomPrintf("Calling _nomTestFuncString() 2\n", nomTstObj);
  nomPrintf("--> %s\n",_nomTestFuncString(nomTstObj, NULLHANDLE));
#endif

#if 0
  //#if 0
  nomPrintf("\n**** Building NOMTest2 class...\n");
  nomPrintf("\nNow building a NOMTest2 object...\n");
  //  NOMTest2NewClass(NOMTest2_MajorVersion, NOMTest2_MinorVersion);
  nomTst2Obj=    NOMTest2New();
  nomPrintf("NOMTest2 object: %x\n", nomTst2Obj);
  //#endif


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

#if 0
  _dumpMtab(nomTstObj->mtab);
  _dumpMtab(nomTst2Obj->mtab);
  //  _dumpMtab(NOMClassMgrObject->mtab);
  //  _dumpClasses();
  _nomTestFunc(nomTstObj, NULLHANDLE);
  _nomTestFunc(nomTst2Obj, NULLHANDLE);
  _nomTestFunc(nomTstObj, NULLHANDLE);
  _nomTestFunc(nomTst2Obj, NULLHANDLE);
#endif

#if 0
  nomPrintf("NOMTest object: %x, %x\n", nomTstObj, NOMTestClassData.classObject);
  nomTstObj=_nomNew((_NOMTest ? _NOMTest : NOMTestNewClass(NOMTest_MajorVersion, NOMTest_MinorVersion)), (void*) 0);
  nomPrintf("NOMTest object: %x\n", nomTstObj);
  nomTstObj=    NOMTestNew();
  nomPrintf("NOMTest object: %x\n", nomTstObj);
#endif

  return NOMClassMgrObject;
}



NOMEXTERN PNOM_ENV NOMLINK nomTkInit(void)
{


  PVOID memPtr;
  PVOID memPool;

  nomPrintf("Entering %s...\n", __FUNCTION__);
  nomPrintf("*************************************\n");
  nomPrintf("!! This function must be rewritten !!\n");
  nomPrintf("!! It's using OS/2 only memory     !!\n");
  nomPrintf("!! functions.                      !!\n");
  nomPrintf("*************************************\n");

  /* Check if we already allocated our shared mem */
  if(NO_ERROR==DosGetNamedSharedMem(&memPtr,SOMTK_SHARED_MEM_NAME_ROOT , PAG_EXECUTE|PAG_READ|PAG_WRITE)) {
    nomPrintf("%s: Found root shared mem: %x.\n", __FUNCTION__, memPtr);
    /* Give process access to memory pool*/
    DosGetSharedMem(  ((PNOM_ENV)memPtr)->pMemPool, PAG_READ|PAG_WRITE|PAG_EXECUTE);
    if(NO_ERROR!=DosSubSetMem(((PNOM_ENV)memPtr)->pMemPool, DOSSUB_SPARSE_OBJ|DOSSUB_SERIALIZE, SOMTK_SHARED_MEM_SIZE_POOL)) {
      DosFreeMem(memPool);
      DosFreeMem(memPtr);
      return NULL;
    }
    pGlobalNomEnv=(PNOM_ENV)memPtr;
    return (PNOM_ENV)memPtr;
  }
  nomPrintf("%s: No root memory yet\n", __FUNCTION__);

  /* Get the mem for the root structure in a shared memory area */
  if(NO_ERROR!=DosAllocSharedMem(&memPtr, SOMTK_SHARED_MEM_NAME_ROOT, SOMTK_SHARED_MEM_SIZE_ROOT,
                                 PAG_COMMIT | PAG_EXECUTE | PAG_READ | PAG_WRITE))
    return NULL;

  nomPrintf("%s: Got root memory: %x\n", __FUNCTION__, memPtr);

  /* Get our shared memory pool */
  if(NO_ERROR!=DosAllocSharedMem(&memPool, NULL, SOMTK_SHARED_MEM_SIZE_POOL,OBJ_GETTABLE | PAG_EXECUTE|PAG_READ|PAG_WRITE)) {
    DosFreeMem(memPtr);
    return NULL;
  }
  nomPrintf("%s: Got shared memory pool: %x\n", __FUNCTION__, memPool);

  /* Now init the structure */
  memset(memPtr, 0, 1 /*SOMTK_SHARED_MEM_SIZE_ROOT*/);
  nomPrintf("%s: zeroed memory\n", __FUNCTION__);

  ((PNOM_ENV)memPtr)->cbSize=sizeof(NOM_ENV);
  /* Init memory pool */
  if(NO_ERROR!=DosSubSetMem(memPool, DOSSUB_INIT|DOSSUB_SPARSE_OBJ|DOSSUB_SERIALIZE, SOMTK_SHARED_MEM_SIZE_POOL)) {
    DosFreeMem(memPool);
    DosFreeMem(memPtr);
    return NULL;
  }
  ((PNOM_ENV)memPtr)->pMemPool=memPool;
  if(NO_ERROR!=DosCreateMutexSem(NULL, &((PNOM_ENV)memPtr)->hmtx, DC_SEM_SHARED, FALSE))
    {
      DosFreeMem(memPool);
      DosFreeMem(memPtr);
      return NULL;
    }
  pGlobalNomEnv=(PNOM_ENV)memPtr;

  return (PNOM_ENV)memPtr;
}











