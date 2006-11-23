#include "config.h"
#include "orbit-idl-c-backend.h"
#include <string.h>

typedef struct {
  FILE *of;
  IDL_tree realif;
  char* chrOverridenMethodName;
} InheritedOutputInfo;

static void
VoyagerOutputOverridenMethodTemplate(IDL_tree curif, InheritedOutputInfo *ioi)
{
  char *id, *realid;
  IDL_tree curitem;
  char* overridenMethodName;

  if(curif == ioi->realif)
    return;

  overridenMethodName=ioi->chrOverridenMethodName;


  realid = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(ioi->realif).ident), "_", 0);

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(curif).ident), "_", 0);


  for(curitem = IDL_INTERFACE(curif).body; curitem; curitem = IDL_LIST(curitem).next) {
    IDL_tree curop = IDL_LIST(curitem).data;

    switch(IDL_NODE_TYPE(curop)) {
    case IDLN_OP_DCL:
      {
        /* Check if the current method (introduced by some parent) is the one to be
           overriden. */
        if(!strcmp(overridenMethodName, IDL_IDENT(IDL_OP_DCL(curop).ident).str)){
          fprintf(ioi->of, "/* Call parent */\n");

          fprintf(ioi->of, "/*         %s_%s_parent_resolved()*/\n",
                  realid, IDL_IDENT(IDL_OP_DCL(curop).ident).str);

#if 0
          fprintf(ioi->of, "#define %s_%s() \\\n        %s_%s()\n",
                  realid, IDL_IDENT(IDL_OP_DCL(curop).ident).str,
                  id, IDL_IDENT(IDL_OP_DCL(curop).ident).str);
#endif
        }
        break;
      }
	default:
	  break;
    }
  }

  g_free(id);
  g_free(realid);

}


void
VoyagerWriteProtoForParentCall (FILE       *of,
                                IDL_tree    op,
                                const char *nom_prefix,
                                gboolean    for_epv)
{
  IDL_tree  sub;
  char     *id;
  char * id2;
  char * ptr;
  
  g_assert (IDL_NODE_TYPE(op) == IDLN_OP_DCL);
  
  //  orbit_cbe_write_param_typespec (of, op);
  
  id = IDL_ns_ident_to_qstring (
          IDL_IDENT_TO_NS (IDL_INTERFACE (
          IDL_get_parent_node (op, IDLN_INTERFACE, NULL)).ident), "_", 0);

    id2=g_strdup(IDL_IDENT (IDL_OP_DCL (op).ident).str);
    if((ptr=strstr(id2, NOM_OVERRIDE_STRING))!=NULL)
       *ptr='\0';
    fprintf (of, "  /* %s, %s line %d */\n", __FILE__, __FUNCTION__, __LINE__); 
    fprintf (of, "  %s%s_%s_parent", nom_prefix ? nom_prefix : "",
			 id, id2);

    g_free(id2);


	fprintf (of, "(");

    fprintf (of, "nomSelf, ", id);

	g_free (id);

	for (sub = IDL_OP_DCL (op).parameter_dcls; sub; sub = IDL_LIST (sub).next) {
		IDL_tree parm = IDL_LIST (sub).data;

		//orbit_cbe_write_param_typespec (of, parm);
		fprintf (of, " %s, ", IDL_IDENT (IDL_PARAM_DCL (parm).simple_declarator).str);
	}

	fprintf (of, " ev);\n");
}

