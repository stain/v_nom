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

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <glib.h>
#define SOM_NO_OBJECTS  /* Otherwise som.h includes the IBM SOM classes */

/* For somToken etc. */
#include <nom.h>
#include <nomtk.h>
#include <nomobj.h>
#include <nomcls.h>
#include <nomclassmanager.h>

/* Define if you want to have messages from somBuildClass() and friends */
//#define DEBUG_NOMBUILDCLASS
/* Define if you want to have messages from building NOMObject */
//#define DEBUG_BUILDNOMOBJECT
/* Define if you want to have messages from building NOMClass */
//#define DEBUG_BUILDNOMCLASS

#ifdef DEBUG_BUILDNOMCLASS
    #define BUILDNOMCLASS_ENTER nomPrintf("\n%s line %d: *** entering %s...\n",__FILE__, __LINE__,  __FUNCTION__);
    #define BUILDNOMCLASS_LEAVE nomPrintf("%s line %d: *** Leaving %s...\n\n",__FILE__, __LINE__,  __FUNCTION__);
#else
    #define BUILDNOMCLASS_ENTER
    #define BUILDNOMCLASS_LEAVE
#endif

#define DBGBUILDNOMCLASS_ENTER BUILDNOMCLASS_ENTER
#define DBGBUILDNOMCLASS_LEAVE BUILDNOMCLASS_LEAVE

/********************************************************/
extern NOMClassMgr* NOMClassMgrObject;

extern PNOM_ENV pGlobalNomEnv;

/******************* somBuildClass **********************/

/*
  Thunking code to get the instance var address from an object pointer pushed
  on the stack. The following translates into this assembler code:

  MOV EAX,DWORD PTR [ESP+4] ;Move object ptr into EAX
  ADD EAX, +4
  RET
*/

static ULONG thunk[]={0x0424448b, 0x00000405, 0x0000c300};

/*
MOV ECX,DWORD PTR [ESP+4] : move object pointer from stack in ECX
MOV EDX,DWORD PTR [ECX]   : move [ECX] in EDX -> mtab in EDX
JMP DWORD PTR [EDX+0ACh]  : JMP to address pointing to by EDX+0ACh
 */
static ULONG mThunkCode[]={0x04244c8b, 0xff00518b, 0x0000aca2 , 0x16000000};


/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
/*
  FIXME:

  This is similar to nomIsA() of NOMObject so better use that later.

 */

static BOOL priv_nomIsA(NOMObject *nomSelf, NOMClass* aClassObj)
{
  nomParentMtabStructPtr pParentMtab=&((NOMClassPriv * )nomSelf)->parentMtabStruct;
  nomMethodTabs psmTab;

  if(!nomSelf||!aClassObj)
    return FALSE;
  
  /*
    FIXME: use nomIsObj here!!
    if(!nomIsObj(nomSelf)||!nomIsObj(aClassObj))
    return FALSE
    */
  
  if(!strcmp(nomSelf->mtab->nomClassName, aClassObj->mtab->nomClassName))
    return TRUE;
  
  psmTab=pParentMtab->next;
  while(psmTab) {
    if(!strcmp(psmTab->mtab->nomClassName, aClassObj->mtab->nomClassName))
      return TRUE;
    
    psmTab=psmTab->next;
  }
  
  /* Return statement to be customized: */
  return FALSE;
}

/*
  Helper function 
 */
static
gulong nomGetNumIntroducedMethods(NOMClassPriv* ncPriv)
{
  return ncPriv->sci->ulNumStaticMethods;
}

/*
  This function should be reworked later to just get the class object from the quark list
  instead of iterating over all parents.
 */
static
gulong priv_getIndexOfMethodInEntries(NOMClassPriv* nClass, nomStaticClassInfo *sci, gchar* chrMethodDescriptor)
{
  int a;
  gulong idx=0;
  gboolean bFound=FALSE;
  char* chrMethodName;

  /* We need the full info here, method name and class introducing it */
  if((chrMethodName=strchr(chrMethodDescriptor, ':'))==NULL)
    return 0;

  chrMethodName++;

  /* chrMethodName is like "ClassName:MethodName". */
  for(a=0; a<sci->ulNumParentsInChain; a++)
    {
      GString *gstr;

      gstr=g_string_new(sci->chrParentClassNames[a]);
      g_string_append_printf(gstr, ":%s", chrMethodName);

      // nomPrintf("Checking %s for %s\n", 
      //      sci->chrParentClassNames[a], gstr->str);
      if(g_quark_try_string(gstr->str))
        {
          int b;
          NOMClassPriv *ncPriv;
          gulong ulNumMethods;

          /* Found class introducing the method */
          //nomPrintf(" %s, %d: Found %s in %s\n", __FUNCTION__, __LINE__,
          //        gstr->str, sci->chrParentClassNames[a]);
          if(NULL==NOMClassMgrObject)
            g_error("%s line %d: No overriding for base classes yet(%s)!\n",
                    __FUNCTION__, __LINE__, gstr->str);
          idx++; /* This is the position for the class pointer */
          ncPriv=_nomGetClassInfoPtrFromName(NOMClassMgrObject, sci->chrParentClassNames[a], NULLHANDLE);
          /* now find the index */
          ulNumMethods=nomGetNumIntroducedMethods(ncPriv);
          for(b=0;b<ulNumMethods;b++)
            {
              //  nomPrintf("%d, checking %s\n", b, *ncPriv->sci->nomSMethods[b].nomMethodId);
              if(!strcmp(chrMethodName,*ncPriv->sci->nomSMethods[b].nomMethodId ))
                {
                  //  nomPrintf("   %s, %d: Found %s in %s, index: %d\n", __FUNCTION__, __LINE__,
                  //        gstr->str, sci->chrParentClassNames[a], b);
                  idx+=b;
                  bFound=TRUE;
                  break;
                }
            }/* for(b) */
        }
      else
        {
          /* Class not yet found... */
          NOMClassPriv *ncPriv;

          if(NULL==NOMClassMgrObject){
            /* Things are getting dirty now. Someone tries overriding while we are
               bootstrapping. That may be either NOMClass or NOMClassMgr. Let's climb
               the list by hand now... */
            gulong aLoop, numMethods;

            /* We only support overriding of NOMObject for now when no manager is
               available. a is the index into the array of class names. a=0 is the
               first name which is always NOMObject. */
            if(0!=a)
              g_error("%s line %d: No Quark. No class manager and attempt to override a class which is not NOMObject\n(method: %s, current class: %s)!\n",
                      __FUNCTION__, __LINE__, gstr->str, sci->chrParentClassNames[a]);
            ncPriv=pGlobalNomEnv->ncpNOMObject;
            
            numMethods=nomGetNumIntroducedMethods(ncPriv);
            for(aLoop=0;aLoop<numMethods;aLoop++)
              {
                //nomPrintf("%d, checking %s\n", aLoop, *ncPriv->sci->nomSMethods[aLoop].nomMethodId);
                if(!strcmp(chrMethodName,*ncPriv->sci->nomSMethods[aLoop].nomMethodId ))
                  {
                    //nomPrintf("   %s, %d: Found %s in %s (hand search), index: %d\n", __FUNCTION__, __LINE__,
                    //        gstr->str, sci->chrParentClassNames[a], aLoop);
                    idx=aLoop+1;
                    bFound=TRUE;
                    break;
                  }
              }/* for(aLoop) */
          }
          else{
            ncPriv=_nomGetClassInfoPtrFromName(NOMClassMgrObject, sci->chrParentClassNames[a], NULLHANDLE);
            //nomPrintf("   %s did not introduce the method. Adding %d to index\n",
            //        sci->chrParentClassNames[a], nomGetNumIntroducedMethods(ncPriv)+1);
            idx+=nomGetNumIntroducedMethods(ncPriv)+1; /* classObject pointer */
          }/* NOMClassMgrObject */
        }
      g_string_free(gstr, TRUE);
      if(bFound)
        break;
    }/* for(a) */

  if(bFound)
    return idx;
  else
    return 0;
}

