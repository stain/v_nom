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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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


/*
  Find a library record in the buffer filled by DosQuerySysState().
 */
static qsLrec_t* qsFindModuleRec(const HREGDLL hRegisterDLL,  USHORT hMod){
  qsLrec_t *       pModRec;
  int a=0;

  pModRec=hRegisterDLL->pLibRec;
  while(NULL!=pModRec)
    {
      a++;
      //            printf("%d Checking: %x -> %04X (%s)\n", a, pModRec, pModRec->hmte, pModRec->pName);

      if (NULLHANDLE==pModRec->pObjInfo   && pModRec->ctObj > 0)
        {
          pModRec->pObjInfo = (qsLObjrec_t*)((char*)pModRec
                                             + ((sizeof(qsLrec_t)
                                             + pModRec->ctImpMod * sizeof(short)
                                             + strlen((char*)pModRec->pName) + 1    /* filename */
                                             + 3) & ~3));
          pModRec->pNextRec = (void*)((char*)pModRec->pObjInfo
                                    + sizeof(qsLObjrec_t) * pModRec->ctObj);
        }
      if(pModRec->hmte==hMod)
        break;

      pModRec=(qsLrec_t *)pModRec->pNextRec;
    }
  return pModRec;
}


#define BUFSIZE 1024*1024
NOMEXTERN HREGDLL NOMLINK nomBeginRegisterDLLWithGC(void)
{
  ULONG rc;
  HREGDLL hReg=NULLHANDLE;
  PTIB     ptib;
  PPIB     ppib;
  char *  buf;

  rc = DosGetInfoBlocks(&ptib, &ppib);
  if (rc!=NO_ERROR)
    return NULLHANDLE;

  buf = malloc(BUFSIZE);
  if(!buf)
    return NULLHANDLE;

  memset(buf,0,BUFSIZE);

  rc = DosQuerySysState(QS_PROCESS | QS_SEMAPHORE | QS_MTE | QS_FILESYS | QS_SHMEMORY ,
                        QS_MTE, /*0x96*/ ppib->pib_ulpid , 1UL, (PCHAR)buf, BUFSIZE);
  if (rc==NO_ERROR) {
    hReg=(qsPtrRec_t*) buf;
  }
  else
    free(buf);

  return hReg;
}

NOMEXTERN void NOMLINK nomEndRegisterDLLWithGC(const HREGDLL hRegisterDLL )
{
  free((char*)hRegisterDLL);
}


/*
  FIXME:

  This function will not find every given DLL because it doesn't follow every import of
  each DLL.
  It's only meant for registering the GTK+, GLIB and friends DLLs. This works because GTK2.DLL
  is directly loaded by the exe and all the friends DLLs are imported by it.

  Feel free to make this function really useful...

  Oh, and some refactoring would be nice, too.

 */
#define OBJREAD         0x0001L
#define OBJWRITE        0x0002L
#define OBJINVALID      0x0080L
NOMEXTERN BOOL NOMLINK nomRegisterDLLByName(const HREGDLL hRegisterDLL, const char* chrDLLName)
{
  qsPrec_t * p;
  int a=0;

  printf("Trying to register DLL %s\n", chrDLLName);

  p=hRegisterDLL->pProcRec;
  while(p && p->RecType == 1)
    {
      a++;
      if (p->cLib) {
        int i;

        for (i=0; i<p->cLib; i++){
          qsLrec_t * pModRec;

          //printf("%d %04X (p: %04x %04X, %04X) ",i, p->pLibRec[i], p, &p->pLibRec[i], &p->pLibRec);

          pModRec=qsFindModuleRec(hRegisterDLL,  p->pLibRec[i]);
          if(pModRec){
            //  printf("DLL name: %s\n", pModRec->pName);
            if(NULLHANDLE!=strstr( pModRec->pName, chrDLLName))
              {
                qsLObjrec_t  *pObjInfo;
                printf("    --> Found DLL %s\n", pModRec->pName);
                pObjInfo=pModRec->pObjInfo;
                if(NULLHANDLE!=pObjInfo)
                  {
                    int iObj;
                    for(iObj=0; iObj<pModRec->ctObj ;iObj++)
                      {
                        if (!(pObjInfo[iObj].oflags & OBJWRITE)) continue;
                        if (!(pObjInfo[iObj].oflags & OBJREAD)) continue;
                        if ((pObjInfo[iObj].oflags & OBJINVALID)) continue;
                        printf("    #%d: %04lX, size: %04lX %04lX\n",
                               iObj, pObjInfo[iObj].oaddr, pObjInfo[iObj].osize, pObjInfo[iObj].oflags); 
                        nomRegisterDataAreaForGC((char*)pObjInfo[iObj].oaddr,
                                                 (char*)(pObjInfo[iObj].oaddr+pObjInfo[iObj].osize));

                      }
                  }
                return TRUE;
              }
            /* Check the imports of this DLL if any */
            if(pModRec->ctImpMod >0)
              {
                int iImps;
                PUSHORT   pImpHmte;

                pImpHmte=(PUSHORT)((void*)pModRec + sizeof(qsLrec_t));

                for(iImps=0; iImps < pModRec->ctImpMod; iImps++)
                  {
                    qsLrec_t * pModImp;
                    // printf("  Trying import #%d (%04X)\n", iImps, pImpHmte[iImps]);
                    pModImp=qsFindModuleRec(hRegisterDLL,  pImpHmte[iImps]);
                    if(pModImp){
                      //printf("  DLL name: %s\n", pModImp->pName);
                      if(NULLHANDLE!=strstr( pModImp->pName, chrDLLName))
                        {
                          qsLObjrec_t  *pObjInfo;
                          printf("    --> Found DLL %s\n", pModImp->pName);
                          pObjInfo=pModImp->pObjInfo;
                          if(NULLHANDLE!=pObjInfo)
                            {
                              int iObj;
                              for(iObj=0; iObj<pModImp->ctObj ;iObj++)
                                {
                                  if (!(pObjInfo[iObj].oflags & OBJWRITE)) continue;
                                  if (!(pObjInfo[iObj].oflags & OBJREAD)) continue;
                                  if ((pObjInfo[iObj].oflags & OBJINVALID)) continue;
                                  
                                  printf("    #%d: %04lX, size: %04lX %04lX\n",
                                         iObj, pObjInfo[iObj].oaddr, pObjInfo[iObj].osize, pObjInfo[iObj].oflags); 
                                  nomRegisterDataAreaForGC((char*)pObjInfo[iObj].oaddr,
                                                           (char*)(pObjInfo[iObj].oaddr+pObjInfo[iObj].osize));
                                }
                            }
                          return TRUE;
                        }
                    }/* for() */
                  }/* for()*/
              }/* if() */
          }
        }/* For() */
      }
      break;
    }
  return NULLHANDLE;
}
