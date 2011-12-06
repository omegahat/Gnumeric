#include "RConvert.h"

USER_OBJECT_
convertFromGnumericArg(Value *value, FunctionEvalInfo *ei, int which)
{
  USER_OBJECT_ ans = NULL_USER_OBJECT;
  switch(value->type) {
    case VALUE_BOOLEAN:
       ans = NEW_LOGICAL(1);
       LOGICAL_DATA(ans)[0] = value->v_bool.val;
      break;
    case VALUE_INTEGER:
       ans = NEW_INTEGER(1);
       INTEGER_DATA(ans)[0] = value->v_int.val;
      break;
    case VALUE_FLOAT:
       ans = NEW_NUMERIC(1);
       NUMERIC_DATA(ans)[0] = value->v_float.val;
      break;
    case VALUE_STRING:
    {
       char *tmp;
#ifdef HAVE_PEEK_STRING
       tmp = value_peek_string(value);
#else
       tmp = value->v_str.val->str;
#endif

       PROTECT(ans = NEW_CHARACTER(1));
       SET_STRING_ELT(ans, 0, COPY_TO_USER_STRING(tmp)); 
       UNPROTECT(1);
    }
      break;
    case VALUE_CELLRANGE:
      ans = convertGnumericCellRange((ValueRange*) &(value->v_range));
      break;
    case VALUE_ARRAY:
      ans = convertGnumericArray(&(value->v_array));
      break;
   case VALUE_EMPTY:
      ans = NULL_USER_OBJECT;
      break;
   case VALUE_ERROR:
        PROBLEM "errors not yet supported"
        ERROR;
     break;
   default:
     fprintf(stderr, "Unhandled case in convertFromGnumericArg %d\n", value->type);fflush(stderr);
  }
  return(ans);
}

USER_OBJECT_
convertGnumericArray(ValueArray *value)
{
  USER_OBJECT_ ans;
     /* Convert to a data frame in the near future. */
  ans = allocMatrix(REALSXP, value->x, value->y);
 
 return(ans);
}

USER_OBJECT_
convertGnumericCellRange(ValueRange *value)
{
  USER_OBJECT_ ans, tmp, names;

  PROTECT(ans = NEW_LIST(2));
  SET_VECTOR_ELT(ans, 0, tmp = NEW_INTEGER(2));
  INTEGER_DATA(tmp)[0] = value->cell.a.row + 1;
  INTEGER_DATA(tmp)[1] = value->cell.a.col + 1;


  SET_VECTOR_ELT(ans, 1, tmp = NEW_INTEGER(2));
  INTEGER_DATA(tmp)[0] = value->cell.b.row + 1;
  INTEGER_DATA(tmp)[1] = value->cell.b.col + 1;

  PROTECT(names = NEW_CHARACTER(2));
    SET_VECTOR_ELT(names, 0, COPY_TO_USER_STRING("row"));
    SET_VECTOR_ELT(names, 1, COPY_TO_USER_STRING("column"));
  
  SET_NAMES(VECTOR_ELT(ans,0), names);
  SET_NAMES(VECTOR_ELT(ans,1), names);

  PROTECT(names = NEW_CHARACTER(2));
    SET_VECTOR_ELT(names, 0, COPY_TO_USER_STRING("start"));
    SET_VECTOR_ELT(names, 1, COPY_TO_USER_STRING("end"));
  SET_NAMES(ans, names);

  PROTECT(names = NEW_CHARACTER(1));
  SET_STRING_ELT(names, 0, COPY_TO_USER_STRING("GnumericCellRange"));
  SET_CLASS(ans, names);
  UNPROTECT(4);
  
  return(ans);
}

/*
 We will want the general converter mechanism of {handling predicate, converter}
 tuples arranged in a linked list.
*/
Value *
convertToGnumericValue(USER_OBJECT_ value)
{
  Value *val;
  int n = GET_LENGTH(value);
  if(n == 0) {
    return(value_new_empty());
  }
  else if(n == 1) {
    if(IS_LOGICAL(value)) {
      val = value_new_bool(LOGICAL_DATA(value)[0] ? TRUE : FALSE);
    } else if(IS_INTEGER(value)) {
      val = value_new_int(INTEGER_DATA(value)[0]);
    } else if(IS_NUMERIC(value)) {
      val = value_new_float(NUMERIC_DATA(value)[0]);
    } else if(IS_CHARACTER(value)) {
      val = value_new_string(CHAR_DEREF(STRING_ELT(value, 0)));
    }
  } else {
    fprintf(stderr, "non-scalar returned from R. Not yet handled.");
  }

  return(val);
}