/*
  
 */
void priv_resolveOverrideMethods(NOMClassPriv *nClass, nomStaticClassInfo *sci)
{
  if(sci->ulNumStaticOverrides) {
    int b;

#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf(" %d: %d method(s) to override\n", __LINE__, sci->ulNumStaticOverrides);
#endif

    /* There're some methods to override */
    for(b=0;b<sci->ulNumStaticOverrides;b++) {/* For every overriden method do */
      nomMethodProc** entries;
      ULONG index;

      entries=&nClass->mtab->entries[0];  /* Adress of array where we enter our resoved method */

#ifdef DEBUG_NOMBUILDCLASS
      nomPrintf(" %d: Going to override \"%s\"\n", __LINE__, *sci->nomOverridenMethods[b].nomMethodId);
#endif

      index=priv_getIndexOfMethodInEntries(nClass, sci, *sci->nomOverridenMethods[b].nomMethodId);
      if(0==index){
        g_warning("%s line %d:\n   We are supposed to override \"%s\" but the method can't be found!", 
                  __FUNCTION__, __LINE__, *sci->nomOverridenMethods[b].nomMethodId);
        return;
      }
      /* This is the parent method adress which will be used by the overriding class */
      *sci->nomOverridenMethods[b].nomParentMethod=entries[index];
      /* Using the found index we insert our new method address into the mtab. */
      entries[index]=sci->nomOverridenMethods[b].nomMethod;
    } /* for(b) */  
  }/* if(sci->numStaticOverrides) */

#ifdef DEBUG_NOMBUILDCLASS
  else
    nomPrintf(" No methods to override\n");
#endif
}


void fillCClassDataStructParentMtab(nomStaticClassInfo *sci, NOMClassPriv *nClass, NOMClass *nomClass)
{
  /* Insert pointer into CClassDataStructure */
  sci->ccds->parentMtab=&nClass->parentMtabStruct;  /* Insert pointer into CClassDataStructure */

  /* Fill somParentMtabStruct in CClassDataStructure */
  sci->ccds->parentMtab->mtab=nClass->mtab;         /* This class mtab */
  sci->ccds->parentMtab->next=nClass->mtabList.next;
  sci->ccds->parentMtab->nomClassObject=nomClass;        /* Class object    */
  sci->ccds->parentMtab->ulInstanceSize=nClass->mtab->ulInstanceSize;
  /* C Class data structure */
}


/*
  - This function builds the nomMethodTab of class nClass.
  - Calculates the method thunks
  - Calculates the instance variable thunks using the parent class info

  It takes a parent class ncpParent which may also have introduced instance variables and
  methods. 
 */
void addMethodAndDataToThisPrivClassStruct(NOMClassPriv* nClass, NOMClassPriv* ncpParent, nomStaticClassInfo *sci) 
{
  BYTE * mem;
  int a;

BUILDNOMCLASS_ENTER

  nClass->sci=sci;   /* Save static class info for internal use */

  /* Copy assembler thunking code for instance data */
  memcpy(nClass->thunk, thunk, sizeof(thunk));
  /* Link parent mtab in */
  nClass->mtabList.next=&ncpParent->mtabList;    

  /* Fill all the pointers to the methodtable we need */
  nClass->mtab=(nomMethodTab*)&nClass->thisMtab;              /* thisMtab is the address where our mtab starts */
  nClass->mtabList.mtab= (nomMethodTab*)&nClass->thisMtab;
  nClass->parentMtabStruct.mtab=(nomMethodTab*)&nClass->thisMtab;
  
#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("nClass->mtabList.next: %x\n", nClass->mtabList.next);
#endif
  
  /* There're some parents. Copy parent mtab data into new mtab. */
  /* Copy object data. This all goes at the address of "nomMethodProc* entries[0]" 
    entries[] contain copies of the ClassDataStruct and thus the proc addresses of the static methods.
    */
  mem=(char*)nClass->mtab; /* Target address */
  memcpy(mem, ncpParent->mtab, ncpParent->mtab->mtabSize); /* copy parent mtab with all proc addresses */
#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf("copy parent data: %d (mtabSize) from %x (mtab of %s, taken from NOMClassPriv) to %x (mtab to build)\n", 
              ncpParent->mtab->mtabSize,
              sci->nomCds, ncpParent->mtab->nomClassName, mem);
#endif

  mem=((char*)nClass->mtab) + ncpParent->mtab->mtabSize; /* points right after the parent mtab now in 
                                                           our private struct */
  /* Add class struct of this class. This includes the proc adresses. */
  if(sci->ulNumStaticMethods) {
#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf("copy own data: %d (classptr+numProcs*procpointersize) from %x (cds, classDataStruct of %x) to %x (our mtab part)\n", 
              sizeof(NOMClass*)+sci->ulNumStaticMethods*sizeof(nomMethodProc*),
              sci->nomCds, sci->nomCds->nomClassObject, mem);
#endif
    nClass->entries0=(NOMClass**)mem; /* Our part in the mtab starts here. We need this position to insert the class object pointer
                                         later. */
    memcpy( mem, sci->nomCds, sizeof(NOMClass*)+sci->ulNumStaticMethods*sizeof(nomMethodProc*));

    /* Now finally put the thunking in so the procedures are resolved correctly. */
    for(a=0;a<sci->ulNumStaticMethods;a++) {
      ULONG ulOffset;

      memcpy(&nClass->mThunk[a], mThunkCode, sizeof(mThunkCode)); /* Copy thunking code */

      ulOffset=(ULONG)((char*)(mem+sizeof(NOMClass*))-(char*)nClass->mtab); /* Skip class object pointer */
      nClass->mThunk[a].thunk[2]=((ulOffset+a*sizeof(nomMethodProc*))<<8)+0xa2;
      /* Put thunking code address into CClassStruct */
       sci->nomCds->nomTokens[a]=(void*)&nClass->mThunk[a];
    }
  }
  /* Thunking code see above. Adjust the offset to the instance data so the right
     variables are accessed. The offset is different depending on the number of parent
     classes (which isn't fix) and the number of parent instance vars (which isn't known
     at compile time) */
  nClass->thunk[1]=(ncpParent->mtab->ulInstanceSize<<8)+0x05; //0x00000405

  sci->ccds->instanceDataToken=&nClass->thunk;

  BUILDNOMCLASS_LEAVE
}

/*
  This function is called when asking for information about the parent of a class.
  It checks if NOMClassMgrObject is already built and if yes asks it for the class.
  If it's not yet built we are still in the bootstrapping stage and the only parent
  we may search is NOMObject. Replacing of NOMObject is not supported by NOM.   
 */
