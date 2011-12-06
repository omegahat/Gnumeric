#include "config.h"
#include <gal/util/e-xml-utils.h>
#include <gnome.h>
#include <locale.h>
#include <math.h>
#include <limits.h>
#include "gnumeric.h"
#include "gnome-xml/parser.h"
#include "gnome-xml/parserInternals.h"
#include "gnome-xml/xmlmemory.h"
#include "xml-io.h"
#include "io-context.h"
#include "workbook-view.h"
#include "workbook.h"
#include "sheet.h"

#include "RGnumeric.h"
#include "R_ext/Boolean.h"


/**
  A structure used for recording an unique list of 
  environments of R functions in a Gnumeric session
  for the purposes of writing out the environments in 
  a worksheet's functions.
 */
typedef struct _R_EnvironmentFunctions R_EnvironmentFunctions;

struct _R_EnvironmentFunctions {
    SEXP env;                     /* The internal environment pointer */
    SEXP functionEnv;             /* The S-level result of environment(functionObject) */
    char *name;                   /* Symbolic name for the environment used in the XML file. */
    R_EnvironmentFunctions *next; /* Next element in the linked list. */
};

static int R_readFunctions(xmlNodePtr node, R_EnvironmentFunctions *envs);
static Rboolean R_addFunction(xmlNodePtr node, R_EnvironmentFunctions *envs);

USER_OBJECT_ RGnumeric_getFunctionObject(const char *def);

USER_OBJECT_ RGnumeric_getFunctionEnv(USER_OBJECT_ body);

static R_EnvironmentFunctions* RGnumeric_writeFunctions(xmlNodePtr, xmlDocPtr, xmlNsPtr);
static int RGnumeric_writeFunction(xmlNodePtr funcsNode, RGnumeric_FuncData *el, xmlNsPtr nameSpace, const char *envName);

static xmlNodePtr RGnumeric_writeRState(xmlNodePtr top, xmlDocPtr xml, xmlNsPtr nameSpace);

xmlNodePtr RGnumeric_writeEnvironments(R_EnvironmentFunctions *environments, xmlNodePtr node, xmlDocPtr xml, xmlNsPtr nameSpace);
xmlNodePtr RGnumeric_writeEnvironment(R_EnvironmentFunctions *env, xmlNodePtr node, xmlDocPtr xml, xmlNsPtr nameSpace);

static int R_processRSearchPath(xmlNodePtr node);
Rboolean RGnumeric_loadPackage(xmlChar *pkgName);

R_EnvironmentFunctions *RGnumeric_readEnvironments(xmlNodePtr node);
R_EnvironmentFunctions *RGnumeric_readEnvironment(xmlNodePtr node);

USER_OBJECT_ RGnumeric_createEnvironment();
void RGnumeric_assignEnvironmentValue(xmlChar *name, USER_OBJECT_ val, USER_OBJECT_ env);

xmlNodePtr RGnumeric_getToplevelNode(xmlNodePtr node, const char *tagName);

USER_OBJECT_ RGnumeric_readEnvironmentElement(xmlNodePtr node, R_EnvironmentFunctions *env);

static void RGnumeric_evalSheetInitCode(xmlNodePtr root);

void R_PreserveObject(SEXP);

/**
  The idea here is to hijack/extend the default Gnumeric
  save mechanism by using its facilities for creating an XML
  tree and then for us to modify that tree by adding the R
  functions in the sheet.
 */

#include "file.h"
#ifdef NEED_GNUMERIC_IO_DEFS
typedef FileSaver  GnumFileSaver;
typedef FileOpener GnumFileOpener;
#endif

