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
* Portions created by the Initial Developer are Copyright (C) 2007
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
#include <os2.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include <glib/gprintf.h> 

#define INCL_FILE
#include "parser.h"

extern GScanner *gScanner;

static void emitHFileHeader(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  fprintf(fh, "/*\n * This file was generated by the NOM IDL compiler for Voyager - DO NOT EDIT!\n");
  fprintf(fh, " *\n *\n * And remember, phase 3 is near...\n */\n");
  fprintf(fh, "/*\n * %s\n */\n", pif->chrSourceFileName);

  /* Protective #ifndef for whole file */
  fprintf(fh, "#ifndef %s_H\n#define %s_H\n\n", pif->chrName, pif->chrName);

  fprintf(fh, "#include <glib.h>\n");
  fprintf(fh, "#include <nomcls.h> /* This is needed for _nomNew() */\n\n");

  fprintf(fh, "#ifdef __cplusplus\n");
  fprintf(fh, "extern \"C\" {\n");
  fprintf(fh, "#endif /* __cplusplus */\n\n");

  /* Define the name as an object */
  fprintf(fh, "#if !defined(_%s_defined)\n", pif->chrName);
  fprintf(fh, "#define _%s_defined 1\n", pif->chrName);
  fprintf(fh, "#ifndef %s\n", pif->chrName);
  fprintf(fh, "#define %s NOMObject\n", pif->chrName);
  fprintf(fh, "typedef %s *P%s;\n", pif->chrName, pif->chrName);
  fprintf(fh, "#endif\n");
  fprintf(fh, "#endif\n\n");

}

/**
   Returns the interface structure (holding all the interface information) of the
   parent of an interface.

   \Param pif Pointer to an interface structure.
   \Returns The interface data structure of the parent interface or NULL if the
   interface has no parent.

 */
static PINTERFACE getParentInterface(PINTERFACE pif)
{
  if(pif->chrParent==NULL)
    return NULL;

  return findInterfaceFromName(pif->chrParent);
}

static void emitParentHeader(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  PINTERFACE pifParent=getParentInterface(pif);

  /* Include header of parent */
  if(pifParent){
    char* chrTemp=strlwr(g_strdup(pifParent->chrName));
    fprintf(fh, "/* Include for the parent class */\n");
    fprintf(fh, "#include \"%s.h\"\n\n", chrTemp);
    g_free(chrTemp);
  }
}

static void emitClassVersion(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "#define %s_MajorVersion %ld\n", pif->chrName, pif->ulMajor);
  fprintf(fh, "#define %s_MinorVersion %ld\n\n", pif->chrName, pif->ulMinor);
}

static void emitClassDataStructs(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  int a;
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "/* Class data structure */\n");
  fprintf(fh, "NOMEXTERN struct %sClassDataStructure {\n", pif->chrName);

  fprintf(fh, "    NOMClass *classObject;\n");
  /* Do introduced methods */
  for(a=0;a<pif->pMethodArray->len;a++)
    {
      PMETHOD pm=(PMETHOD)g_ptr_array_index(pif->pMethodArray, a);
      fprintf(fh, "    nomMToken %s;\n", pm->chrName);
    }
  fprintf(fh, "}%sClassData\n\n", pif->chrName);

  fprintf(fh, "NOMEXTERN struct %sCClassDataStructure {\n", pif->chrName);
  fprintf(fh, "   nomMethodTabs parentMtab;\n");
  fprintf(fh, "   nomDToken instanceDataToken;\n");
  fprintf(fh, "} NOMDLINK %sCClassData;\n\n", pif->chrName);

}


static void emitObjectCheckFunction(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "/* This function is used to check if a given object is valid and the\n");
  fprintf(fh, "   object supports the method */\n");
  fprintf(fh, "NOMEXTERN gboolean NOMLINK nomCheckObjectPtr(NOMObject *nomSelf, NOMClass* nomClass, gchar* chrMethodName, CORBA_Environment *ev);\n\n");
}

/*
  \param pArray Pointer to the list of parameters.
 */
