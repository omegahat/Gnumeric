/* @file */
#include "RUtils.h"

/**
  Utility to create an S object that contains the address of a
  C level object and has a particular S class specified by
  className.
  This is used to create objects of class GnumericSheetRef,
  GnumericCellRef, etc.

  @param ptr the C-level address to be stored in the S object.
  @param className the name of the S class to be assigned to this 
   object
 */
USER_OBJECT_
RGnumeric_createInternalReference(void *ptr, const char *className)
{
  USER_OBJECT_ ans, klass;
  double val = (double) (long) ptr;
  PROTECT(ans = NEW_NUMERIC(1));
   NUMERIC_DATA(ans)[0] = val;
  PROTECT(klass = NEW_CHARACTER(1)); 
   SET_STRING_ELT(klass, 0, COPY_TO_USER_STRING(className));

  SET_CLASS(ans, klass);
  UNPROTECT(2);

  return(ans);
}

/**
  This dereferences the C-level address stored in an S reference
  object. If checkClass is specified, we verify that the S object
  is an instance of this class. This is done by comparing the 
  className to each of the elements in the "class" attribute
  of the S object.
  @see #IsInstanceOf
 
  @param sref the S object typically generated previously 
     by RGnumeric_createInternalReference
  @param checkClass the name of the class of which the S object
    is expected to be an instance. If this is non-trivial
     (i.e. not-NULL and not the empty string ""), the class(es)
     of the S object is compared to this and the value is returned
     only if it is an instance of this class.
 */

void *
RGnumeric_resolveInternalReference(USER_OBJECT_ sref, const char *checkClass)
{
  void *ptr;
  double val;
  if(checkClass && checkClass[0] && !IsInstanceOf(sref, checkClass)) {
    PROBLEM "Object is not an object of class %s", checkClass
    ERROR;
  }
  val = NUMERIC_DATA(sref)[0];
  ptr = (void*) (long) val;

  return(ptr);
}

/**
  Check the S object is an instance of the class specified
   by className.

 */
Rboolean
IsInstanceOf(USER_OBJECT_ sobj, const char *className)
{
  USER_OBJECT_ klasses = GET_CLASS(sobj);
  int i;

  for(i = 0; i < GET_LENGTH(klasses); i++) {
    if(strcmp(className, CHAR_DEREF(STRING_ELT(klasses, i))) == 0)
      return(TRUE);
  }

  return(FALSE);
}

/**
  Convenience method for setting the names of a list or vector
  by copying the C-level names to an S character vector and 
  setting this as the "names" attribute for the target S object.
 */
void
Internal_setNames(const char ** const names, int num, USER_OBJECT_ ans)
{
 USER_OBJECT_ snames;
 int i;
  PROTECT(snames = NEW_CHARACTER(num));
  for(i = 0; i < num;i++)
    SET_STRING_ELT(snames, i, COPY_TO_USER_STRING(names[i]));
  SET_NAMES(ans, snames);

  UNPROTECT(1);
}

