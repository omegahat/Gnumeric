/*
 This file provides the function definition/registration and
 invocation mechanism for defining R functions as Gnumeric 
 functions.
 */

#include "RGnumeric.h"

#ifdef HAVE_GNUMERIC_CONFIG_H
#include "gnumeric-config.h"
#endif

GNUMERIC_MODULE_PLUGIN_INFO_DECL;

#include "RConvert.h"
#include "RSheet.h"

#include <sys/stat.h>
#include <unistd.h>

#ifdef VERSION
#undef VERSION
#endif

#ifdef PACKAGE
#undef PACKAGE
#endif

#include "Defn.h"

Value *RGnumeric_fixedArgCall(FunctionEvalInfo *ei, Value **args);
Value *RGnumeric_varArgCall(FunctionEvalInfo *ei, GList *args);


gboolean RGnumeric_hasSheetArgument(USER_OBJECT_ func, gboolean *cellArg);

#include "RUtils.h"

#ifdef _R_
void R_PreserveObject(SEXP);
#endif


/**
  For simplicity, we currently use the same arrangement as is used in the Python
  to store a linked list of the functions that are defined. 
  We traverse this list when looking for a function definition.
  The traversal does pointer arithmetic to compare the FunctionDefinition 
  instance and then returns the associated function object.
  In the future, we can use a hash table, or even an R named list to get 
  hopefully better speed up.
 */

static GList *funclist = NULL;

GList *
RGnumeric_getFunctionList()
{
    return(funclist);
}


RGnumeric_FuncData *
RGnumeric_getFunctionDef(const gchar *name, int *n)
{
    int num = g_list_length(funclist), i;
    const char *tmp;
    for(i = 0; i < num; i++) {
    	RGnumeric_FuncData *el;
        el = (RGnumeric_FuncData*) g_list_nth_data(funclist, i);
        tmp = function_def_get_name(el->fndef);
        if(strcmp(name, tmp)==0) {
            if(n)
		*n = i;
	    return(el);
	}
    }

    return(NULL);
}


RGnumeric_FuncData *
RGnumeric_unregisterFunctionDef(const gchar *name)
{
  RGnumeric_FuncData *fun;
  int n = -1;
  fun = RGnumeric_getFunctionDef(name, &n);

  if(fun) {
      FunctionCategory *category;
      category = function_get_category("R");
      function_remove(category, name);      
      g_list_remove(funclist, fun);
  }

  return(fun);
}



/**
 Comparator for functions so that gnumeric can find the correct function.
 */
static int
fndef_compare(FuncData *fdata, FunctionDefinition *fndef)
{
   return (fdata->fndef != fndef);
}

/**
 Register an S function with gnumeric to make it available
 as a gnumeric function with name given by sname.
 */
USER_OBJECT_
RGnumeric_registerFunction(USER_OBJECT_ sname, USER_OBJECT_ func, 
                           USER_OBJECT_ sargs,  USER_OBJECT_ sargNames, 
                            USER_OBJECT_ helpString, USER_OBJECT_ scategory)
{

  /* function_add_args(); */
 char *name, *args, *arg_names, *categoryName;
 char **help;
 int n, i;

 USER_OBJECT_ ans;

 name = CHAR_DEREF(STRING_ELT(sname, 0));
 args = CHAR_DEREF(STRING_ELT(sargs, 0));
 arg_names = CHAR_DEREF(STRING_ELT(sargNames, 0));
 categoryName = CHAR_DEREF(STRING_ELT(scategory,0));

 n = GET_LENGTH(helpString);
 help = (char **) g_malloc((n+1) * sizeof(char *));
 for(i = 0;i < n; i++) {
   help[i] = g_strdup(CHAR_DEREF(STRING_ELT(helpString, i)));
 }
 help[n] = NULL;

 RGnumeric_internal_registerFunction(name, func, args, arg_names, help, categoryName);

 ans = NEW_INTEGER(1);
 INTEGER_DATA(ans)[0] = g_list_length(funclist);

 return(ans);
}