static NOMClassPriv* priv_getClassFromName(gchar* chrClassName)
{
  NOMClassPriv *ncpParent=NULL;

  if(NULL==NOMClassMgrObject){
    /* If we build NOMClass or NOMClassMgr we just use the pointer we saved before. */
    if(!strcmp(chrClassName, "NOMObject"))
      ncpParent=pGlobalNomEnv->ncpNOMObject;
    else
      g_error("Asking for a parent not being NOMObject while NOMClassMgrObject is NULL. This can't be!!!");
  }/* NULL== NOMClassMgrObject */
  else
    {
      ncpParent=_nomGetClassInfoPtrFromName(NOMClassMgrObject, chrClassName,
                                  NULLHANDLE);
    }

  return ncpParent;
}

/*
  This function creates a private class structure for an object from the given sci with 
  correctly resolved methods which holds the mtab. This struct is kept by the metaclass
  and holds all the information for creating objects.

  nomClass: class object
  */
static NOMClassPriv * NOMLINK priv_buildPrivClassStruct(long inherit_vars,
                                                        nomStaticClassInfo *sci,
                                                        long majorVersion,
                                                        long minorVersion,
                                                        NOMClass* nomClass)
{
  ULONG ulParentDataSize=0;
  ULONG mtabSize;
  ULONG ulMemSize=0;
  NOMClassPriv  *nClass, *ncpParent;

  DBGBUILDNOMCLASS_ENTER

  if(!nomClass||!sci)
    return NULLHANDLE;

  /* The addresse of static methods in sci are already resolved. See nomBuildClass() */

  /* Get parent class if any */
  if(NULL!=sci->chrParentClassNames){
    ncpParent=priv_getClassFromName(sci->chrParentClassNames[sci->ulNumParentsInChain]);
  }/* nomIdAllParents */
  else
    ncpParent = NULLHANDLE;

  if(!ncpParent) {
    g_warning("We are supposed to create a NOMClassPriv but there's no parent!\n");
    /* FIXME:       
       Maybe we should panic here.  */
    return NULLHANDLE; /* Only SOMObject has no parent, so we have a problem here. */
  }

  /* Calculate size of new class object */
  ulMemSize=sizeof(NOMClassPriv)-sizeof(nomMethodTab); /* start size class struct */

#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("  ncpParent->mtab->mtabSize: %d. Parent is: %x (priv) %s\n",
            ncpParent->mtab->mtabSize, ncpParent, ncpParent->mtab->nomClassName);
#endif
  mtabSize=ncpParent->mtab->mtabSize+sizeof(nomMethodProc*)*(sci->ulNumStaticMethods)+
    sizeof(NOMObject*);/* numStaticMethods is correct here!
                          NOT numStaticMethods-1!
                          entries[0] in fact contains the 
                          class pointer not a method
                          pointer. */  
  ulMemSize+=mtabSize; /* add place for new procs and the new class pointer */
  ulParentDataSize=ncpParent->mtab->ulInstanceSize; /* Parent instance size */
#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("  %s: mtabSize will be: %d, ulParentDataSize is: %d\n", __FUNCTION__, mtabSize, ulParentDataSize);
#endif
  /* Alloc object struct using NOMCalloc. */
  if((nClass=(NOMClassPriv*)NOMCalloc(1, ulMemSize))==NULLHANDLE)
    return NULLHANDLE;

  /* Get mem for method thunking code */
  nClass->mThunk=NOMMalloc(sizeof(nomMethodThunk)*sci->ulNumStaticMethods);
  if(!nClass->mThunk) {
    NOMFree(nClass);
    return NULLHANDLE;
  }

  /* Add class struct of this class.
     This includes 
     -resolving the new method adresses
     -add the parent mtab info to the new built mtab
  */
  addMethodAndDataToThisPrivClassStruct( nClass, ncpParent, sci) ;

#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("%s:  mtab: %x\n", __FUNCTION__, nClass->mtab);
#endif
  /*
    We don't create a class object here so the following isn't done:
    sci->cds->classObject=somClass;
  */

  /* Resolve ovverrides if any */
  priv_resolveOverrideMethods(nClass, sci);

  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  nClass->mtab->mtabSize=mtabSize;
  nClass->mtab->nomClassObject=nomClass; /* Class object (metaclass). We build a normal class here. */
  nClass->mtab->nomClsInfo=(nomClassInfo*)nClass;
#warning !!!!! Change this when nomId is a GQuark !!!!!
  nClass->mtab->nomClassName=*sci->nomClassId;
  nClass->mtab->ulInstanceSize=sci->ulInstanceDataSize+ulParentDataSize; /* Size of instance data of this class and all
                                                                            parent classes. This isn't actually allocated for this class
                                                                            object but the entry is used when creating the objects. */
  fillCClassDataStructParentMtab(sci, nClass, nomClass);

#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("  New NOMClassPriv*: %x\n", nClass);
#endif

  _nomSetObjectCreateInfo(nomClass, nClass, NULLHANDLE);

  DBGBUILDNOMCLASS_LEAVE

  return nClass;
}


NOMClass * NOMLINK priv_buildWithExplicitMetaClass(glong ulReserved,
                                                    nomStaticClassInfo *sci,
                                                    gulong majorVersion,
                                                    gulong minorVersion)
{
  NOMClass *nomClass, *nomClassParent;
  
  if(NULL==NOMClassMgrObject)
    return NULLHANDLE;

  /* Search for meta class. */
#warning !!!!! Change this when nomID is a GQuark !!!!!
  nomClassParent=_nomFindClassFromName(NOMClassMgrObject, *sci->nomExplicitMetaId, majorVersion, minorVersion, NULLHANDLE);
  
  if(!nomClassParent)
    return NULLHANDLE;

  /* Create a new class object. We create a copy here because we may change the mtab entries
     through overriding or two classes may use the same meta class but have different
     sizes, methods etc. I wonder how IBM SOM manages to use the same metaclass
     for different classes without (apparently) copying it for different uses... */
  if((nomClass=(NOMClass*)NOMCalloc(1, _nomGetSize(nomClassParent, NULLHANDLE)))==NULLHANDLE)
    return NULLHANDLE;

  
  /* Mabe we should just copy the whole struct here? */
  nomClass->mtab=nomClassParent->mtab;
#warning !!!!! No call of _nomSetInstanceSize  !!!!!
#warning !!!!! No call of _nomSetObjectsSCI   !!!!!
#if 0
  /* Set object data */
  _nomSetInstanceSize(nomClass, _nomGetSize(nomClassParent));
  /* Save objects sci pointer. We need it when we create instances  */
  _nomSetObjectsSCI(somClass, sci);
#endif

  /* Update static class data of class to be build */
  sci->nomCds->nomClassObject=nomClass;

  /* Now we have a meta class. Create the NOMClassPriv* holding the mtab for the class
     to be built (which is not a meta class). */
  priv_buildPrivClassStruct(ulReserved, sci,
                            majorVersion, minorVersion,
                            nomClass);
  
#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("New class Object (SOMClass): %x \n", nomClass);
#endif

  /* nomClassReady() is called in nomBuildClass() */
  return nomClass;
}


