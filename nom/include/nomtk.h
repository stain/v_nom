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

#ifndef NOMTK_H_INCLUDED
#define NOMTK_H_INCLUDED

/* The shared memory holding all the objects */
#define SOMTK_SHARED_MEM_SIZE_POOL	0x100000  /* 1Mb */

/* This holds the som environment, e.g. heads of the object lists... */
#define SOMTK_SHARED_MEM_SIZE_ROOT   sizeof(NOM_ENV)
#define SOMTK_SHARED_MEM_NAME_ROOT   "\\SHAREMEM\\CWSOMTK\\ROOT"


/* That's the base structure of all the SOM stuff */
typedef struct _nomEnv {
  ULONG cbSize;      /* Size of this struct */
  PVOID pMemPool;    /* Shared memory heap for sub alloc */
  HMTX  hmtx;        /* Mutex sem to protect this structure */
  ULONG ulNumRegIds; /* Number of registered somIDs */
  NOMClassPriv  *ncpNOMObject;  /* This is for NOMObject*/
  nomClasses livingMetaClasses; /* List of created meta classes. */
  NOMClass *defaultMetaClass;   /* This is a pointer to the NOMClass object */
  NOMClass *nomObjectMetaClass; /* Metaclass of NOMObject */
}NOM_ENV;
typedef NOM_ENV *PNOM_ENV;

/* Redefine SOMFree(), SOMCalloc(), SOMMalloc */
#define SOMFree   NOMFree
#define SOMMalloc NOMMalloc
#define SOMCalloc NOMCalloc


NOMEXTERN PNOM_ENV NOMLINK nomTkInit(void);

NOMEXTERN nomToken NOMLINK NOMMalloc(const ULONG size);
NOMEXTERN boolean NOMLINK NOMFree(const nomToken memPtr);
NOMEXTERN nomToken NOMLINK  NOMCalloc(const ULONG num, const ULONG size);
NOMEXTERN gboolean NOMLINK nomIsObj(NOMObject * nomObj);
NOMEXTERN int NOMLINK nomPrintf(string chrFormat, ...);
NOMEXTERN NOMClassMgr * NOMLINK nomEnvironmentNew (void);
NOMEXTERN void NOMLINK dumpClasses(void);

/* Functions used by nomBuildClass() */
ULONG priv_requestSomEnvMutex(PNOM_ENV pEnv);
ULONG priv_releaseSomEnvMutex(PNOM_ENV pEnv);
BOOL priv_addPrivClassToGlobalClassList(PNOM_ENV pEnv, NOMClassPriv * nClass);
NOMClassPriv* priv_findPrivClassInGlobalClassListFromName(PNOM_ENV pEnv, char* nClass);

NOMClass * NOMLINK priv_buildNOMClass(NOM_ulong ulReserved,
                                      nomStaticClassInfo *sci,
                                      NOM_ulong majorVersion,
                                      NOM_ulong minorVersion);
NOMClassPriv * NOMLINK priv_buildNOMObjectClassInfo(NOM_ulong ulReserved,
                                                    nomStaticClassInfo *sci,
                                                    NOM_ulong majorVersion,
                                                    NOM_ulong minorVersion);
void addMethodAndDataToThisPrivClassStruct(NOMClassPriv* nClass, NOMClassPriv* ncpParent, nomStaticClassInfo *sci);
NOMClass*  createNOMObjectClassObjectAndUpdateNOMObject(NOMClass* nomClass, NOMClassPriv* nClass, NOM_ulong ulSize);
void fillCClassDataStructParentMtab(nomStaticClassInfo *sci, NOMClassPriv *nClass, NOMClass *nomClass);
void priv_resolveOverrideMethods(NOMClassPriv *nClass, nomStaticClassInfo *sci);

/* Debug function */
void _dumpClassDataStruct(nomClassDataStructure* cds, ULONG ulNumMethods);
void _dumpSci(nomStaticClassInfo* sci);
void  _dumpMtab(nomMethodTab* mtab);
void  _dumpObjShort(NOMObject* sObj);
void _dumpClasses();

#if 0
ULONG calculateNameHash(char * theString);
/* Debugging functions */


void _dumpStaticMTab(somStaticMethod_t* smt, ULONG ulMethod);
SOMEXTERN void SOMLINK _dumpParentMTab(somParentMtabStructPtr pMtabPtr);
void _dumpParentMTabList(somParentMtabStructPtr pMtabPtr);
SOMEXTERN void SOMLINK _dumpObj(SOMObject* sObj);
void _dumpMethodTabList(somMethodTabList *mtabList);
void SOMLINK _dumpMtab(somMethodTab* mtab);
SOMEXTERN void SOMLINK _dumpMTabListPrivClass(SOMClassPriv *sClass);
SOMEXTERN void SOMLINK _dumpObjShort(SOMObject* sObj);
#endif

#endif /* NOMTK_H_INCLUDED */




