void
R_file_save (GnumFileSaver const *fs, IOContext *io_context,
                    WorkbookView *wb_view, const gchar *filename)
{
	xmlDocPtr xml;
	XmlParseContext *ctxt;
        xmlNsPtr nameSpace;
	int ret;
	R_EnvironmentFunctions *environments;

	g_return_if_fail (wb_view != NULL);
	g_return_if_fail (filename != NULL);


    	    /* Taken from gnumeric_xml_write_workbook() in xml-io.c */
	xml = xmlNewDoc ("1.0");
	if (xml == NULL) {
		gnumeric_io_error_save (io_context, "");
		return;
	}
	ctxt = xml_parse_ctx_new (xml, NULL);
	xml->xmlRootNode = xml_workbook_write (ctxt, wb_view);

        /* Now add the R functions. */
        nameSpace = xmlNewNs(xml->xmlRootNode, "http://www.r-project.org", "R");

        environments = RGnumeric_writeFunctions(xml->xmlRootNode, xml, nameSpace);
        RGnumeric_writeEnvironments(environments, xml->xmlRootNode, xml, nameSpace);
        RGnumeric_writeRState(xml->xmlRootNode, xml, nameSpace);


	xml_parse_ctx_destroy (ctxt);

   	   /* Write the file. */
	xmlSetDocCompressMode (xml, 9);
	ret = xmlSaveFile (filename, xml);

	xmlFreeDoc (xml);
	if (ret < 0) {
		gnumeric_io_error_save (io_context, "");
		return;
	}
#if 0
       xmlFreeNs(nameSpace);
#endif
}

/**
  Retrieve the search() path for the R session.
 */
static USER_OBJECT_
RGnumeric_getSearchPath()
{
    USER_OBJECT_ e, val;
    int ok;
    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, Rf_install("search"));
    val = R_tryEval(e, R_GlobalEnv, &ok);
    UNPROTECT(1);
    return(val);
}

/**
   Write the state of the R session into the XML DOM model
   as 
     <R:searchPath>
      <R:element name="pkg-name"/>
        ...
     </R:searchPath>    
 */
static xmlNodePtr
RGnumeric_writeRState(xmlNodePtr top, xmlDocPtr xml, xmlNsPtr nameSpace)
{
   xmlNodePtr node, tmp;
   int i, n;
   USER_OBJECT_ searchPath;
   node = xmlNewChild(top, nameSpace, "searchPath", NULL);

   PROTECT(searchPath = RGnumeric_getSearchPath());
   n = GET_LENGTH(searchPath);
   for(i = 0; i < n ; i++) {  
       tmp = xmlNewChild(node, nameSpace, "element", NULL);
       xmlSetProp(tmp, "name", CHAR_DEREF(STRING_ELT(searchPath, i)));
   }
   UNPROTECT(1);

   return(node);
}

/**
 Loop over all but the last of the R_EnvironmentFunctions objects
 and add its contents to the XML tree.
 This iterates over the elements and calls 
    #RGnumeric_writeEnvironment()
  @param environments  the linked list of environments to be serialized in the XML DOM.
  @param node  the node in te XML DOM under which the new elements should be added
     as children

  @param xml the document object associated with this tree/DOM.
  @param xmlNsPtr the XML namespace for R.
 */
xmlNodePtr
RGnumeric_writeEnvironments(R_EnvironmentFunctions *environments, xmlNodePtr node, xmlDocPtr xml, xmlNsPtr nameSpace)
{
    R_EnvironmentFunctions *tmp;
    xmlNodePtr nnode;
    int n = 0;

    tmp = environments;

    if(tmp == NULL) {
	return(0);
    }

    nnode = xmlNewChild(node, nameSpace, "environments", NULL);        

    while(tmp) {
        if(tmp->env == R_GlobalEnv || tmp->next == NULL)
	    break;
        RGnumeric_writeEnvironment(tmp, nnode, xml, nameSpace);
        tmp = tmp->next;
	n++;
    }

    return(nnode);
}

/**
 Retrieves the names of the variables in an R environment.
 This calls the R function objects().

 @param env  the R environment object.
 */
USER_OBJECT_
getEnvironmentNames(USER_OBJECT_ env)
{
  int errorOccurred;
  USER_OBJECT_ ans, tmp, tmp1, e;

  PROTECT(e = allocVector(LANGSXP, 3));
  SETCAR(e, Rf_install("objects"));

  tmp = CDR(e);
  SETCAR(tmp, env);
  SET_TAG(tmp, Rf_install("envir"));
  tmp = CDR(tmp);
  SETCAR(tmp, tmp1 = NEW_LOGICAL(1));
  LOGICAL_DATA(tmp1)[0] = TRUE;
  SET_TAG(tmp, Rf_install("all.names"));
   
  ans = R_tryEval(e, R_GlobalEnv, &errorOccurred);
  if(errorOccurred) {
      fprintf(stderr, "Error occurred\n");fflush(stderr);
  }
  UNPROTECT(1);

  return(ans);
}