/*
  This function is called when a class for a given sci should be build with a parent
   which isn't derived from SOMClass. In that case SOMClass is the class to be used for
   the class object. This doesn't mean we reuse the SOMClass structs. Instead a new
   copy is created so individula overriding is possible.
*/
static
NOMClass * NOMLINK priv_buildWithNOMClassAsMeta(gulong ulReserved,
                                                nomStaticClassInfo *sci,
                                                long majorVersion,
                                                long minorVersion)
{
  NOMClass  *nomClass, *nomClassDefault;
  NOMClassPriv *nClass;

#ifdef DEBUG_NOMBUILDCLASS
#warning !!!!! Change this when nomId is a GQuark !!!!!
  nomPrintf("\n\n\nEntering %s to build %s\n", __FUNCTION__, *sci->nomClassId);
#endif

  /**** Create the meta class object ****/
  nomClassDefault=pGlobalNomEnv->defaultMetaClass; // this gives a NOMClass* not a NOMClassPriv*

  if(!nomClassDefault)
    return NULLHANDLE;

  /* Found NOMClass object */

  //nomPrintf("_nomGetSize(): %d\n", _nomGetSize(nomClassParent, NULLHANDLE));

  /* Create an object */
  if((nomClass=(NOMClass*)NOMCalloc(1, _nomGetSize(nomClassDefault, NULLHANDLE)))==NULLHANDLE)
    return NULLHANDLE;

  nomClass->mtab=nomClassDefault->mtab;

#warning !!!!! _nomSetInstanceSize() not called here !!!!!
#warning !!!!! _nomSetObjectsSCI() not called here !!!!!
#if 0
  /* Set object data */
  _nomSetInstanceSize(somClass, _nomGetSize(nomClassDefault));
  /* Save objects sci pointer. We need it when we create instances  */
  _nomSetObjectsSCI(nomClass, sci);
#endif

  /* Update static class data of the class to be built */
  sci->nomCds->nomClassObject=nomClass;

  /* Now we have a meta class. Create the NOMClassPriv* holding the mtab for the class
     to be built. */
  nClass=priv_buildPrivClassStruct(ulReserved, sci,
                                   majorVersion, minorVersion,
                                   nomClass);

#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("%s: New class Object (NOMClass): %x NOMClassPriv*: %x\n", __FUNCTION__, nomClass, nomClass->mtab->nomClsInfo);
  _dumpMtab(nomClass->mtab);
  _dumpObjShort(nomClass);
#endif

  /* nomClassReady() is called in nomBuildClass(), so don't call it here. Same goes for _nomInit(). */
  return nomClass;
}


/*
  nomClass:  NOMClass
  nClass: NOMClassPriv for NOMClass
  ulSize: size of a NOMClass object

  This function is used to add a meta class to NOMOBject which was built before.
*/
NOMClass*  createNOMObjectClassObjectAndUpdateNOMObject(NOMClass* nomClass, NOMClassPriv* nClass, gulong ulSize)
{
  NOMClassPriv *ncp;
  NOMClass  *nomObjClass;
  
  DBGBUILDNOMCLASS_ENTER
    
  /* The NOMClassPriv for NOMObject */
  ncp= pGlobalNomEnv->ncpNOMObject;  

  /* Allocate a class object for NOMObject for creating NOMObject instances. */
  if((nomObjClass=(NOMClass*)NOMCalloc(1, ulSize))==NULLHANDLE) {
    /* We panic here for the simple reason that without a working NOMObject the whole object system
       will not work. */
    g_error("No memory for building the class object _NOMObject for NOMObject.");
    return NULLHANDLE;
  }

  nomObjClass->mtab=nomClass->mtab;         /* Now it's an object */

  _nomSetObjectCreateInfo(nomObjClass, pGlobalNomEnv->ncpNOMObject, NULLHANDLE); /* This NOMClassPriv holds all info to build 
                                                                                    instances of NOMObject (not that anybody
                                                                                    should do that but... */
#warning !!!!!!!!!!!!! _somSetObjectsSCI() not called!!!!!!!!!!!!!!!!
#warning !!!!!!!!!!!!! _somSetClassData() not called!!!!!!!!!!!!!!!!
#if 0
  _somSetObjectsSCI(nomObjClass, ncp->sci);
  _somSetClassData(somObjClass, scp->sci->cds);
#endif

  /* Update NOMObject data */
  ncp->mtab->nomClassObject=nomObjClass;         /* This is the real NOMClass pointer, not a pointer to NOMClassPriv */
  ncp->mtab->entries[0]=(void*)nomObjClass;

  /* Put it into the class data of NOMObject */
  ncp->sci->nomCds->nomClassObject=nomObjClass;

#ifdef DEBUG_BUILDNOMOBJECT
    nomPrintf("%d: metaclass for NOMObject created: %x\n",__LINE__, nomObjClass);
#endif
    DBGBUILDNOMCLASS_LEAVE
    return nomObjClass;
}


