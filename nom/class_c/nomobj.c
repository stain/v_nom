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
#include <gtk/gtk.h>

#include "nom.h"
#include "nomtk.h"

#include "nomobj.ih"


NOM_Scope void  NOMLINK impl_NOMObject_nomInit(NOMObject *nomSelf, CORBA_Environment *ev)
{  
  //nomPrintf("    Entering %s (%x) with nomSelf: 0x%x. nomSelf is: %s.\n",
  //        __FUNCTION__, impl_NOMObject_nomInit, nomSelf , nomSelf->mtab->nomClassName);
}

NOM_Scope void  NOMLINK impl_NOMObject_nomUninit(NOMObject *nomSelf, CORBA_Environment *ev)
{
  /* NOMObjectData *nomThis = NOMObjectGetData(nomSelf); */
  
  nomPrintf("    Entering %s (%x) with nomSelf: 0x%x. SomSelf is: %s.\n",
            __FUNCTION__, impl_NOMObject_nomUninit, nomSelf , nomSelf->mtab->nomClassName);
}

NOM_Scope CORBA_long NOMLINK impl_NOMObject_nomGetSize(NOMObject* nomSelf, CORBA_Environment *ev)
{
  //nomPrintf("    Entering %s (%x) with nomSelf: 0x%x. nomSelf is: %s.\n",
  //          __FUNCTION__, impl_NOMObject_nomGetSize, nomSelf , nomSelf->mtab->nomClassName);

  if(!nomSelf) {
    return 0;
  }

  return nomSelf->mtab->ulInstanceSize;
}

NOM_Scope void NOMLINK impl_NOMObject_delete(NOMObject* nomSelf, CORBA_Environment *ev)
{
/* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */

  /* Give object the chance to free resources */
  _nomUninit(nomSelf, NULLHANDLE);

  /* And now delete the object */
  /*
    FIXME: we should probably call a class function here, so the
    class can keep track of objects.
   */
  NOMFree(nomSelf);
}

NOM_Scope PNOMClass NOMLINK impl_NOMObject_nomGetClass(NOMObject* nomSelf, CORBA_Environment *ev)
{
/* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */

  return nomSelf->mtab->nomClassObject;
}

/*
  Create a new class of the kind the caller is. This method ensures that subclasses
  are properly handled without the need to override this class in every subclass.
 */
NOM_Scope PNOMObject NOMLINK impl_NOMObject_new(NOMObject* nomSelf, CORBA_Environment *ev)
{
/* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */
  PNOMObject nomRetval;
  NOMClass* nomCls;

  /* We don't know which class we're actually. So we can't just create a new object using
     <CkassName>New() here.
     It is possible that we are called by a subclass. So get the class object and let the
     class object create the correct class. */
  nomCls=NOMObject_nomGetClass(nomSelf, NULLHANDLE);
  return NOMClass_nomNew(nomCls, NULLHANDLE);
}



