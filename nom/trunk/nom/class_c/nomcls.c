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
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>


#include "nom.h"
#include "nomtk.h"

#include "nomcls.ih"
#include "nomclassmanager.h"

extern NOMClassMgr* NOMClassMgrObject;


NOM_Scope CORBA_Object NOMLINK impl_NOMClass_nomNew(NOMClass* nomSelf, CORBA_Environment *ev)
{
  NOMClassData* nomThis=NOMClassGetData(nomSelf);
  NOMClassPriv *ncp;
  string nObj;
  
  nomPrintf("    Entering %s (%x) with nomSelf: 0x%x. nomSelf is: %s.\n",
            __FUNCTION__, impl_NOMClass_nomNew, nomSelf, nomSelf->mtab->nomClassName);

  if(!nomSelf)
    return NULLHANDLE;

  //  nomPrintf("instanceVar: %x\n", _ncpObject);

  if(!_ncpObject)
    return NULLHANDLE;

  ncp=(NOMClassPriv*)_ncpObject;
  //  nomPrintf("_ncpObject: %x size %d \n", ncp, ncp->mtab->ulInstanceSize);

  if((nObj=_nomAllocate(nomSelf, ncp->mtab->ulInstanceSize, NULLHANDLE))==NULLHANDLE)
    return NULLHANDLE;

  /* _nomInit() is called in _nomRenew() */
  return _nomRenew(nomSelf, (CORBA_Object)nObj, NULLHANDLE); /* This will also init the object */
}

NOM_Scope CORBA_Object NOMLINK impl_NOMClass_nomRenewNoInit(NOMClass* nomSelf,
                                                            const CORBA_Object nomObj,
                                                            CORBA_Environment *ev)
{
  NOMClassPriv *ncp;
  NOMClassData *nomThis = NOMClassGetData(nomSelf);

  ncp=(NOMClassPriv*)_ncpObject;

  /* Docs say the memory is first zeroed */
  memset(nomObj, 0, ncp->mtab->ulInstanceSize);

  /* Set mtab so it's an object */
  ((NOMObject*)nomObj)->mtab=ncp->mtab;

  /* NO call to _nomInit() */

  /* Return statement to be customized: */
  return nomObj;
}

NOM_Scope CORBA_Object NOMLINK impl_NOMClass_nomRenew(NOMClass* nomSelf, const CORBA_Object nomObj, CORBA_Environment *ev)
{
  _nomRenewNoInit(nomSelf, nomObj, NULLHANDLE);

  /* And now give the object the possibility to initialize... */
  _nomInit((NOMObject*)nomObj, NULLHANDLE);
  
  return nomObj;
}


NOM_Scope CORBA_string NOMLINK impl_NOMClass_nomAllocate(NOMClass* nomSelf, const CORBA_long ulSize, CORBA_Environment *ev)
{
  NOMClassPriv *ncp;
  NOMClassData *nomThis = NOMClassGetData(nomSelf);

  ncp=(NOMClassPriv*)_ncpObject;
  //  nomPrintf("%s: size %d\n", __FUNCTION__, ulSize);
  return NOMMalloc(ulSize);
}

NOM_Scope CORBA_string NOMLINK impl_NOMClass_nomGetName(NOMClass* nomSelf, CORBA_Environment *ev)
{
  return nomSelf->mtab->nomClassName;
}


NOM_Scope void NOMLINK impl_NOMClass_nomDeallocate(NOMClass* nomSelf, const CORBA_char * memptr, CORBA_Environment *ev)
{
  NOMFree((nomToken)memptr);
}


NOM_Scope void NOMLINK impl_NOMClass_nomSetObjectCreateInfo(NOMClass* nomSelf, const gpointer ncpObject, CORBA_Environment *ev)
{
  NOMClassData* nomThis=NOMClassGetData(nomSelf);

  nomPrintf("    Entering %s  with nomSelf: 0x%x. nomSelf is: %s.\n",
            __FUNCTION__,  nomSelf, nomSelf->mtab->nomClassName);

  _ncpObject=ncpObject;
}

NOM_Scope gpointer NOMLINK impl_NOMClass_nomGetObjectCreateInfo(NOMClass* nomSelf, CORBA_Environment *ev)
{
  NOMClassData* nomThis=NOMClassGetData(nomSelf);

  return _ncpObject;
}

