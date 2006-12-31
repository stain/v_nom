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
  This file contains functions for working with the garbage collector.
 */

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

/* Garbage collector */
#include <gc.h>

gboolean bUseGC=FALSE; /* MArk if we use the garbage collector */

static gpointer  gcMalloc(gulong ulBytes)
{
  //printf("Hi there...\n");
  // return malloc(ulBytes);
  return (gpointer) GC_malloc(ulBytes);
}

static gpointer  gcRealloc(gpointer mem, gulong ulBytes)
{
  // printf("...and here\n");
  //  return realloc(mem, ulBytes);
  return (gpointer) GC_realloc(mem, ulBytes); 
}

static void  gcFree(gpointer mem)
{
  //  printf("free(): %x\n", mem);
  return;
  GC_free(mem); 
}


/*
  This is called from the EMX wrapper to set the garbage collector
  memory functions as the GLIB default allocation function.
 */
void _System  nomInitGarbageCollection(void* pMemInExe)
{
 GMemVTable vtbl={0};

 /* Init the garbage collector */
 GC_init();

 vtbl.malloc=(gpointer)gcMalloc;
 vtbl.realloc=(gpointer)gcRealloc;
 vtbl.free=(gpointer)gcFree; 

 g_mem_set_vtable(&vtbl);
 fprintf(stderr, "   GC memory functions set for GLIB. (%s: %d)\n", __FILE__, __LINE__);

 bUseGC=TRUE;
}

NOMEXTERN void NOMLINK  nomRegisterDataAreaForGC(char* pStart, char* pEnd)
{
  GC_add_roots(pStart, pEnd);
}