static void emitMethodParams(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray)
{
  FILE* fh=pLocalPI->outFile;
  int a;

  for(a=0;a<pArray->len;a++)
    {
      int b;
      PMETHODPARAM pm=(PMETHODPARAM)g_ptr_array_index(pArray, a);

      switch(pm->uiDirection)
        {
        case PARM_DIRECTION_IN:
          fprintf(fh, "    const %s", pm->chrType);
          break;
        case PARM_DIRECTION_OUT:
          fprintf(fh, "    %s*", pm->chrType);
          break;
        case PARM_DIRECTION_INOUT:

          break;
        default:
          fprintf(fh, "    %s*", pm->chrType);
          break;
        }
      for(b=0;b<pm->uiStar;b++)
        fprintf(fh, "*");
      fprintf(fh, " %s,\n", pm->chrName);      
    }
}

/*
  \param pArray Pointer to the list of parameters.
 */
static void emitMethodParamsNoTypes(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray)
{
  FILE* fh=pLocalPI->outFile;
  int a;

  for(a=0;a<pArray->len;a++)
    {
      PMETHODPARAM pm=(PMETHODPARAM)g_ptr_array_index(pArray, a);
      fprintf(fh, " %s,", pm->chrName);      
    }
}

static void emitNewMethods(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  int a;
  GPtrArray *pArray;
  FILE* fh=pLocalPI->outFile;

  pArray=pif->pMethodArray;

  for(a=0;a<pArray->len;a++)
    {
      int b;
      PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);

      fprintf(fh, "/*\n * New method: %s \n */\n",  pm->chrName);
      fprintf(fh, "#ifndef _decl_%s_%s_\n", pif->chrName, pm->chrName);
      fprintf(fh, "#define _decl_%s_%s_\n\n", pif->chrName,  pm->chrName);

      /* Do return type */
      fprintf(fh, "typedef %s", pm->mpReturn.chrType);
      for(b=0;b<pm->mpReturn.uiStar;b++)
        fprintf(fh, "*");

      fprintf(fh, " NOMLINK nomTP_%s_%s(%s* nomSelf,\n", pif->chrName,  pm->chrName, pif->chrName);

      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);

      fprintf(fh, "    Corba_Environment *ev);\n");
      fprintf(fh, "typedef nomTP_%s_%s *nomTD_%s_%s\n", pif->chrName,  pm->chrName,
              pif->chrName,  pm->chrName);
      fprintf(fh, "/* define the name for this method */\n");
      fprintf(fh, "#define nomMNDef_%s_%s \"%s\"\n", pif->chrName, pm->chrName,  pm->chrName);
      fprintf(fh, "#define nomMNFullDef_%s_%s \"%s:%s\"\n",
              pif->chrName, pm->chrName, pif->chrName, pm->chrName);

      fprintf(fh, "/* define method call as a macro */\n");
      fprintf(fh, "#ifndef NOM_NO_PARAM_CHECK /* Parameter check at all? */\n");
      fprintf(fh, "#ifdef %s_%s_ParmCheck_h /* Extended parameter check enabled */\n",
              pif->chrName, pm->chrName);
      fprintf(fh, "NOMEXTERN gboolean NOMLINK parmCheckFunc_%s_%s(%s *nomSelf,\n",
              pif->chrName,  pm->chrName, pif->chrName);
      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, "    Corba_Environment *ev);\n");

      fprintf(fh, "#define %s_%s(nomSelf,", pif->chrName, pm->chrName);
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev) \\\n");
      fprintf(fh, "        (parmCheckFunc_%s_%s(nomSelf,", pif->chrName, pm->chrName);
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev) ? \\\n");
      fprintf(fh, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", pif->chrName, pm->chrName);
      fprintf(fh, "        (nomSelf,");
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev)) : %s_%s_retval)\n", pif->chrName, pm->chrName);
      fprintf(fh, "#else /* Extended parameter check */\n");
      fprintf(fh, "#define %s_%s(nomSelf,", pif->chrName, pm->chrName);
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev) \\\n");
      fprintf(fh, "        (nomCheckObjectPtr((NOMObject*)nomSelf, %sClassData.classObject,",pif->chrName);
      fprintf(fh, "\"%s_%s\", ev) ? \\\n", pif->chrName, pm->chrName);

      fprintf(fh, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", pif->chrName, pm->chrName);
      fprintf(fh, "        (nomSelf,");
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev)) : (gpointer) NULL)\n");
      fprintf(fh, "#endif\n");
      fprintf(fh, "#else /* NOM_NO_PARAM_CHECK */\n");
      fprintf(fh, "#define %s_%s(nomSelf,", pif->chrName, pm->chrName);
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev) \\\n");
      fprintf(fh, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", pif->chrName, pm->chrName);
      fprintf(fh, "        (nomSelf,");
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev))\n");
      fprintf(fh, "#endif\n");
      fprintf(fh, "#define _%s %s_%s\n", pm->chrName, pif->chrName, pm->chrName);
      fprintf(fh, "#endif /* _decl_%s_%s_ */ \n\n", pif->chrName,  pm->chrName);
    }
};