static void
cs_output_stub (IDL_tree     tree,
		OIDL_C_Info *ci,
		int         *idx)
{
	FILE     *of = ci->fh;
	char     *iface_id;
	char     *opname;
	gboolean  has_retval, has_args;
	g_return_if_fail (idx != NULL);
	iface_id = IDL_ns_ident_to_qstring (
			IDL_IDENT_TO_NS (IDL_INTERFACE (
				IDL_get_parent_node (tree, IDLN_INTERFACE, NULL)
					).ident), "_", 0);
	opname = IDL_ns_ident_to_qstring (IDL_IDENT_TO_NS (IDL_OP_DCL (tree).ident), "_", 0);
	has_retval = IDL_OP_DCL (tree).op_type_spec != NULL;
	has_args   = IDL_OP_DCL (tree).parameter_dcls != NULL;
#ifdef USE_LIBIDL_CODE
	orbit_cbe_op_write_proto (of, tree, "", FALSE);
	fprintf (of, "{\n");

	if (has_retval) {
		orbit_cbe_write_param_typespec (of, tree);
		fprintf (of, " " ORBIT_RETVAL_VAR_NAME ";\n");
	}
	fprintf (ci->fh, "POA_%s__epv *%s;\n", iface_id, ORBIT_EPV_VAR_NAME);
	fprintf (ci->fh, "gpointer _ORBIT_servant;\n");
	/* in-proc part */
	fprintf (ci->fh, "if ((%s = ORBit_c_stub_invoke\n", ORBIT_EPV_VAR_NAME);
	fprintf (ci->fh, "		(_obj, %s__classid, &_ORBIT_servant,\n", iface_id);
	fprintf (ci->fh, "               G_STRUCT_OFFSET (POA_%s__epv, %s)))) {\n",
		 iface_id, IDL_IDENT (IDL_OP_DCL (tree).ident).str);
	fprintf (ci->fh, "if (ORBit_small_flags & ORBIT_SMALL_FAST_LOCALS && \n");
	fprintf (ci->fh, "    ORBIT_STUB_IsBypass (_obj, %s__classid) && \n", iface_id);
	fprintf (ci->fh, "    (%s = (POA_%s__epv*) ORBIT_STUB_GetEpv (_obj, %s__classid))->%s) {\n",
		 ORBIT_EPV_VAR_NAME, iface_id, iface_id, IDL_IDENT (IDL_OP_DCL (tree).ident).str);
	fprintf (ci->fh, "ORBIT_STUB_PreCall (_obj);\n");
	fprintf (ci->fh, "%s%s->%s (_ORBIT_servant, ",
		 IDL_OP_DCL (tree).op_type_spec? ORBIT_RETVAL_VAR_NAME " = ":"",
		 ORBIT_EPV_VAR_NAME,
		 IDL_IDENT (IDL_OP_DCL (tree).ident).str);
	for (node = IDL_OP_DCL (tree).parameter_dcls; node; node = IDL_LIST (node).next)
		fprintf (ci->fh, "%s, ",
			 IDL_IDENT (IDL_PARAM_DCL (IDL_LIST (node).data).simple_declarator).str);
	if (IDL_OP_DCL (tree).context_expr)
		fprintf (ci->fh, "_ctx, ");
	fprintf (ci->fh, "ev);\n");
	fprintf (ci->fh, "ORBit_stub_post_invoke (_obj, %s);\n", ORBIT_EPV_VAR_NAME);
	fprintf (of, " } else { /* remote marshal */\n");

	/* remote invocation part */
	if (has_args)
		orbit_cbe_flatten_args (tree, of, "_args");
	fprintf (of, "ORBit_c_stub_invoke (_obj, "
		 "&%s__iinterface.methods, %d, ", iface_id, *idx);
	if (has_retval)
		fprintf (of, "&_ORBIT_retval, ");
	else
		fprintf (of, "NULL, ");
	if (has_args)
		fprintf (of, "_args, ");
	else
		fprintf (of, "NULL, ");
	if (IDL_OP_DCL (tree).context_expr)
		fprintf (ci->fh, "_ctx, ");
	else
		fprintf (ci->fh, "NULL, ");
		
	fprintf (of, "ev, ");
	fprintf (of, "%s__classid, G_STRUCT_OFFSET (POA_%s__epv, %s),\n",
		 iface_id, iface_id, IDL_IDENT (IDL_OP_DCL (tree).ident).str);
	fprintf (of, "(ORBitSmallSkeleton) _ORBIT_skel_small_%s);\n\n", opname);
	if (has_retval)
		fprintf (of, "return " ORBIT_RETVAL_VAR_NAME ";\n");
	fprintf (of, "}\n");
#else
    if(strstr(opname, NOM_INSTANCEVAR_STRING)==NULL &&
       strstr(opname, NOM_OVERRIDE_STRING)==NULL)
      {
        /* Skip specially marked methods containing "__INSTANCEVAR__  */
        fprintf (of, "NOM_Scope ");
        orbit_cbe_op_write_proto (of, tree, "NOMLINK impl_", FALSE);
        fprintf (of, "\n{\n");
        fprintf (of, "/* %sData* nomThis=%sGetData(nomSelf); */\n",
                 iface_id, iface_id);
        if (has_retval) {
          fprintf (of, "  ");
          orbit_cbe_write_param_typespec (of, tree);
          fprintf (of, " " ORBIT_RETVAL_VAR_NAME ";\n\n");
          
          fprintf (of, "  return " ORBIT_RETVAL_VAR_NAME ";\n");
        }
        else
          fprintf (of, "\n"); /* Beautyfication... */
        
        fprintf (of, "}\n\n");
      }
    if(strstr(opname, NOM_OVERRIDE_STRING)!=NULL)
      {
        IDL_tree tmptree;

        /* There's an overriden method here */
        fprintf (of, "NOM_Scope ");

        orbit_cbe_op_write_proto (of, tree, "NOMLINK impl_", FALSE);

        fprintf (of, "\n{\n");
        fprintf (of, "/* %sData* nomThis=%sGetData(nomSelf); */\n",
                 iface_id, iface_id);
        if (has_retval) {
          fprintf (of, "  ");
          orbit_cbe_write_param_typespec (of, tree);
          fprintf (of, " " ORBIT_RETVAL_VAR_NAME ";\n\n");
          
          fprintf (of, "  return " ORBIT_RETVAL_VAR_NAME ";\n");
        }
        else
          fprintf (of, "\n"); /* Beautyfication... */
        
        fprintf (of, "#if 0\n");
        if (has_retval) 
          fprintf (of, "  return ");
        VoyagerWriteProtoForParentCall (of, tree, "", FALSE);

        fprintf (of, "#endif\n");
        //        fprintf(of, "/*  %s_%s_parent_resolved()*/\n",
        //      realid, IDL_IDENT(IDL_OP_DCL(curop).ident).str);

#if 0
        /* Inherited */
        tmptree = IDL_get_parent_node(tree, IDLN_INTERFACE, NULL);; 
        if(IDL_INTERFACE(tmptree).inheritance_spec) {
          char * ptr=opname;
          char * str;
          
          printf("!!!! %s\n", opname);
          InheritedOutputInfo ioi;
          ioi.of = ci->fh;
          ioi.realif = tmptree;
          
          //        str= IDL_ns_ident_to_qstring( IDL_IDENT_TO_NS (IDL_INTERFACE(tree).ident), "_", 0);
          
          if((ptr=strchr(opname, '_'))!=NULL)
            ptr++;
          str=g_strdup(ptr);
          if((ptr=strchr(str, '_'))!=NULL)
            *ptr='\0';
          
          ioi.chrOverridenMethodName=str;
          IDL_tree_traverse_parents(IDL_INTERFACE(tmptree).inheritance_spec, (GFunc)VoyagerOutputOverridenMethodTemplate, &ioi);
          if(NULL!=ptr)
            *ptr='_';
        }
#endif
        
        fprintf (of, "}\n\n");
      }
#endif

	g_free (iface_id);
	(*idx)++;
}


