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

#ifndef NOMAPI_H_INCLUDED
#define NOMAPI_H_INCLUDED

/* Method and data Tokens. */
typedef nomToken nomMToken;
typedef nomToken nomDToken;


#ifndef NOM_CLASSINFO_DEFINED
#define NOM_CLASSINFO_DEFINED
typedef nomToken nomClassInfo;
#endif

typedef struct nomMethodTabStruct {
  NOMClass	      *nomClassObject;    /* Pointer to the class object               */
  nomClassInfo    *nomClsInfo;        /* Pointer to NOMClassPriv. That struct holds
                                         thunking code and extended info.          */
  char            *nomClassName;      /* Pointer to this class' name               */
  gulong          ulInstanceSize;     /* Size of an instance of this class         */
  gulong          mtabSize;           /* Size of this mtab (includes method ptrs.) */
  nomMethodProc*  entries[1];
} nomMethodTab, *nomMethodTabPtr;

typedef struct nomMethodTabList {
  nomMethodTab             *mtab;
  struct nomMethodTabList  *next;
} nomMethodTabList, *nomMethodTabs;

typedef struct {
  nomMethodTab    *mtab;              /* This class' mtab */
  nomMethodTabs   next;               /* The parent mtabs */
  NOMClass	      *nomClassObject;
  gulong	      ulInstanceSize;
} nomParentMtabStruct, *nomParentMtabStructPtr;

typedef struct {
    NOMClass *nomClassObject;
    nomToken nomTokens[1];    /* method tokens, etc. */
} nomClassDataStructure, *NomClassDataStructurePtr;

typedef struct nomStaticMethodDescStruct {
  nomMToken *nomMAddressInClassData;
  nomID nomMethodId;
  char** chrMethodDescriptor;
  nomMethodProc *nomMethod;
} nomStaticMethodDesc;

typedef struct nomOverridenMethodDescStruct {
  nomID nomMethodId;
  nomMethodProc *nomMethod;
  nomMethodProc **nomParentMethod;
} nomOverridenMethodDesc;

typedef struct {
    nomParentMtabStructPtr parentMtab;
    nomDToken		   instanceDataToken;
} nomCClassDataStructure, *nomCClassDataStructurePtr;


/* This struct carries quite some dead entries (for us)... */
typedef struct nomStaticClassInfoStruct {
  gulong    ulVersion;
  gulong    ulNumStaticMethods;
  gulong    ulNumStaticOverrides;
  gulong    ulMajorVersion;
  gulong    ulMinorVersion;
  gulong    ulInstanceDataSize;
  gulong    ulNumParents;         /* Used for multiple inheritance */
  nomID     nomClassId;
  nomID     nomExplicitMetaId;

  nomClassDataStructure  *nomCds;
  nomCClassDataStructure *ccds;
  nomStaticMethodDesc*   nomSMethods;
  nomID                  *nomIdAllParents;
  char**     chrParentClassNames;
  gulong    ulNumParentsInChain;
  nomOverridenMethodDesc*    nomOverridenMethods;
} nomStaticClassInfo, *nomStaticClassInfoPtr;


typedef struct
{
  ULONG thunk[4];
}nomMethodThunk;

#define NOM_FLG_IS_METACLASS         0x00000001
#define NOM_FLG_NOMUNINIT_OVERRIDEN  0x00000002

/* This structure holds additional informationen about class not to be found in nomMethodTab.
   It holds the default method table of the class and the thunking code necessary to access
   data and methods.
*/
typedef struct 
{
  nomMethodTab  *mtab;                  /* This is the mtab for this class it points to thisMtab at the
                                           end (not mtab for objects created by this meta class)          */
  gulong            ulClassSize;        /* The size of an instance (mtab+ instance vars)                  */
  gulong        ulPrivClassSize;        /* The size of this private struct including mtab (not pointr but
                                           real filled structure. Do we need this?                         */
  // gulong        ulIsMetaClass;          /* Set to 1 if this is a metaclass                                 */
  gulong        ulClassFlags;           /* Set to 1 if this is a metaclass                                 */
  nomStaticClassInfo *sci;              /* Class description                                               */
  nomMethodTabList mtabList;            /* The (private) internal list of mtabs we maintain 
                                           struct nomMethodTabList {
                                           nomMethodTab             *mtab; /mtab for this class
                                           struct nomMethodTabList  *next; / parents
                                           } nomMethodTabList, *nomMethodTabs;                             */
  nomParentMtabStruct parentMtabStruct; /* used for method lookup and other stuff                          */
  gulong thunk[3];                      /* Thunking code to get the address of an instance var             */
  nomMethodThunk *mThunk;               /* The thunk code to call the methods of this class                */
  NOMClass**    entries0;               /* Address where our part of the mtab starts (classObject pointer) */
  nomMethodTab  thisMtab;               /* mtab structure of this class                                    */
}NOMClassPriv;

/* For holding a list of class objects */
typedef struct nomClassList {
  NOMClass   *cls;
  struct nomClassList *next;
} nomClassList, *nomClasses;

NOMEXTERN NOMClass * NOMLINK nomBuildClass (gulong ulReserved,
                                            nomStaticClassInfo *sci,
                                            gulong ulMajorVersion,
                                            gulong ulMinorVersion);

//#define nomIsObj(a) ((a)!= 0)
#endif /* NOMAPI_H_INCLUDED */