static void emitNewMacro(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "/*\n * Class creation function\n */\n");
  fprintf(fh, "NOMEXTERN NOMClass * NOMLINK %sNewClass(gulong clsMajorVersion, gulong clsMinorVersion);\n\n",
          pif->chrName);
  
  fprintf(fh, "#define _%s (%s*)%sClassData.classObject\n\n",
          pif->chrName, pif->chrMetaClass, pif->chrName);
  
  fprintf(fh, "/*\n * New macro for WPObject\n */\n\n");
  fprintf(fh, "#define %sNew() \\\n", pif->chrName);
  fprintf(fh, "        ((%s*)_nomNew((_%s ? _%s : ", pif->chrName, pif->chrName, pif->chrName);
  fprintf(fh, "(%s*)%sNewClass(%s_MajorVersion, %s_MinorVersion)), (void*) 0))\n\n",
          pif->chrName, pif->chrName, pif->chrName, pif->chrName);

}

static void emitParentClassMethods(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  PINTERFACE pifParent=pif;

  while((pifParent=getParentInterface(pifParent))!=NULLHANDLE)
    {
      GPtrArray *pArray; 
      int a;
      /* Do this parents methods */
      pArray=pifParent->pMethodArray;
      for(a=0;a<pArray->len;a++)
        {
          PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);

          fprintf(fh, "#define %s_%s \\\n", pif->chrName, pm->chrName);
          fprintf(fh, "        %s_%s \n", pifParent->chrName, pm->chrName);
        }
    }
  fprintf(fh, "\n");
}

static void emitHFileFooter(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "#ifdef __cplusplus\n");
  fprintf(fh, "}\n");
  fprintf(fh, "#endif /* __cplusplus */\n\n");

  fprintf(fh, "\n#endif /* %s_H */\n", pif->chrName);
}

void emitHFile(GPtrArray* pInterfaceArray)
{
  int a;
  PPARSEINFO pLocalPI=(PPARSEINFO)gScanner->user_data;

  for(a=0;a<pInterfaceArray->len;a++)
    {
      PINTERFACE pif=g_ptr_array_index(pLocalPI->pInterfaceArray, a); 
      /* Only interfaces from the file given on the command line */
      if(!strcmp(pif->chrSourceFileName, pLocalPI->chrRootSourceFile))
        {
          gchar*  chrTemp;
          
          chrTemp=g_strconcat(pif->chrFileStem, ".h", NULL);

          printInterface(pif);
          if((pLocalPI->outFile=openOutfile(gScanner, chrTemp))!=NULLHANDLE)
            {
              emitHFileHeader(pLocalPI, pif);
              emitParentHeader(pLocalPI, pif);
              emitClassVersion(pLocalPI, pif);
              emitClassDataStructs(pLocalPI, pif);
              emitNewMacro(pLocalPI, pif);
              emitObjectCheckFunction(pLocalPI, pif);
              emitNewMethods(pLocalPI, pif);
              emitParentClassMethods(pLocalPI, pif);
              emitHFileFooter(pLocalPI, pif);
              closeOutfile(pLocalPI->outFile);
            }
          g_free(chrTemp);
        }
    }
}