/**
  Get a text version of the variable identified by <code>name</code>
  in the given R environment.
  This  uses <code>dput</code>

  @param name the internal CHARSXP R object giving the name of the 
     object to be dput()
  @param env  the R environment in which the variable is to be found.
 */
USER_OBJECT_
RGnumeric_dputEnvValue(USER_OBJECT_ name, USER_OBJECT_ env)
{
    USER_OBJECT_ e, ans, tmp;
    int errorOccurred;
    PROTECT(e = allocVector(LANGSXP, 3)); 
    SETCAR(e, Rf_install("dputEnvironmentObject"));
    SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
    SET_STRING_ELT(tmp, 0, name);
    SETCAR(CDR(CDR(e)), env);
    ans = R_tryEval(e, R_GlobalEnv, &errorOccurred);
    UNPROTECT(1);
    return(ans);
}

/**
 Serialize the contents of the specified R environment into the XML tree.

 @param env the R environment information
 @param node the node to which the serialized tree should be added as a child.
 @param xml the document associated with the overall DOM/tree.
 @param nameSpace the XML namespace identifying R.
 */
xmlNodePtr 
RGnumeric_writeEnvironment(R_EnvironmentFunctions *env, xmlNodePtr node, xmlDocPtr xml, xmlNsPtr nameSpace)
{
  xmlNodePtr nnode, tmpNode;
  USER_OBJECT_ names, ans;
  int n, i;
  char *defString;

  nnode = xmlNewChild(node, nameSpace, "environment", NULL);    
  xmlSetProp(nnode, "name", env->name);

  names = getEnvironmentNames(env->functionEnv);
  if(names == NULL || names == NULL_USER_OBJECT)
      return(nnode);

  PROTECT(names);
 
  n = GET_LENGTH(names);
  for(i = 0; i < n; i++) {
      tmpNode = xmlNewChild(nnode, nameSpace, "element", NULL);        
      xmlSetProp(tmpNode, "name", CHAR_DEREF(STRING_ELT(names,i)));


      ans = RGnumeric_dputEnvValue(STRING_ELT(names, i), env->functionEnv);      

      defString = CHAR_DEREF(STRING_ELT(ans, 1));
      xmlSetProp(tmpNode, "s-type", defString);

      defString = CHAR_DEREF(STRING_ELT(ans, 0));
      xmlAddChild(tmpNode, xmlNewCDataBlock(node->doc, defString, strlen(defString)));
  }
  UNPROTECT(1);
  return(nnode);
}


/**

 The idea here is iterate over the Gnumeric functions registered
 via R and write out the definitions of these functions, including
 the R function, the Gnumeric argument declaration, etc.
 We also have to handle preserving the shared environments of these functions.
 To do this, we examine each function as we write it out and look at is environment. 
 If this is the default Global environment, then we write out the function with no
 extra information. If its environment is not the global environment, then we 
 add this to a list of the unique environments. We give each environment a unique
 name and specify this name as an attribute in the <definition> tag when writing
 the body of the R function. Then, when we read these definitions back, we 
 match them to the corresponding environment.
 We return this list so that these environment can be serialized.

 For the moment, anything other than the absolutely simple data types in R
 will not work.
 */
