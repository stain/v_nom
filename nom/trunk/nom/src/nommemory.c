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

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <nom.h>
#include <nomtk.h>

/* This tells us if memory allocation is done using the garbage collector */
extern gboolean bUseGC; /* Set during initialization */

NOMEXTERN nomToken NOMLINK NOMMalloc(ULONG size)
{
  gchar* memPtr;

  if((memPtr=g_malloc(size))==NULLHANDLE)
    return NULLHANDLE;

  return (nomToken) memPtr;

#if 0
  PULONG memPtr;

  if((memPtr=g_malloc(size+sizeof(gpointer)))==NULLHANDLE)
    return NULLHANDLE;

  *memPtr=size;
  memPtr++;
  return (nomToken) memPtr;
#endif
}

nomToken NOMLINK NOMCalloc(const ULONG num, const ULONG size)
{
  gchar* memPtr;

  if((memPtr=g_malloc(size*num ))==NULLHANDLE)
    return NULLHANDLE; /* We won't end here because GLib just terminates the process :-/ 
                          A really sick idea imho. */
  if(!bUseGC)
    memset(memPtr, 0, size*num); /* GC always returns zeroed memory */

  return (nomToken) memPtr;

#if 0
  PULONG memPtr;

  if((memPtr=g_malloc(size*num + sizeof(ULONG)))==NULLHANDLE)
    return NULLHANDLE; /* We won't end here because GLib just terminates the process :-/ 
                          A really sick idea imho. */
  if(!bUseGC)
    memset(memPtr, 0, size*num + sizeof(ULONG)); /* GC always returns zeroed memory */

  *memPtr=size;
  memPtr++;
  return (nomToken) memPtr;
#endif
}

/*
  Note: unlike as in SOM this one isn't replaceable.
 */
NOMEXTERN boolean NOMLINK NOMFree(const nomToken memPtr)
{
#if 0
  ULONG* pul=(PULONG)memPtr;

  pul--;
  g_free(pul);
#endif
  g_free(memPtr);
  return TRUE;
}