Rboolean
RGnumeric_internal_registerFunction(char *name, USER_OBJECT_ func, char *args, char *arg_names,
                                     char **help, char *categoryName)
{
 FunctionCategory *category;
 gboolean varArgs = FALSE;
 FunctionDefinition *fndef;
 FuncData *fdata;
 char *tmp;
 Rboolean append = FALSE;

     name = g_strdup(name);
     args = g_strdup(args);
     arg_names = g_strdup(arg_names);
     categoryName = g_strdup(categoryName);
     
     tmp = args;
     while(*tmp) {
      if(tmp[0] == '|') {
	 varArgs = TRUE;
         break;
      }
      tmp++;
     }

   category = function_get_category(categoryName);

 if(varArgs) {
#ifdef RGNUMERIC_DEBUG
      fprintf(stderr, "Registering variable argument function %s\n", name);fflush(stderr);
#endif
    fndef = function_add_nodes(category, name, NULL,
				    arg_names, help,
				     (FunctionNodes*) RGnumeric_varArgCall);
 } else {
  fndef = function_add_args(category, name, args, arg_names, 
                            help, (FunctionArgs*)RGnumeric_fixedArgCall);
 }

 fdata = RGnumeric_getFunctionDef(name, NULL);

 if(fdata == NULL) {
   fdata = g_new(FuncData, 1);
   append = TRUE;
 } else {
   g_free(fdata->argTypes);
 }

 fdata->fndef = fndef;
 R_PreserveObject(func);
 fdata->codeobj = func;
 fdata->argTypes = g_strdup(args);

 if(append)
     funclist = g_list_append(funclist, fdata);


#ifdef RGNUMERIC_DEBUG
 fprintf(stderr, "Registered function @ %p (%p)",fdata->fndef, fdata);fflush(stderr);
#endif

 return(append);
}


/**
  Call an S function from gnumeric that is defined with a variable number
  of arguments, i.e. has a | in the argument type string.
 */
Value *
RGnumeric_varArgCall(FunctionEvalInfo *ei, GList *args)
{

    int ErrorOccurred = FALSE;
    GList *tmp;
    USER_OBJECT_ e, ptr, fun, val;

    int nargs, i;
    Value *ans = NULL, *ev;
    GList *l;
    gboolean getsSheetArg, getsCellArg;

      /* How many arguments did we actually get. */
    nargs = g_list_length(args);

      /* Find the S function associated with this Gnumeric call. */
    l = g_list_find_custom(funclist, (gpointer)ei->func_def,
 	   	              (GCompareFunc) fndef_compare);
    if(!l) {
      ans = value_new_error(ei->pos, "Cannot find the associated R function!");
      return(ans);
    }
    fun = ((FuncData*) l->data)->codeobj;

      /* Do we want the sheet argument? */
    getsSheetArg = RGnumeric_hasSheetArgument(fun, &getsCellArg);
    if(getsSheetArg)
	nargs++;

      /* Start creating the expression to call the function. */
    PROTECT(e = allocVector(LANGSXP, nargs+1));

    tmp = args;
    SETCAR(e, fun);
    ptr = CDR(e);
       /* Add each argument, converted from Gnumeric to S. */
    for(i = 0; i < nargs; i++) {
#ifdef EXPR_EVAL
        ev = expr_eval(tmp->data, ei->pos, EVAL_PERMIT_NON_SCALAR);
#else
	ev = eval_expr(ei->pos, tmp->data, EVAL_PERMIT_NON_SCALAR);       
#endif
        SETCAR(ptr, convertFromGnumericArg(ev, ei, i));
        ptr = CDR(ptr);
	value_release (ev);
	tmp = tmp->next;
    }

    if(getsSheetArg) {
      SETCAR(ptr, RGnumeric_sheetReference(ei->pos->sheet));
      SET_TAG(ptr, Rf_install(".sheet"));
      ptr = CDR(ptr);
    }
    if(getsCellArg) {
      SETCAR(ptr, RGnumeric_getCellInfo(ei->pos));
      SET_TAG(ptr, Rf_install(".cell"));
    }

    PROTECT(val = R_tryEval(e, R_GlobalEnv, &ErrorOccurred));
    if(ErrorOccurred) {
      ErrorOccurred = FALSE;
      UNPROTECT(2);
      return(value_new_error(ei->pos, "Error in code run in R"));
    }
    ans = convertToGnumericValue(val);
    UNPROTECT(2);

    return(ans);
}