static R_EnvironmentFunctions *
RGnumeric_writeFunctions(xmlNodePtr node, xmlDocPtr xml, xmlNsPtr nameSpace)
{
    int n = 0, i;
    GList *els;
    xmlNodePtr top;
    R_EnvironmentFunctions *topEnv, *tmp;    
    SEXP tmpEnv;
    char *name;
    int numEnvs = 1;

    topEnv = (R_EnvironmentFunctions *)g_malloc(sizeof(R_EnvironmentFunctions));

      topEnv->env = R_GlobalEnv;
      topEnv->name = NULL;
      topEnv->next = NULL;
 
    top = node;
    node = xmlNewChild(node, nameSpace, "functions", NULL);    

    els = RGnumeric_getFunctionList();
    n = g_list_length(els);
    for(i = 0; i < n; i++) {
	RGnumeric_FuncData *el;
        el = (RGnumeric_FuncData*) g_list_nth_data(els, i);

	/* Find out whether the enviroment of this function has been
           seen before. If not, add it to the list of environments that
           we have seen and will need to write out.
         */
        tmpEnv = CLOENV(el->codeobj);
        if(tmpEnv != R_GlobalEnv) {
	    tmp = topEnv;
	    while(tmp) {
		if(tmp->env == tmpEnv) 
		    break;
		tmp = tmp->next;
	    }
	    if(tmp == NULL) {
		tmp = (R_EnvironmentFunctions *)g_malloc(sizeof(R_EnvironmentFunctions));
		tmp->next = topEnv;
		tmp->name = (char *)g_malloc(sizeof(char)*10);
                tmp->functionEnv = RGnumeric_getFunctionEnv(el->codeobj);
		numEnvs++;
		sprintf(tmp->name, "%d", numEnvs);
		topEnv = tmp;
	    }
            name = tmp->name;
	} else
	    name = NULL;

        RGnumeric_writeFunction(node, el, nameSpace, name);
    }

    return(topEnv);
}

/**
  Call environment(f) where f is the function object
  passed to this routine.
  @param fun the R function  object whose environment is to be found.
 */
USER_OBJECT_
RGnumeric_getFunctionEnv(USER_OBJECT_ fun)
{
    USER_OBJECT_ e, ans;
    int errorOccurred;

    PROTECT(e = allocVector(LANGSXP, 2));
    SETCAR(e, Rf_install("environment"));
    SETCAR(CDR(e), fun);

    ans = R_tryEval(e, R_GlobalEnv, &errorOccurred);

    UNPROTECT(1);
    return(ans);
}

/**
  Write the details of the specified R-Gnumeric function as a child of the
  XML node given by `funcsNode'.
  @param funcsNode the node to which the definition of this R-Gnumeric function is 
      to be added as a child node.
  @param el the element of the R-Gnumeric function list, containing information about
   the name, arguments and definition of the R-Gnumeric function.
  @param nameSpace the XML namespace  identifying R.
  @param envName  an identifier for the environment of the function used to cross-reference
    the shared environment elements of the DOM.
 */
static int
RGnumeric_writeFunction(xmlNodePtr funcsNode, RGnumeric_FuncData *el, xmlNsPtr nameSpace, const char *envName)
{
   xmlNodePtr node, tmpNode;
   USER_OBJECT_ tmp;
   char *argTypes;
   char *defString = "function(i){ print(i); i}";

    node = xmlNewChild(funcsNode, nameSpace, "function", NULL);
    xmlSetProp(node, "name", function_def_get_name(el->fndef));

    tmpNode = xmlNewChild(node, nameSpace, "description", NULL);
#if COMPUTE_ARG_TYPES
    /* This is not used until we can retrieve this information for variable argument
       functions directly from Gnumeric. Instead, we store this information
       when the functions are registered via R.
     */
    function_def_count_args(el->fndef, &min, &max);
    n = max; 
    argTypes = (gchar *) g_malloc( (n+1) * sizeof(gchar *));
    for(i = 0; i < n; i++) {
	argTypes[i] = function_def_get_arg_type(el->fndef, i);
    }
    argTypes[n] = '\0';
#else
    argTypes = el->argTypes;
#endif

    tmpNode = xmlNewChild(node, nameSpace, "arguments", NULL);
    xmlSetProp(tmpNode, "value", argTypes);  

    xmlSetProp(tmpNode, "description", "");  
#if COMPUTE_ARG_TYPES
    g_free(argTypes);
#endif

    tmp = RGnumeric_getFunctionText(el->codeobj);
    if(tmp) {
      defString = CHAR_DEREF(STRING_ELT(tmp, 0));
      tmpNode = xmlNewChild(node, NULL, "definition", NULL);

      if(envName) {
	  xmlSetProp(tmpNode, "environment", envName);
      }
      xmlAddChild(tmpNode, xmlNewCDataBlock(funcsNode->doc, defString, strlen(defString)));
    }



    return(1);
}