static void
cs_output_stubs (IDL_tree     tree,
		 OIDL_C_Info *ci,
		 int         *idx)
{
	if (!tree)
		return;
	switch (IDL_NODE_TYPE (tree)) {
	case IDLN_MODULE:
		cs_output_stubs (IDL_MODULE (tree).definition_list, ci, idx);
		break;
	case IDLN_LIST: {
		IDL_tree sub;
		for (sub = tree; sub; sub = IDL_LIST (sub).next)
			cs_output_stubs (IDL_LIST (sub).data, ci, idx);
		break;
		}
	case IDLN_ATTR_DCL: {
		IDL_tree node;
      
		for (node = IDL_ATTR_DCL (tree).simple_declarations; node; node = IDL_LIST (node).next) {
			OIDL_Attr_Info *ai;
			ai = IDL_LIST (node).data->data;
	
			cs_output_stubs (ai->op1, ci, idx);
			if (ai->op2)
				cs_output_stubs (ai->op2, ci, idx);
		}
		break;
		}
	case IDLN_INTERFACE: {
		int real_idx = 0;
		cs_output_stubs (IDL_INTERFACE (tree).body, ci, &real_idx);
		break;
		}
	case IDLN_OP_DCL:
		cs_output_stub (tree, ci, idx);
		break;
	default:
		break;
	}
}

void
orbit_idl_output_c_stubs (IDL_tree       tree,
			  OIDL_Run_Info *rinfo,
			  OIDL_C_Info   *ci)
{
	fprintf (ci->fh, OIDL_C_WARNING);
	fprintf (ci->fh, "#include <string.h>\n");
#ifdef USE_LIBIDL_CODE
    fprintf (ci->fh, "#define ORBIT2_STUBS_API\n");
#endif
	fprintf (ci->fh, "#include \"%s.ih\"\n\n", ci->base_name);
	cs_output_stubs (tree, ci, NULL);
}