/**
  Call an S function from gnumeric.
 */
Value *
RGnumeric_fixedArgCall(FunctionEvalInfo *ei, Value **args)
{
  int ErrorOccurred;
  int argc = 0, i;
  SEXP e, val, tmp, fun, arg;
  Value *gvalue = NULL;
  GList *l;
  int min=0, max=0;
  gboolean takesSheetArg = FALSE, takesCellArg = FALSE;

  FunctionDefinition const * const fndef = ei->func_def;

  function_def_count_args (fndef, &min, &max);
  for (argc = min; argc < max; argc++)
		if (!args [argc])
			break;

#ifdef RGNUMERIC_DEBUG
  fprintf(stderr, "In fixedArgCall: # args %d\n", argc);fflush(stderr);
#endif

  l = g_list_find_custom(funclist, (gpointer)ei->func_def,
 	  	          (GCompareFunc) fndef_compare);
  if(!l) {
    gvalue = value_new_error(ei->pos, "Cannot find the associated R function!");
    return(gvalue);
  }
  fun = ((FuncData*) l->data)->codeobj;
    /* Allocate the function call. */

  takesSheetArg = RGnumeric_hasSheetArgument(fun, &takesCellArg);

  PROTECT(e = allocVector(LANGSXP, argc+1 + takesSheetArg + takesCellArg));  
  
    /* Set the elements of the call, namely the function itself and 
       then the arguments converted to S objects.
     */
  SETCAR(e, fun);
  tmp = CDR(e);
  for(i = 0; i < argc; i++) {
    arg = convertFromGnumericArg(args[i], ei, i);
    SETCAR(tmp, arg);
    tmp = CDR(tmp);
  }

    /* Need to put the argument names on these elements. */
  if(takesSheetArg) {
    SETCAR(tmp, RGnumeric_sheetReference(ei->pos->sheet));
    SET_TAG(tmp, Rf_install(".sheet"));
    tmp = CDR(tmp);
  }
  if(takesCellArg) {
    SETCAR(tmp, RGnumeric_getCellInfo(ei->pos));
    SET_TAG(tmp, Rf_install(".cell"));
  }


    /* Here we evaluate the function call.
       We have to handle errors so this is a little tricker than
       a simple call to eval(). We create an R evaluation context
       so that we can trap any errors at this point and then
       return control to Gnumeric.
       This currently requires access to R internals, specificall
       the RCNTXT definition in Defn.h. This means the R source 
       must be available to us.
     */
   ErrorOccurred = FALSE;
   PROTECT(val = R_tryEval(e, R_GlobalEnv, &ErrorOccurred));

  if(ErrorOccurred != FALSE) {
    ErrorOccurred = FALSE;
    UNPROTECT(2);
    return(value_new_error(ei->pos, "Error in code run in R"));
  }

  gvalue = convertToGnumericValue(val);

  UNPROTECT(2);
  return(gvalue);
}

#if 0
/**
 Evaluate the given expression and catch any errors,
 returning to here if any errors occur.
 This ensures that if an error occurs in the expression,
 gnumeric will still be back in control and 
*/
USER_OBJECT_
tryEval(USER_OBJECT_ e, int *ErrorOccurred)
{
   USER_OBJECT_ val = NULL_USER_OBJECT;
   RCNTXT cntxt;  
   if(ErrorOccurred)
     *ErrorOccurred = FALSE;
   begincontext(&cntxt, CTXT_RESTART, e, R_NilValue, R_GlobalEnv, R_NilValue);
   if(SETJMP(cntxt.cjmpbuf) == 0) {  
     PROTECT(val = eval(e, R_GlobalEnv));
   } else {
     if(ErrorOccurred)
        *ErrorOccurred = TRUE;
     val = NULL;
     fprintf(stderr, "Error in R call:");fflush(stderr);
     Rf_PrintValue(e);
   }
   endcontext(&cntxt);   
   if(val != NULL) {
       UNPROTECT(1);
   }

  return(val);
}
#endif