NOMEXTERN NOMClass * NOMLINK nomBuildClass(gulong ulReserved,
                                           nomStaticClassInfo *sci,
                                           gulong ulMajorVersion,
                                           gulong ulMinorVersion)
{
  NOMClass *nomClass;
  NOMClassPriv *nClass;
  NOMClassPriv *ncpParent;
  ULONG ulParentDataSize=0;
  ULONG mtabSize;
  ULONG ulMemSize=0;
  int a;

#ifdef DEBUG_NOMBUILDCLASS
  nomParentMtabStructPtr pParentMtab;
  nomMethodTabs psmTab;
  /* Print some info for debbuging */
  nomPrintf("\n%d: Entering %s to build class %s. ---> SCMO: %x (NOMClassManagerObject)\n",
            __LINE__, __FUNCTION__, *sci->nomClassId, NULL /*NOMClassMgrObject*/);
  nomPrintf("cds: %x nomClassObject: %x\n", sci->nomCds, sci->nomCds->nomClassObject);
#endif

  /* Check if already built */
  if(sci->nomCds->nomClassObject) {
#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf("%d: Class %s already built. returning 0x%x\n", __LINE__, *sci->nomClassId, sci->nomCds->nomClassObject);
#endif
    return (sci->nomCds)->nomClassObject; /* Yes,return the object */
  }

  /* Do we want to build again NOMObject the mother of all classes?
     This happens because every class automatically tries to build the parents
     (which includes NOMObject somewhere). NOMObject doesn't have a class object
     yet when created
     so we have to check here the global pointer to the NOMObject private data.
  */

  if(!strcmp(*sci->nomClassId, "NOMObject")){
    if(pGlobalNomEnv->ncpNOMObject!=NULLHANDLE){
#ifdef DEBUG_NOMBUILDCLASS
      nomPrintf("%d: Class %s already built. returning 0x%x\n", __LINE__, *sci->nomClassId, sci->nomCds->nomClassObject);
#endif
      /* FIXME: this seems to be broken!! */
      return NULLHANDLE; /* SOMObject already built */
    }
  }

#ifdef _DEBUG_NOMBUILDCLASS
  nomPrintf("%d: Dumping sci:\n", __LINE__);
  _dumpSci(sci);
#endif

  if(sci->ulNumStaticMethods!=0 && !sci->nomSMethods){
    nomPrintf("  !!! %s line %d: sci->nomSMethods is NULL for %s !!!\n", __FUNCTION__, __LINE__, *sci->nomClassId);
    return NULLHANDLE;
  }
  /* Fill static classdata with the info from nomStaticMethodDesc array. This means
     putting the method adresses into the classdata struct.
     Be aware that the order of the methods in the somStaticMethod_t array is not
     necessarily in the order as specified in the releaseorder list. But each array member
     (which is a nomStaticMethodDesc) contains the address of the correct
     field in the static classdata struct. */
  /* This also resolves methods for objects not only class objects. Be careful when
     removing here. You have to correct priv_buildObjecttemplate(). */
  /* Insert class data structure method tokens */
  for(a=0;a<sci->ulNumStaticMethods;a++) {
    *sci->nomSMethods[a].nomMAddressInClassData=(void*)sci->nomSMethods[a].nomMethod; /*  Address to place the resolved function address in see *.ih files. */
#ifdef _DEBUG_NOMBUILDCLASS
    nomPrintf("  static method: %s, %lx\n", *sci->nomSMethods[a].chrMethodDescriptor, sci->nomSMethods[a].nomMethod);
    nomPrintf("%d: %d: method: %x %s (classdata addr. %x) (Fill static class struct with procs)\n", 
              __LINE__, a, sci->nomSMethods[a].nomMethod,  *sci->nomSMethods[a].nomMethodId, sci->nomSMethods[a].nomMAddressInClassData);
#endif
  }

#ifdef _DEBUG_NOMBUILDCLASS
  nomPrintf("%d: Dumping the filled classdata structure:\n", __LINE__);
  _dumpClassDataStruct(sci->nomCds, sci->ulNumStaticMethods);
#endif

  /* Do we want to build SOMObject the mother of all classes? */
  if(!strcmp(*sci->nomClassId, "NOMObject")){
#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf("%d: Trying to build  %s\n", __LINE__, *sci->nomClassId);
#endif
    priv_buildNOMObjectClassInfo(ulReserved, sci, /* yes */
                                 ulMajorVersion, ulMinorVersion);
    return NULLHANDLE; /* We can't return a SOMClass for SOMObject because SOMClass isn't
                          built yet. */
  }

  /* Do we want to build NOMClass? */
  if(!strcmp(*sci->nomClassId, "NOMClass"))
    return priv_buildNOMClass(ulReserved, sci, /* yes */
                              ulMajorVersion, ulMinorVersion);

  /* Get parent class */
  if(sci->nomIdAllParents) {
#ifdef DEBUG_NOMBUILDCLASS
#warning !!!!! Change this when nomId is a GQuark !!!!!
    nomPrintf("%d: About to search parent %s...\n", __LINE__, **(sci->nomIdAllParents));
#endif

    ncpParent=priv_getClassFromName(sci->chrParentClassNames[sci->ulNumParentsInChain]);

#if 0
    ncpParent=priv_getClassFromName(**(sci->nomIdAllParents));
    ncpParent=priv_findPrivClassInGlobalClassListFromName(pGlobalNomEnv,
                                                          **(sci->nomIdAllParents)); /* This may also return a class not derived
                                                                                        from SOMClass */
#endif
    if(!ncpParent)
      return NULLHANDLE; /* Every class except NOMObject must have a parent!! */
  }/* nomIdAllParents */
  else
    return NULLHANDLE; /* Every class except NOMObject must have a parent!! */

  if(!ncpParent)
    return NULLHANDLE; /* Every class except NOMObject must have a parent!! */

#ifdef DEBUG_NOMBUILDCLASS
  /* Do some debugging here... */
  nomPrintf("%d: Found parent private class info struct. Dumping parentMTabStruct...\n", __LINE__);
  pParentMtab=&ncpParent->parentMtabStruct;
  nomPrintf("     parent class: %s (priv %x), pParentMtab: %x, pParentMtab->mtab %x, next: %x\n", ncpParent->mtab->nomClassName,
            ncpParent, pParentMtab, pParentMtab->mtab, pParentMtab->next);
  /* climb parent list */
  psmTab=pParentMtab->next;
  while(psmTab) {
    nomPrintf("     next class: %s, next: %x\n", psmTab->mtab->nomClassName, psmTab->next);
    psmTab=psmTab->next;
  }
#endif /* DEBUG_NOMBUILDCLASS */


  /* Check if parent is a class object (derived from NOMClass). */
  if(!priv_nomIsA((NOMObject*)ncpParent, pGlobalNomEnv->defaultMetaClass)) {
    /* No parent is normal object so we have either to use NOMClass as parent
       or an explicit meta class if given. */
#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf("Class %x (ncpParent->mtab->nomClassName: %s) is not a NOMClass\n", ncpParent, ncpParent->mtab->nomClassName);
#endif

    if(sci->nomExplicitMetaId)
      {
        /* The explicit metaclass is created at this point. Now it will be filled
           with the info how to create objects.
           sClass=(SOMClassPriv*)priv_findClassInClassList(pGlobalSomEnv, *(sci->explicitMetaId));
           
           if(!scParent)
           return NULLHANDLE;  Every class except SOMObject must have a parent!! */
#ifdef DEBUG_NOMBUILDCLASS
        nomPrintf("sci->nomExplicitMetaId is set\n");
#endif
        


        nomClass= priv_buildWithExplicitMetaClass(ulReserved, sci,
                                                  ulMajorVersion, ulMinorVersion);
        if(nomClass){
#ifdef DEBUG_NOMBUILDCLASS
          nomPrintf("%s: class is %x\n", nomClass->mtab->nomClassName, nomClass);
#endif    
          _nomInit(nomClass, NULLHANDLE);
          _nomClassReady(nomClass, NULLHANDLE);
        }

        return nomClass;
      }/* nomExplicitMetaId */
    else {
      /* Use NOMClass as meta class. The following call will create the
         class object and will also fill in the necessary object info for
         creating instances. */
      nomClass= priv_buildWithNOMClassAsMeta(ulReserved, sci,
                                             ulMajorVersion, ulMinorVersion);

      if(nomClass){
        //#warning !!!!! No call of  _nomClassReady() here !!!!!
        //#if 0
        _nomInit(nomClass, NULLHANDLE);
        _nomClassReady(nomClass, NULLHANDLE);   
        //#endif
      }
      return nomClass;
    }
  }/* NOMClass derived? */

  /* Child of some NOMClass */


  /**** From this point we are building a new class object (derived from NOMClass ****/
  ulMemSize=sizeof(NOMClassPriv)-sizeof(nomMethodTab); /* start size class struct */

  /* Calculate size of new class object */
#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("Parent class %x (ncpParent->mtab->nomClassName: %s) is a NOMClass (or derived)\n",
            ncpParent, ncpParent->mtab->nomClassName);
  nomPrintf("ncParent->mtab->mtabSize: %d\n", ncpParent->mtab->mtabSize);
#endif
  mtabSize=ncpParent->mtab->mtabSize+sizeof(nomMethodProc*)*(sci->ulNumStaticMethods)+sizeof(NOMClass*);/* ulNumStaticMethods is correct here!
                                                                                                        NOT numStaticMethods-1!
                                                                                                        entries[0] in fact contains the 
                                                                                                        class pointer not a method
                                                                                                        pointer. */
  ulMemSize+=mtabSize; /* add place for new procs and the new class pointer */
  ulParentDataSize=ncpParent->mtab->ulInstanceSize; /* Parent instance size */
  