NOM_Scope void NOMLINK impl_NOMClass_nomClassReady(NOMClass* nomSelf, CORBA_Environment *ev)
{

  nomPrintf("    Entering %s  with nomSelf: 0x%x. nomSelf is: %s.\n",
            __FUNCTION__, nomSelf, nomSelf->mtab->nomClassName);

  /* Register the class object if not alread done */
  if(NULL!=NOMClassMgrObject)
    {
      /* First check if this class is already in the list. This is important
         because there may be several classes having the same metaclass (usually NOMClass).
         The only difference of these metaclasses are different object templates for creating
         the instances. So to prevent unnecessary growth of the list just insert one class
         of each type. This should be sufficient for now and the future.

         We insert the mtab pointer which is an anchor to all other class info we may
         be interested in. */

      /* FIXME: no use of version information, yet. */
      //nomPrintf("I'm here (%s): ", _nomGetName(nomSelf, NULLHANDLE));
      if(!_nomFindClassFromName(NOMClassMgrObject, _nomGetName(nomSelf, NULLHANDLE),
                              0, 0, NULLHANDLE))
        {
          NOMClassPriv* ncPriv;
          gulong a, ulNumIntroducedMethods;

          nomPrintf("%s: Metaclass not registered yet.\n", __FUNCTION__);

          /* No, not in the list, so register it */
          ncPriv=(NOMClassPriv*)nomSelf->mtab->nomClsInfo;
          ncPriv->ulIsMetaClass=1; /* Mark that we are a metaclass */
          _nomRegisterClass(NOMClassMgrObject, nomSelf->mtab, NULLHANDLE);
          /* Register all the methods this class introduces */
          ulNumIntroducedMethods=ncPriv->sci->ulNumStaticMethods;
          for(a=0;a<ulNumIntroducedMethods;a++)
            {
              if(*ncPriv->sci->nomSMethods[a].chrMethodDescriptor)
                _nomRegisterMethod(NOMClassMgrObject, nomSelf->mtab,
                                   *ncPriv->sci->nomSMethods[a].chrMethodDescriptor, NULLHANDLE);
            }
          /* Metaclass is registered. Register the object class this
             metaclass may create. */
          ncPriv=(NOMClassPriv*)_nomGetObjectCreateInfo(nomSelf, NULLHANDLE);//nomSelf->mtab->nomClsInfo;
          if(ncPriv){
#warning !!!!! NOMClass does not have this pointer, this is a bug !!!!!
            ncPriv->ulIsMetaClass=0; /* Mark that we are not a metaclass (should be 0 already) */
            _nomRegisterClass(NOMClassMgrObject, ncPriv->mtab, NULLHANDLE);

            /* Register all the methods this class introduces */
            ulNumIntroducedMethods=ncPriv->sci->ulNumStaticMethods;
            for(a=0;a<ulNumIntroducedMethods;a++)
              {
                if(*ncPriv->sci->nomSMethods[a].chrMethodDescriptor)
                  _nomRegisterMethod(NOMClassMgrObject, ncPriv->mtab,
                                     *ncPriv->sci->nomSMethods[a].chrMethodDescriptor, NULLHANDLE);
              }/* for(a) */
          }/* ncPriv */
        }
      else{
        NOMClassPriv* ncPriv;
        /* Metaclass is already registered. Register the object class this
           metaclass may create. */

        nomPrintf("%s: Metaclass already registered, registering normal object class now.\n", __FUNCTION__);

        ncPriv=(NOMClassPriv*)_nomGetObjectCreateInfo(nomSelf, NULLHANDLE);//nomSelf->mtab->nomClsInfo;
        ncPriv->ulIsMetaClass=0; /* Mark that we are not a metaclass (should be 0 already) */
        if(ncPriv){
          gulong a, ulNumIntroducedMethods;
          /* Register all the methods this class introduces */
          ulNumIntroducedMethods=ncPriv->sci->ulNumStaticMethods;
          for(a=0;a<ulNumIntroducedMethods;a++)
            {
              if(*ncPriv->sci->nomSMethods[a].chrMethodDescriptor)
                _nomRegisterMethod(NOMClassMgrObject, ncPriv->mtab,
                                   *ncPriv->sci->nomSMethods[a].chrMethodDescriptor, NULLHANDLE);
            }
          nomPrintf("%s %s \n", nomSelf->mtab->nomClassName, ncPriv->mtab->nomClassName);
          _nomRegisterClass(NOMClassMgrObject, ncPriv->mtab, NULLHANDLE);
        }
      }
    }
  else
    nomPrintf("%s: no NOMClassMgrObject yet.\n", __FUNCTION__);

}

NOM_Scope void NOMLINK impl_NOMClass_nomInit(NOMClass* nomSelf, CORBA_Environment *ev)
{
/* NOMClassData* nomThis=NOMClassGetData(nomSelf); */

  nomPrintf("    Entering %s  with nomSelf: 0x%x. nomSelf is: %s.\n",
            __FUNCTION__, nomSelf, nomSelf->mtab->nomClassName);

  //#if 0
  /* orbit-idl-c-stubs.c, VoyagerWriteProtoForParentCall line 84 */
  NOMClass_nomInit_parent(nomSelf,  ev);
  //#endif
}