/**
  Get the textual definition of the specified S function.
  This calls an S function (getFunctionString) to do the conversion.
  That function strips the environment from the function so that the
  result is a legal S string defining the function.
  This could also use dput().
  @param fun the R function object to be stored in text form.
 */
USER_OBJECT_
RGnumeric_getFunctionText(USER_OBJECT_ fun)
{
    USER_OBJECT_ e, val;
    int errorOccurred;
    PROTECT(e = allocVector(LANGSXP, 2));
    SETCAR(e, Rf_install("getFunctionString"));
    SETCAR(CDR(e), fun);
    val = R_tryEval(e, R_GlobalEnv, &errorOccurred);
    UNPROTECT(1);
    if(errorOccurred)
	return(NULL);
    return(val);
}


/**
  Read a Gnumeric file previously written using the R-Gnumeric file saver
  plugin (or created manually).
 */
void
R_file_open(GnumFileOpener const *fo, IOContext *io_context,
                  WorkbookView *wb_view, const char *filename)
{
	xmlDocPtr res;
	xmlNsPtr gmr;
	XmlParseContext *ctxt;
	GnumericXMLVersion    version;
        extern Rboolean RGnumeric_libraryAvailable;

	g_return_if_fail (filename != NULL);

	/*
	 * Load the file into an XML tree.
	 */
	res = xmlParseFile (filename);
	if (res == NULL) {
		gnumeric_io_error_read (io_context, "");
		return;
	}
	if (res->xmlRootNode == NULL) {
		xmlFreeDoc (res);
		gnumeric_io_error_read (io_context,
			_("Invalid xml file. Tree is empty ?"));
		return;
	}

	/* Do a bit of checking, get the namespaces, and check the top elem. */
	gmr = xml_check_version (res, &version);
	if (gmr == NULL) {
		xmlFreeDoc (res);
		gnumeric_io_error_read (io_context,
			_("Is not an Gnumeric Workbook file"));
		return;
	}

	/* Parse the file */
	ctxt = xml_parse_ctx_new (res, gmr);
	ctxt->version = version;
        /* Now get our R functions. */
        if(RGnumeric_libraryAvailable) {
	  R_EnvironmentFunctions *envs;

          envs = RGnumeric_readEnvironments(res->xmlRootNode);
          R_readFunctions(res->xmlRootNode, envs);
          R_processRSearchPath(res->xmlRootNode);

          RGnumeric_evalSheetInitCode(res->xmlRootNode);
	}

	xml_workbook_read (io_context, wb_view, ctxt, res->xmlRootNode);
	workbook_set_saveinfo (wb_view_workbook (wb_view), filename, FILE_FL_AUTO,
	                       gnumeric_xml_get_saver ());
	xml_parse_ctx_destroy (ctxt);
	xmlFreeDoc (res);

	return;
}


/**

 */
static void
RGnumeric_evalSheetInitCode(xmlNodePtr root)
{
     /* Should check namespace also. */
  xmlNodePtr node;
  xmlChar *code;
  node = RGnumeric_getToplevelNode(root, "init");

  if(node == NULL) 
      return;

  code = xmlNodeListGetString(node->doc, node->childs, 1);  
  if(code && code[0]) {
      RGnumeric_getFunctionObject(code);
  }
}


/**
  Read the R:searchPath element in the XML document.
  @param node the R:searchPath node in the XML document that 
     contains the individual search path elements that identify the
     packages.
 */