/**
 Reflectance information about what functions are currently registered.
 This currently returns just the functions, indexed by their name as a 
 simple named list.
 An ability to get the more complete description including the argument types and 
 names and the help string will be added later.
 */
USER_OBJECT_
RGnumeric_getRegisteredFunctions(USER_OBJECT_ sfull)
{
  int n =  g_list_length(funclist), i;
  FuncData *data;
  USER_OBJECT_ ans, names;
  const char *name;

  PROTECT(ans = NEW_LIST(n));
  PROTECT(names = NEW_CHARACTER(n));

  for(i = 0; i < n; i++) {
    data = (FuncData *) g_list_nth_data(funclist, i);
    SET_VECTOR_ELT(ans, i, data->codeobj);
    name = function_def_get_name(data->fndef);
    SET_STRING_ELT(names, i, COPY_TO_USER_STRING((char *)name));
  }
  
   SET_NAMES(ans, names);
  UNPROTECT(2);
  return(ans);
}

/**
 Simple test to see if there is a <i>.sheet</i> and/or a <i>.cell</i> argument in the
 formals of the S function.
 */
gboolean
RGnumeric_hasSheetArgument(USER_OBJECT_ func, gboolean *cellArg)
{
  USER_OBJECT_ e, val;
  gboolean ans;
  if(cellArg)
      *cellArg = FALSE;
  PROTECT(e = allocVector(LANGSXP, 2));
  PROTECT(val = Rf_findFun(Rf_install("gnumeric.hasSheetArgument"), R_GlobalEnv));
  SETCAR(e, val);
  SETCAR(CDR(e), func);
  PROTECT(val = eval(e, R_GlobalEnv));
  ans = LOGICAL_DATA(val)[0];
  if(cellArg && GET_LENGTH(val) > 1)
      *cellArg =  LOGICAL_DATA(val)[1];

  UNPROTECT(3);
  return(ans);
}


/**
   Reads the the specified file in R via the S function
   <b>source()</b>.
 */
Rboolean
RGnumeric_loadProfile(const char *name)
{
  int ok = 0;
  struct stat buf;

  if((stat(name, &buf)) == 0) {
      SEXP tmp, val, e;
    /* These are static methods so cannot use them:
       R_LoadProfile(fp, R_NilValue);  
       R_ReplFile(fp, R_NilValue, 0, 0);
     */

  PROTECT(e = allocVector(LANGSXP, 2));
  PROTECT(val = Rf_findFun(Rf_install("source"), R_GlobalEnv));
  SETCAR(e, val);
  PROTECT(tmp = NEW_CHARACTER(1));
  SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(name));
  SETCAR(CDR(e), tmp);
  val = eval(e, R_NilValue);
    ok = TRUE;

  UNPROTECT(3);
  }

  return(ok);
}



/*
  This is located here to avoid the aggrevating macros
  in R which prepend `Rf_' to commonly used `words' in 
  other peoples code.
 */
#ifdef eval
#undef eval
#endif
/**
  This creates an S object representing the location of 
  the cell in which an S function is being called from 
  Gnumeric.
 */
USER_OBJECT_
RGnumeric_getCellInfo(const EvalPos * const pos)
{
    USER_OBJECT_ ans;
    ans = NEW_INTEGER(2);
    INTEGER_DATA(ans)[0] = pos->eval.row + 1;
    INTEGER_DATA(ans)[1] = pos->eval.col + 1;

    return(ans);
}