#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("%s mtabSize is: %d, ulParentDataSize is: %d\n", *sci->nomClassId, mtabSize, ulParentDataSize);
#endif


  /* Alloc class struct using SOMCalloc. This means the struct is allocated in shared mem */
  if((nClass=(NOMClassPriv*)NOMCalloc(1, ulMemSize))==NULLHANDLE)
    return NULLHANDLE;

  /* Get mem for method thunking code */
  nClass->mThunk=NOMMalloc(sizeof(nomMethodThunk)*sci->ulNumStaticMethods);
  if(!nClass->mThunk) {
    NOMFree(nClass);
    return NULLHANDLE;
  }

  nClass->ulClassSize=sci->ulInstanceDataSize+ulParentDataSize;

  if((nomClass=(NOMClass*)NOMCalloc(1, sci->ulInstanceDataSize+ulParentDataSize))==NULLHANDLE) {
    NOMFree(nClass->mThunk);
    NOMFree(nClass);
    return NULLHANDLE;
  }

  nClass->ulPrivClassSize=ulMemSize;

  
  /* Add class struct of this class. This includes resolving the method adresses. */
  addMethodAndDataToThisPrivClassStruct( nClass, ncpParent, sci) ;

  /* Resolve ovverrides if any */
  //#warning !!!!! no resolving of overriden methods here !!!!!
  priv_resolveOverrideMethods(nClass, sci);

  nomClass->mtab=nClass->mtab;  
#ifdef DEBUG_nOMBUILDCLASS
  nomPrintf("mtab: %x\n", nClass->mtab);
#endif
  sci->nomCds->nomClassObject=nomClass; /* Put class pointer in static struct */

  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  nClass->mtab->mtabSize=mtabSize;
  nClass->mtab->nomClassObject=nomClass;
  nClass->mtab->nomClsInfo=(nomClassInfo*)nClass;
  nClass->mtab->nomClassName=*sci->nomClassId;
  nClass->mtab->ulInstanceSize=sci->ulInstanceDataSize+ulParentDataSize; /* Size of instance data of this class and all
                                                                        parent classes. This isn't actually allocated for this class
                                                                        object but the entry is used when creating the objects. */
  fillCClassDataStructParentMtab(sci, nClass, nomClass);

  /* Thunking see above */
  nClass->thunk[1]=(ulParentDataSize<<8)+0x05; //0x00000405
  sci->ccds->instanceDataToken=&nClass->thunk;

  /* Set this class size into instance var */
#warning !!!!! No call of _nomSetInstanceSize() here !!!!!
  // _nomSetInstanceSize(nomClass, sci->ulInstanceDataSize+ulParentDataSize);

#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("New class ptr (class object): %x (NOMClassPriv: %x) for %s\n", nomClass, nClass, *sci->nomClassId);
#endif

  //priv_addPrivClassToGlobalClassList(pGlobalNomEnv, nClass);
  _nomInit(nomClass, NULLHANDLE);
  _nomClassReady(nomClass, NULLHANDLE);
  return nomClass;
};


/*********************************************************************************************************************/
/*     Unused stuff */
/*********************************************************************************************************************/



#if 0
#include <cwsomcls.h>
#include <somclassmanager.h>
/********************************************************/
/*  Internal functions                                  */
/********************************************************/

/* String manager functions */
static BOOL priv_addSomIdToIdList(PSOM_ENV pEnv, somIdItem * sid)
{
  somIdItem *tmpList;

  if(!sid || !pEnv)
    return FALSE;

  if(NO_ERROR != priv_requestSomEnvMutex(pEnv))
     return FALSE;

  if(!pEnv->livingSomIds)
    {
      /* Add first class */
      pEnv->livingSomIds=sid;
      priv_releaseSomEnvMutex(pEnv);
      return TRUE;
    }
  tmpList=sid;
  
  tmpList->next=(somIdItem*)pEnv->livingSomIds;
  pEnv->livingSomIds=tmpList;
  pEnv->ulNumRegIds++;

  priv_releaseSomEnvMutex(pEnv);
  return TRUE;
}

/* This returns the ID of a string, this means the hash of this string if
   already registered. */
static somIdItem* priv_findSomIdInList(PSOM_ENV pEnv, string aString)
{
  somIdItem *tmpItem;
  ULONG ulHash;

  if(!pEnv|| aString)
    return 0;

  ulHash=calculateNameHash(aString);

  if(NO_ERROR != priv_requestSomEnvMutex(pEnv))
    return NULL;

  tmpItem=pEnv->livingSomIds;
  while(tmpItem)
    {
      if(tmpItem->id == ulHash) {
        priv_releaseSomEnvMutex(pEnv);
        return tmpItem;  /* We have found the string return somId */
      }
      tmpItem=tmpItem->next;
    };
  priv_releaseSomEnvMutex(pEnv);

  return NULL; /* No id yet. */
}

/********************************************************/
/*   Toolkit functions, exported                        */
/********************************************************/

/*
  Caller is responsible for freeing the returned somId with SOMFree.
  Note: this is not the internal id (hash) of a string!

  FIXME:
  This function must be checked if it's correct.
 */

somId SOMLINK somIdFromString (string aString)
{
  /* This call automatically registers the ID with the runtime */
  somIdItem *sid;
  somId sID;

  if(!aString)
    return NULL;

  /* Do we already have an ID at all? */
  sid=priv_findSomIdInList(pGlobalSomEnv, aString);

  if(sid) {
    sID=SOMMalloc(sizeof(void*));
    if(!sID)
      return NULLHANDLE;

    *sID=(char*)sid;
    return sID;
  }

  /* No somId registered  yet, so create one */
  if((sid=(somIdItem*)SOMCalloc(1, sizeof(somIdItem)))==NULLHANDLE)
    return NULLHANDLE;

  sid->idString=SOMMalloc(strlen(aString)+1);
  if(!sid->idString)
    {
      SOMFree(sid);
      return NULLHANDLE;
    }

  sid->id=calculateNameHash(aString);

  strcpy(sid->idString, aString);
  if(!priv_addSomIdToIdList(pGlobalSomEnv, sid)) {
    SOMFree(sid->idString);
    SOMFree(sid);
    return NULLHANDLE;
  }

  sID=SOMMalloc(sizeof(void*));
  if(!sID)
    return NULLHANDLE;

  *sID=(char*)sid;
  return sID;
}

/* Returns the total number of ids that have been registered so far, */
unsigned long SOMLINK somTotalRegIds(void)
{
  long numIds;

  if(NO_ERROR != priv_requestSomEnvMutex(pGlobalSomEnv))
    return 0;

  numIds=pGlobalSomEnv->ulNumRegIds;
  priv_releaseSomEnvMutex(pGlobalSomEnv);

  return numIds;
}

/*
  FIXME: semaphores!!!!!
 */
/*
  This function tries to find the class introducing the method 'sid' at first.
 */
static SOMClassPriv* priv_getOverrideClass(somId *sid)
{
  char* chrPtr;
  SOMClassPriv *scp;
  ULONG ulLen;
  char *chrMem;

  if(!sid)
    return NULLHANDLE;

  if((chrPtr=strchr(**sid, ':'))==NULLHANDLE)
    return NULLHANDLE;

  /* Create local copy */
  ulLen=strlen(**sid);
  if(ulLen>5000) /* prevent stack overflow in case of error */
    return NULLHANDLE;

  chrMem=alloca(ulLen);
  strcpy(chrMem, **sid);

  if((chrPtr=strchr(chrMem, ':'))==NULLHANDLE)
    return NULLHANDLE; /* How should that happen, but anyway... */

  *chrPtr=0; /* Now we have separated the class name */
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: searching override for %s\n", __LINE__, __FUNCTION__, chrMem);
#endif
  scp=priv_findPrivClassInGlobalClassList(pGlobalSomEnv, chrMem);
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: found %x\n", __LINE__, __FUNCTION__, scp);
#endif
  if(!scp)
    return NULLHANDLE;
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: found %x (SOMClassPriv) ->%x (SOMClass)\n", __LINE__, __FUNCTION__, scp, scp->mtab->classObject);
#endif
  return scp;
}