static int
R_processRSearchPath(xmlNodePtr node)
{
     xmlNodePtr tmp;
     int n = 0;

     tmp = node->childs;
     while(tmp) {
	 if(strcmp(tmp->name, "searchPath") == 0) {
	     break;
	 }
         tmp = tmp->next;
     }

     if(tmp == NULL)
         return(0);

     tmp = tmp->childs;
     while(tmp) {
         xmlChar * pkgName = xmlGetProp(tmp, "name");
         if(pkgName) {
	     RGnumeric_loadPackage(pkgName);
             n++;
	 }
	 tmp = tmp->next;
     }

     return(n);
}


static int
R_readFunctions(xmlNodePtr node, R_EnvironmentFunctions *envs)
{
     xmlNodePtr rfuns = NULL, tmp;
     int n = 0;

     tmp = node->childs;
     while(tmp) {
	 if(strcmp(tmp->name, "functions") == 0) {
             rfuns = tmp; 
	     break;
	 }
         tmp = tmp->next;
     }

     if(rfuns) {
	 tmp = rfuns->childs;
         while(tmp) {
	     R_addFunction(tmp, envs);
             n++;
             tmp = tmp->next;
	 }
     }

    return(n);
}


/*
 Read the elements of the XML node containing an R function definition
 and convert them to the relevant pieces and register the function with
 R and Gnumeric.
 */
static Rboolean
R_addFunction(xmlNodePtr node, R_EnvironmentFunctions *envs)
{
    xmlNodePtr els = node->childs, tmp;
    xmlChar *name = xmlGetProp(node, "name"), *envName;

    if(name) {
      xmlChar *args, *argDesc, *def, *desc;
      char **help;
      USER_OBJECT_ fun;

      tmp = els;
      desc = xmlNodeListGetString(node->doc, els->childs, 0);
      help = (char **)g_malloc(2 * sizeof(char *));     
      if(desc)
	  desc = "";
      help[0] = g_strdup(desc);
      help[1] = NULL;
      
      tmp = els->next;
      args = xmlGetProp(tmp, "value");
      argDesc = xmlGetProp(tmp, "description");

      tmp = tmp->next;
      envName = xmlGetProp(tmp, "environment");
      def = xmlNodeListGetString(node->doc, tmp->childs, 1);   
      fun = RGnumeric_getFunctionObject(def);
      if(fun) {
        PROTECT(fun);

  	/* Check if there is an environment attribute for this node
           and if there is, lookup that environment in `envs' and
           set the associated environment for this function.
         */

        if(envName) {
	    while(envs) {
		if(strcmp(envName, envs->name) == 0) {
		    SET_CLOENV(fun, envs->functionEnv);
		}
     	        envs = envs->next;
	    }
	}

        RGnumeric_internal_registerFunction(name, fun, args, argDesc, help, "R");
        UNPROTECT(1);
        return(FALSE);
      } else {
	fprintf(stderr, "cannot convert function string %s\n", def);fflush(stderr);
      }
    }

    return(TRUE);
}


/**
 Parse and evaluate the definition of a function given as
 a string in the form function(x,y, z) { <body> }
 @param def the text version of an R function.
 */
USER_OBJECT_
RGnumeric_getFunctionObject(const char *def)
{
  USER_OBJECT_ e, fun = NULL, tmp;
  int errorOccurred;
 
  PROTECT(e = allocVector(LANGSXP, 2));
  SETCAR(e, Rf_install("parseEval"));

  SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
  SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(def));
  fun = R_tryEval(e, R_GlobalEnv, &errorOccurred);
  if(errorOccurred)
      return(NULL);

  UNPROTECT(1);
  return(fun);
}

/**
  Process the R:environments tag, reading each of the individual
  R:environment elements.
 */
R_EnvironmentFunctions *
RGnumeric_readEnvironments(xmlNodePtr node)
{
  R_EnvironmentFunctions *top = NULL, *tmp;
  xmlNodePtr kid;

  node = RGnumeric_getToplevelNode(node, "environments");
  if(node == NULL)
      return(NULL);

  kid = node->childs;
  while(kid) {
      tmp = RGnumeric_readEnvironment(kid);
      if(tmp) {
        /* Add it to the front of the list. */
	  tmp->next = top;
	  top = tmp;
      }
      kid = kid->next;
  }


#if 0
  tmp = top;
  while(tmp) {
   USER_OBJECT_ f;
   f = getEnvironmentNames(tmp->functionEnv);
   fprintf(stderr, "Environment: %s\n", tmp->name);fflush(stderr);
   Rf_PrintValue(f);
   tmp = tmp->next;
  }
#endif

  return(top);
}


