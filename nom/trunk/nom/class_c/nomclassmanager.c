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
/*
 * And remember, phase 3 is near...
 */

#define INCL_DOS
#include <os2.h>
#include <string.h>
#include <gtk/gtk.h>


#include "nom.h"
#include "nomtk.h"

#include "nomclassmanager.ih"


NOM_Scope CORBA_Object NOMLINK impl_NOMClassMgr_nomFindClassFromId(NOMClassMgr* nomSelf,
                                                                   const CORBA_long classId,
                                                                   const CORBA_long ulMajorVersion,
                                                                   const CORBA_long ulMinorVersion,
                                                                   CORBA_Environment *ev)
{
  /*  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf); */
  CORBA_Object nomRetval;

  nomPrintf("%s: %s not implemented yet (line %d)\n", __FILE__, __FUNCTION__, __LINE__);
  nomRetval=NULL;
  return nomRetval;
}

NOM_Scope CORBA_Object NOMLINK impl_NOMClassMgr_nomFindClassFromName(NOMClassMgr* nomSelf, 
                                                                     const CORBA_char * className,
                                                                     const CORBA_long ulMajorVersion,
                                                                     const CORBA_long ulMinorVersion,
                                                                     CORBA_Environment *ev)
{
  CORBA_Object nomRetval=NULLHANDLE;
  nomMethodTab * mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  /* This is only for NOMClass objects */
  if(strchr(className, ':'))
    return NULLHANDLE;

  
  mtab=g_datalist_get_data(&_gdataClassList, className);
  if(mtab){
    NOMClassPriv* ncPriv;
    ncPriv=(NOMClassPriv*)mtab->nomClsInfo;
    if(1==ncPriv->ulIsMetaClass){
      //    nomPrintf("%s: found %s\n", __FUNCTION__, mtab->nomClassName);
      nomRetval=(CORBA_Object)ncPriv->sci->nomCds->nomClassObject;
    }

  }
  return nomRetval;
}

/*
  This function is called when a class is (accidently) removed from our class list.
  This may happen e.g. when a class is registered again using the same name. The old
  registration is removed then and the new inserted (which may be the very same data).
  When this happens this is presumably a bug:
 */
static void
priv_handleClassRemove(gpointer data)
{
  nomMethodTab* mtab;
  mtab=(nomMethodTab*)data;
  if(mtab)
    g_warning("%s: supposed to remove class %s mtab: %lx. This is probably a bug.\n",
              __FUNCTION__, mtab->nomClassName, (long)mtab);
}

/* We register mtabs as unique pointers to classes. It's possible to get every
   information from an mtab. */
NOM_Scope void NOMLINK impl_NOMClassMgr_nomRegisterClass(NOMClassMgr* nomSelf, const gpointer classMtab,
                                                         CORBA_Environment *ev)
{
  nomMethodTab* mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  mtab=(nomMethodTab*) classMtab;

  if(NULL==_gdataClassList)
    g_datalist_init(&_gdataClassList);

  g_datalist_set_data_full(&_gdataClassList, mtab->nomClassName, classMtab, priv_handleClassRemove);
  //  g_datalist_set_data_full(&_gdataClassList, mtab->nomClassName, classMtab, priv_handleClassRemove);
  //nomPrintf("%s: registering %lx, %s classList: %lx\n", __FUNCTION__, 
  //classMtab, mtab->nomClassName, _gdataClassList);
}




NOM_Scope PGData NOMLINK impl_NOMClassMgr_nomGetClassList(NOMClassMgr* nomSelf, CORBA_Environment *ev)
{
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);
  nomPrintf("    Entering %s  with nomSelf: 0x%x. nomSelf is: %s.\n",
            __FUNCTION__, nomSelf, nomSelf->mtab->nomClassName);

  return _gdataClassList;
}

NOM_Scope gpointer NOMLINK impl_NOMClassMgr_nomGetClassInfoPtrFromName(NOMClassMgr* nomSelf,
                                                                       const CORBA_char * className, CORBA_Environment *ev)
{
  nomMethodTab * mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  /* This is only for NOMClassPriv objects */
  if(strchr(className, ':'))
    return NULLHANDLE;

  mtab=g_datalist_get_data(&_gdataClassList, className);
  if(mtab)
    return mtab->nomClsInfo;
  else
    return NULLHANDLE;
}


/*
  This function is called when a method is (accidently) removed from our list.
  This may happen e.g. when a class is registered again using the same name. The old
  registration is removed then and the new inserted (which may be the very same data).
  When this happens this is presumably a bug:
 */
static void
priv_handleMethodRemoveFromList(gpointer data)
{
  nomMethodTab* mtab;
  mtab=(nomMethodTab*)data;
  if(mtab)
    g_warning("%s: supposed to remove method mtab: %lx. This is probably a bug.\n",
              __FUNCTION__, (long)mtab);
}

NOM_Scope void NOMLINK impl_NOMClassMgr_nomRegisterMethod(NOMClassMgr* nomSelf, 
                                                          const gpointer classMtab, 
                                                          const CORBA_char * chrMethodName,
                                                          CORBA_Environment *ev)
{
  nomMethodTab* mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  mtab=(nomMethodTab*) classMtab;

  if(NULL==_gdataMethodList)
    g_datalist_init(&_gdataMethodList);

  g_datalist_set_data_full(&_gdataMethodList, chrMethodName, classMtab, priv_handleMethodRemoveFromList);
  //g_datalist_set_data_full(&_gdataClassList, mtab->nomClassName, classMtab, priv_handleClassRemove);
  // nomPrintf("%s: registering %lx, %s methodList: %lx\n", __FUNCTION__, classMtab, chrMethodName, _gdataMethodList);

}