/*
  FIXME: semaphores!!!!!
 */
/*
  This function tries to find the class introducing the method 'sid' at first.
  It returns a SOMClass not a SOMClassPriv.
 */
static SOMClass* priv_getOverrideSOMClass(somId *sid)
{
  char* chrPtr;
  SOMClassPriv *scp;
  ULONG ulLen;
  char *chrMem;

  if(!sid)
    return NULLHANDLE;

  if((chrPtr=strchr(**sid, ':'))==NULLHANDLE)
    return NULLHANDLE;

  /* Create local copy */
  ulLen=strlen(**sid);
  if(ulLen>5000) /* prevent stack overflow in case of error */
    return NULLHANDLE;

  chrMem=alloca(ulLen);
  strcpy(chrMem, **sid);

  if((chrPtr=strchr(chrMem, ':'))==NULLHANDLE)
    return NULLHANDLE; /* How should that happen, but anyway... */

  *chrPtr=0; /* Now we have separated the class name */
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: searching override for %s\n", __LINE__, __FUNCTION__, chrMem);
#endif
  scp=priv_findPrivClassInGlobalClassList(pGlobalSomEnv, chrMem);
  somPrintf("%d: %s: found %x (SOMClassPriv)\n", __LINE__, __FUNCTION__, scp);
  if(!scp)
    return NULLHANDLE;
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: found %x (SOMClassPriv) ->%x\n", __LINE__, __FUNCTION__, scp, scp->mtab->classObject);
#endif
  return scp->mtab->classObject;
}

/*
  This function finds the class which introduced the method (methodId) to be overriden. It gets the index
  in the mtab of that class and returns it. Using this index the correct method address is taken
  from the mtab of the parent class of the class which wants to override a method (sClass). By using the
  parent instead the original class (introducing this method in the beginning) any overriding
  done in a class subclassing the introducing class is automatically taken into account.
 */
/*
  FIXME: semaphores ????
 */
static ULONG priv_getIndexOfMethodToBeOverriden(somId *methodId )
{
  return 0;
}

/*
  Class format:

  struct _SOMClass {
  struct somMethodTabStruct  *mtab;
  struct somClassInfo;
  struct somMethodTabListStruct;
  struct somParentMtabStruct;               Struct to place parent mtab pointer
  ULONG  thunk[3];                       Assembler thunking code
  somMethodTabStruct mtabStruct;         See beloe
  ClassDataStruct class1;
  ClassDataStruct class2;
  ClassDataStruct class3;
  ...
  };
  
  -- Object Instance Structure:
  
  struct somMethodTabStruct;
  typedef struct SOMAny_struct {
  struct somMethodTabStruct  *mtab;
  integer4 body[1];
  } SOMAny;

  The following struct was built by CW
  struct somClassInfo
  {
  SOMClass        *classObject;       <- This is a pointer to the SOMObject class object in SOM.
  }

typedef struct somMethodTabStruct {
    SOMClass        *classObject;
    somClassInfo    *classInfo; 
    char            *className;
    long            instanceSize;
    long            dataAlignment;
    long            mtabSize;
    long            protectedDataOffset; / from class's introduced data
    somDToken       protectedDataToken;
    somEmbeddedObjStruct *embeddedObjs;
    / remaining structure is opaque /
    somMethodProc* entries[1];           <-- I found that this isn't correct (or I misseed something in the includes). When dumping a mtab
                                             the following has this structure:
                                   
                                             SOMClass         *classObject; /The first class object (SOMObject)
                This is basically a copy ->  somMethodProc*   firstMethod_1;
                of the ClassDataStruct       somMethodProc*   secondMethod_1;
                                             ...
                                             SOMClass         *classObject; /The second class object
                ClassDataStruct of 2.    ->  somMethodProc*   firstMethod_2; 
                class                        somMethodProc*   secondMethod_2;
                        
} somMethodTab, *somMethodTabPtr;
*/

/*
  Build a "SOMObject class" usable for building other class objects e.g. SOMClass
  (base for all class objects). Right after SOMObject is constructed, it will be used only
  for holding the created mtab structure which contains all the resolved procedure
  pointers. This mtab is referenced by the SOMObject and thus by SOMClass (and all classes
  to come).
  SOMObject will get a full featured class object later when SOMClass is built.


  Is this still correct???:

  Within this function a "real" SOMObject will be created which will be used when
  creating normal objects derived from SOMObject but not from a class object.
  This is necessary because the class structure and the object structure are different
  so we need two different templates. The class created by this function is only used
  for holding data necessary for building SOMClass. This class pointer will not be used
  for anything else.
 */
