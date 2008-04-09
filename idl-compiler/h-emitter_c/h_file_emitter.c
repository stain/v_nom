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
#ifdef __OS2__
# include <os2.h>
#endif  /* __OS2__ */

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
   This function writes an #include statement to the header for each found
   interface. Thus all the interface information is known to the sourcefile
   using this header.
 */
static void emitInterfaceIncludes(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  int a;

  for(a=0;a<pLocalPI->pInterfaceArray->len;a++)
    {
      PINTERFACE pifAll=g_ptr_array_index(pLocalPI->pInterfaceArray, a);

      fprintf(fh, "/* Include for class %s */\n", pifAll->chrName);
      if(pifAll->chrFileStem)
        fprintf(fh, "#include \"%s.h\"\n", pifAll->chrFileStem);
    }
  fprintf(fh, "\n");
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
  fprintf(fh, "#ifdef NOM_%s_IMPLEMENTATION_FILE\n", pif->chrName);
  fprintf(fh, "NOMDLLEXPORT\n");
  fprintf(fh, "#else\n");
  fprintf(fh, "NOMDLLIMPORT\n");
  fprintf(fh, "#endif\n");
  fprintf(fh, "NOMEXTERN struct %sClassDataStructure {\n", pif->chrName);

  fprintf(fh, "    NOMClass *classObject;\n");
  /* Do introduced methods */
  for(a=0;a<pif->pMethodArray->len;a++)
    {
      PMETHOD pm=(PMETHOD)g_ptr_array_index(pif->pMethodArray, a);
      fprintf(fh, "    nomMToken %s;\n", pm->chrName);
    }
  fprintf(fh, "} NOMDLINK %sClassData;\n\n", pif->chrName);

  fprintf(fh, "#ifdef NOM_%s_IMPLEMENTATION_FILE\n", pif->chrName);
  fprintf(fh, "NOMDLLEXPORT\n");
  fprintf(fh, "#else\n");
  fprintf(fh, "NOMDLLIMPORT\n");
  fprintf(fh, "#endif\n");
  fprintf(fh, "NOMEXTERN struct %sCClassDataStructure {\n", pif->chrName);
  fprintf(fh, "   nomMethodTabs parentMtab;\n");
  fprintf(fh, "   nomDToken instanceDataToken;\n");
  fprintf(fh, "} NOMDLINK %sCClassData;\n\n", pif->chrName);

}