/**
  Read and create an R environment definition and store the information in
 a {@link #R_EnvironmentFunctions R_EnvironmentFunctions}.
 */
R_EnvironmentFunctions *
RGnumeric_readEnvironment(xmlNodePtr node)
{
  xmlNodePtr tmp;
  xmlChar *name;
  R_EnvironmentFunctions *env;

  if(node== NULL)
      return(NULL);

  env = (R_EnvironmentFunctions *)g_malloc(sizeof(R_EnvironmentFunctions));
  name = xmlGetProp(node, "name");


  env->name = strdup(name);

  env->functionEnv = RGnumeric_createEnvironment();
  R_PreserveObject(env->functionEnv);

  tmp = node->childs;
  while(tmp) {
      RGnumeric_readEnvironmentElement(tmp, env);
      tmp = tmp->next;
  }

  return(env);
}

/**
  Read an element from the R environment definition.
 */
USER_OBJECT_
RGnumeric_readEnvironmentElement(xmlNodePtr node, R_EnvironmentFunctions *env)
{
 xmlChar *def, *name;
 USER_OBJECT_ val;
 name = xmlGetProp(node, "name");
 def = xmlNodeListGetString(node->doc, node->childs, 1);   

 PROTECT(val = RGnumeric_getFunctionObject(def));
 RGnumeric_assignEnvironmentValue(name,  val, env->functionEnv);
 UNPROTECT(1);
 return(val);
}

/**
 Assign the specified value using the given name into the  
 environment.
 */
void
RGnumeric_assignEnvironmentValue(xmlChar *name, USER_OBJECT_ val, USER_OBJECT_ env)
{
    USER_OBJECT_ e, tmp; 

    PROTECT(e = allocVector(LANGSXP, 4)); 
    SETCAR(e, Rf_install("assign"));

    SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
    SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(name));

    tmp = CDR(CDR(e));
    SETCAR(tmp, val);

    tmp = CDR(CDR(CDR(e)));
    SETCAR(tmp, env);
    SET_TAG(tmp, Rf_install("envir"));

    (void) R_tryEval(e, R_GlobalEnv, NULL);

    UNPROTECT(1);

    return;
}

/**
  Create a new environment, calling the S function new.env()
 */
USER_OBJECT_
RGnumeric_createEnvironment()
{
    USER_OBJECT_ e, ans;
    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, Rf_install("new.env"));

    ans = R_tryEval(e, R_GlobalEnv, NULL);

    UNPROTECT(1);

    return(ans);
}

/**
 Load the specified R package, calling the S version of
     library(pkgName)
 ignoring Autoloads and .GlobalEnv
 This takes care of stripping any "package:" prefix.
 @param pkgName the name of the package to be loaded
 */
Rboolean
RGnumeric_loadPackage(xmlChar *pkgName)
{
    SEXP e, tmp;
    Rboolean ans = TRUE;
    int error;
    if(strncmp(pkgName, "package:", 8) == 0)
	pkgName += 8;

    if(strcmp(pkgName, "Autoloads") == 0 || strcmp(pkgName, ".GlobalEnv") == 0)
	return(TRUE);

    PROTECT(e = allocVector(LANGSXP, 2));
    SETCAR(e, Rf_install("library"));
    SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
    SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(pkgName));
    R_tryEval(e, R_GlobalEnv, &error);
    if(error)
	ans = FALSE;
    UNPROTECT(1);
    return((Rboolean) ans);
}

xmlNodePtr
RGnumeric_getToplevelNode(xmlNodePtr node, const char *tagName)
{
     xmlNodePtr tmp = node->childs;
     while(tmp) {
	 if(strcmp(tmp->name, tagName) == 0) {
             return(tmp);
	 }
         tmp = tmp->next;
     }

     return(NULL);
}