SOMClass * SOMLINK priv_buildSOMObject(long inherit_vars,
                                       somStaticClassInfo *sci,
                                       long majorVersion,
                                       long minorVersion)
{
  BYTE * mem;
  SOMClassPriv *sClass; /* This struct holds our private data. A pointer will be in mtab->classInfo */
  SOMClass *somClass;   /* A real SOMClass pointer */
  int a;
  ULONG mtabSize;
  ULONG ulMemSize=0;
  ULONG ulParentDataSize=0;

#ifdef DEBUG_BUILDSOMOBJECT
  somPrintf("%d: Entering %s to build a temporary SOMObject class object\n", __LINE__, __FUNCTION__);
  somPrintf("%d: Entering %s to build the mtab for SOMObjects (pEnv->mtabSOMObject)\n", __LINE__, __FUNCTION__);
  _dumpSci(sci);
#endif

  /* Note: SOMObject has no parents */
  return NULLHANDLE;
  
  /* ulMemsize will be the size of our private class structure SOMClassPriv */
  ulMemSize=sizeof(SOMClassPriv)-sizeof(somMethodTab); /* start size class struct without the somMethodTab
                                                          holding the method pointers. The size of this
                                                          somMethodTab will be added later. */

  /* Calculate the size of the method tab to be added to the size of the private class struct */
  mtabSize=sizeof(somMethodTab)+sizeof(somMethodProc*)*(sci->numStaticMethods);/* numStaticMethods is correct here! NOT 
                                                                                  numStaticMethods-1! entries[0] in fact
                                                                                  contains the class pointer not a method
                                                                                  pointer. */
  ulMemSize+=mtabSize; /* Add size of base mtab struct */

  /* Alloc private class struct using SOMCalloc. */
  if((sClass=(SOMClassPriv*)SOMCalloc(1, ulMemSize))==NULLHANDLE)
    return NULLHANDLE;

  /* Get mem for method thunking code. This assembler code is needed so the indirect
     jump to the methods from the object pointer which is known does work. For each class
     an individual thunking code must be calculated because the number of instance
     variables is not defined. */
  sClass->mThunk=SOMMalloc(sizeof(cwMethodThunk)*sci->numStaticMethods);
  if(!sClass->mThunk) {
    SOMFree(sClass);
    return NULLHANDLE; 
  }

  /* The size of each instance of this class. A SOM object has a method tab pointer
     at the beginning followed by the instance variables. */
  sClass->ulClassSize=sci->instanceDataSize+sizeof(somMethodTab*);

  sClass->sci=sci;                                   /* Save static class info for internal use          */
  sClass->ulPrivClassSize=ulMemSize;                 /* Internal housekeeping. Not needed by SOM methods */
  memcpy(sClass->thunk, thunk, sizeof(thunk));       /* Copy assembler thunking code for instance data   */

  /* Fill all the pointers to methodtable we need in the *private* structure */
  sClass->mtab=(somMethodTab*)&sClass->thisMtab;            /* create the mtab pointer and store it           */
  sClass->mtabList.mtab= (somMethodTab*)&sClass->thisMtab;  /* thisMtab is the position where the mtab starts */
  sClass->parentMtabStruct.mtab=(somMethodTab*)&sClass->thisMtab;

  /* And now the real SOMClass struct which will be seen by the user. A SOMClass has a mTab pointer
     at the beginning and the instance data following. */
  if((somClass=(SOMClass*)SOMCalloc(1, sci->instanceDataSize+sizeof(somMethodTab*)))==NULLHANDLE) {
    SOMFree(sClass->mThunk);
    SOMFree(sClass);
    return NULLHANDLE;
  }
  somClass->mtab=sClass->mtab;

#ifdef DEBUG_BUILDSOMOBJECT
  somPrintf("mtab: %x sClass: %x, somClass: %x\n", sClass->mtab, sClass, somClass);
#endif

  /*
    FIXME: this somClass must be deleted after updating with the real metaclass!
   */
  /*
    We don't have a class object yet...
  */
  // sci->cds->classObject=somClass; /* Put class pointer in static struct */

  /* Copy class data. This goes at the address of "somMethodProc* entries[0]".
     Entries[] contain copies of the ClassDataStruct and thus the proc addresses of the static methods.
     We don't use the static classDataStruct directly because subclasses will override the proc addresses. */
  mem=(char*)&(somClass->mtab->entries[0]); /* Target address.entries[0] will contain the class pointer */

  /* Add class struct of this class. This includes the proc adresses. */
  if(sci->numStaticMethods) {
#ifdef DEBUG_BUILDSOMOBJECT
    somPrintf("copy: %d (classptr+numProcs*procpointersize) from %x (cds, classDataStruct) to %x\n", 
              sizeof(SOMClass*)+sci->numStaticMethods*sizeof(somMethodProc*),
              sci->cds, mem);
#endif
    /* Copy classDataStruct with the resolved proc addresses */
    memcpy( mem, sci->cds, sizeof(SOMClass*)+sci->numStaticMethods*sizeof(somMethodProc*));

    /* Now finally put the thunking in so the procedures are resolved correctly. */
    for(a=0;a<sci->numStaticMethods;a++) {
      ULONG ulOffset;

      memcpy(&sClass->mThunk[a], mThunkCode, sizeof(mThunkCode));               /* Copy method thunking code template  */
      ulOffset=(ULONG)((char*)(mem+sizeof(SOMClass*))-(char*)somClass->mtab);   /* Skip priv class data pointer        */
      sClass->mThunk[a].thunk[2]=((ulOffset+a*sizeof(somMethodProc*))<<8)+0xa2; /* Calculate offset for assembler code */
#ifdef DEBUG_BUILDSOMOBJECT
      somPrintf(" %d: %d : Thunk offset: %d (0x%x) -> address will be: %x\n",
                __LINE__, a, ulOffset, ulOffset, mem+ulOffset+a*sizeof(somMethodProc*) );
#endif
      /* Put thunking code address into CClassStruct */
      sci->cds->tokens[a]=(void*)&sClass->mThunk[a];
    } /* for */
  } /* if(numStaticMethods) */

  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  sClass->mtab->mtabSize=mtabSize;     /* This mtab is the same as the one used in the real SOMClass */

#if 0
  /* We don't have a class object yet. */
  sClass->mtab->classObject=somClass; /* This is the real SOMClass pointer, not a pointer to SOMClassPriv */
#endif

  sClass->mtab->classInfo=(somClassInfo*)sClass;  /* FIXME: I think I may just use this undocumented field for the private data. */
  sClass->mtab->className=*sci->classId;
  sClass->mtab->instanceSize=sci->instanceDataSize+sizeof(somMethodTabPtr); /* sizeof(methodTabStruct*) + size of instance data of this class
                                                                               and all parent classes. This is SOMObject so we have no parents. */
  /*
    FIXME:   
    The following is not yet initialized (and the previous may be buggy...) */
  //    long	    dataAlignment;
  //    long	    protectedDataOffset; /* from class's introduced data */
  //    somDToken	    protectedDataToken;
  //    somEmbeddedObjStruct *embeddedObjs;

  sci->ccds->parentMtab=&sClass->parentMtabStruct;  /* Insert pointer into CClassDataStructure */

  /* Fill somParentMtabStruct in CClassDataStructure */
  sci->ccds->parentMtab->mtab=sClass->mtab;         /* This class mtab                               */
  sci->ccds->parentMtab->next=NULL;                 /* We dont have parents because we are SOMObject */
  sci->ccds->parentMtab->classObject=somClass;      /* SOMClass* Class object, this means ourself    */
  sci->ccds->parentMtab->instanceSize=sClass->mtab->instanceSize;
  /* C Class data structure */

  /* Thunking code see above. Adjust the offset to the instance data so the right
     variables are accessed. The offset is different depending on the number of parent
     classes (which isn't fix) and the number of parent instance vars (which isn't known
     at compile time) */
  sClass->thunk[1]=(ulParentDataSize<<8)+0x05; //0x00000405
  sci->ccds->instanceDataToken=&sClass->thunk;

#ifdef DEBUG_BUILDSOMOBJECT
  somPrintf("New class ptr (temp. class object for SOMObject): %x (SOMClassPriv: %x) for %s\n",
            somClass, sClass, *sci->classId);

  somPrintf("%d: Dumping the filled classdata structure:\n", __LINE__);
  _dumpClassDataStruct(sci->cds, sci->numStaticMethods);
#endif
#ifdef DEBUG_OBJECTS
  _dumpMTabListPrivClass(sClass);
#endif

  /* SOMObject is special because it's root of normal classes and meta classes. Because of this it
     isn't insert into the normal private meta class list. We also use this info to check if SOMObject
     was already built in other places. */
  pGlobalSomEnv->scpSOMObject=sClass;

  priv_addPrivClassToGlobalClassList(pGlobalSomEnv, sClass);

#ifdef DEBUG_OBJECTS
  _dumpObj(somClass);
#endif

  /*
    We don't run the somInit() method here because the SOMObject class isn't yet completely built.
    First a SOMClass meta class must be created because SOMObject needs a SOMClass as the class object like
    any other class. In case some code in somInit() tries to access the class object initialization would
    fail. In the function building the root SOMClass a metaclass for SOMObject will be created and attached.

    Not running somInit() here shouldn't be a problem because we know what we are doing ;-).
    If there will ever be the need for special initialization for this very first SOMObject we think again...

    In any case fiddling with somInit() in SOMObject and SOMClass is a dangerous thing because at least one of the
    two classes won't be completely built at that point in time (because SOMClass is a subclass of SOMObject
    and SOMObject needs a SOMClass as metaclass).
 
    _somInit(somClass);
  */
  return somClass;
}
#endif