static void emitObjectCheckFunction(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  if(strcmp(pif->chrName , "NOMObject"))
    {
/* FIXME: why is this here too? it's already in nomtk.h... */
      fprintf(fh, "/* This function is used to check if a given object is valid and the\n");
      fprintf(fh, "   object supports the method */\n");
      fprintf(fh, "NOMEXTERN gboolean NOMLINK nomCheckObjectPtr(NOMObject *nomSelf, NOMClass* nomClass, gchar* chrMethodName, CORBA_Environment *ev);\n\n");
    }
  else
    {
      fprintf(fh, "/* This function is used to check if the given object is valid and a NOMObject */\n");
      fprintf(fh, "NOMEXTERN gboolean NOMLINK nomCheckNOMObjectPtr(NOMObject *nomSelf, NOMClass* nomClass, gchar* chrMethodName, CORBA_Environment *ev);\n\n");
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
      fprintf(fh, "typedef ");
      emitReturnType(pLocalPI, pif, pm);

      fprintf(fh, " NOMLINK nomTP_%s_%s(%s* nomSelf,\n", pif->chrName,  pm->chrName, pif->chrName);
      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, "    CORBA_Environment *ev);\n");
      fprintf(fh, "typedef nomTP_%s_%s *nomTD_%s_%s;\n", pif->chrName,  pm->chrName,
              pif->chrName,  pm->chrName);
      fprintf(fh, "/* define the name for this method */\n");
      fprintf(fh, "#define nomMNDef_%s_%s \"%s\"\n", pif->chrName, pm->chrName,  pm->chrName);
      fprintf(fh, "#define nomMNFullDef_%s_%s \"%s:%s\"\n",
              pif->chrName, pm->chrName, pif->chrName, pm->chrName);

      fprintf(fh, "/* define method call as a macro */\n");
      fprintf(fh, "#ifndef NOM_NO_PARAM_CHECK /* Parameter check at all? */\n");
      fprintf(fh, "#ifdef %s_%s_ParmCheck_h /* Extended parameter check enabled */\n",
              pif->chrName, pm->chrName);
      /* Forward declaration of parameter test function */
      fprintf(fh, "#ifdef NOM_%s_IMPLEMENTATION_FILE\n", pif->chrName);
      fprintf(fh, "NOMDLLEXPORT\n");
      fprintf(fh, "#else\n");
      fprintf(fh, "NOMDLLIMPORT\n");
      fprintf(fh, "#endif\n");
      fprintf(fh, "NOMEXTERN gboolean NOMLINK parmCheckFunc_%s_%s(%s *nomSelf,\n",
              pif->chrName,  pm->chrName, pif->chrName);
      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, "    CORBA_Environment *ev);\n");
      /* Macro to be used when several parameters are checked */
      fprintf(fh, "#define %s_%s(nomSelf,", pif->chrName, pm->chrName);
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev) \\\n");

      if(strcmp(pm->chrName, "nomIsObject"))
        {
          fprintf(fh, "        (parmCheckFunc_%s_%s(nomSelf,", pif->chrName, pm->chrName);
          /* Do parameters */
          emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
          fprintf(fh, " ev) ? \\\n");
          fprintf(fh, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", pif->chrName, pm->chrName);
          fprintf(fh, "        (nomSelf,");
          /* Do parameters */
          emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
          fprintf(fh, " ev)) : %s_%s_retval)\n", pif->chrName, pm->chrName);
        }
      else
        {
          /* No check for nomIsObject or otherwise we have a recursion */
          fprintf(fh, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", pif->chrName, pm->chrName);
          fprintf(fh, "        (nomSelf,");
          /* Do parameters */
          emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
          fprintf(fh, " ev))\n");
        }
      fprintf(fh, "#else /* Extended parameter check */\n"); /* else NOM_NO_PARAM_CHECK */
      /* Check object only  */
      fprintf(fh, "#define %s_%s(nomSelf,", pif->chrName, pm->chrName);
      /* Do parameters */
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev) \\\n");
      if(strcmp(pm->chrName, "nomIsObject"))
        {
          if(strcmp(pif->chrName , "NOMObject"))
            fprintf(fh, "        (nomCheckObjectPtr((NOMObject*)nomSelf, %sClassData.classObject,",pif->chrName);
          else
            fprintf(fh, "        (nomCheckNOMObjectPtr(nomSelf, %sClassData.classObject,",pif->chrName);
          fprintf(fh, "\"%s_%s\", ev) ? \\\n", pif->chrName, pm->chrName);
          fprintf(fh, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", pif->chrName, pm->chrName);
          fprintf(fh, "        (nomSelf,");
          /* Do parameters */
          emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
          fprintf(fh, " ev)) : (%s", pm->mpReturn.chrType);
          for(b=0;b<pm->mpReturn.uiStar;b++)
            fprintf(fh, "*");
          fprintf(fh, ") NULL)\n");
        }
      else
        {
          /* No check for nomIsObject or otherwise we have a recursion */
          fprintf(fh, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", pif->chrName, pm->chrName);
          fprintf(fh, "        (nomSelf,");
          /* Do parameters */
          emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
          fprintf(fh, " ev))\n");
        }
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
  fprintf(fh, "#ifdef NOM_%s_IMPLEMENTATION_FILE\n", pif->chrName);
  fprintf(fh, "NOMDLLEXPORT\n");
  fprintf(fh, "#else\n");
  fprintf(fh, "NOMDLLIMPORT\n");
  fprintf(fh, "#endif\n");
  fprintf(fh, "NOMEXTERN NOMClass * NOMLINK %sNewClass(gulong clsMajorVersion, gulong clsMinorVersion);\n\n",
          pif->chrName);
  
#if 0
  if(pif->chrMetaClass)
    fprintf(fh, "#define _%s (%s*)%sClassData.classObject\n\n",
            pif->chrName, pif->chrMetaClass, pif->chrName);
  else
#endif
    fprintf(fh, "#define _%s %sClassData.classObject\n\n",
            pif->chrName,  pif->chrName);

  fprintf(fh, "/*\n * New macro for WPObject\n */\n\n");
  fprintf(fh, "#define %sNew() \\\n", pif->chrName);
  fprintf(fh, "        ((%s*)_nomNew((_%s ? _%s : ", pif->chrName, pif->chrName, pif->chrName);
  fprintf(fh, "%sNewClass(%s_MajorVersion, %s_MinorVersion)), (void*) 0))\n\n",
          pif->chrName, pif->chrName, pif->chrName);

}

static void emitParentClassMethods(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  PINTERFACE pifParent=pif;

  while((pifParent=getParentInterface(pifParent))!=NULL)
    {
      GPtrArray *pArray; 
      int a;
      /* Do this parents methods */
      pArray=pifParent->pMethodArray;
      for(a=0;a<pArray->len;a++)
        {
          PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);

          fprintf(fh, "#define %s_%s(nomSelf, ", pif->chrName, pm->chrName);
          /* Do parameters */
          emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
          fprintf(fh, " ev) \\\n");
          fprintf(fh, "        %s_%s((%s*) nomSelf, ", pifParent->chrName, pm->chrName, pifParent->chrName);
          /* Do parameters */
          emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
          fprintf(fh, " ev)\n");
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
          /* Only interfaces which are fully defined. No forwarder */
          if(!pif->fIsForwardDeclaration)
            {
              gchar*  chrTemp;

              chrTemp=g_strconcat(pif->chrFileStem, ".h", NULL);
              
              //printInterface(pif);              
              if((pLocalPI->outFile=openOutfile(gScanner, chrTemp))!=NULL)
                {
                  emitHFileHeader(pLocalPI, pif);
                  emitInterfaceIncludes(pLocalPI, pif);
                  emitClassVersion(pLocalPI, pif);
                  emitClassDataStructs(pLocalPI, pif);
                  emitNewMacro(pLocalPI, pif);
                  emitObjectCheckFunction(pLocalPI, pif);
                  emitNewMethods(pLocalPI, pif);
                  emitParentClassMethods(pLocalPI, pif);
                  emitHFileFooter(pLocalPI, pif);
                  closeOutfile(pLocalPI->outFile);
                  pLocalPI->outFile = NULL;
                }
              g_free(chrTemp);
            }/* fIsForwardDeclaration */
        }
    }
}


