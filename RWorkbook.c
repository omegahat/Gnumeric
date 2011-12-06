#include "RGnumeric.h"
#include "RSheet.h"
#include "RConvert.h"

#include <gtk/gtkobject.h>
#include "workbook.h"
#include "workbook-control-gui.h"
#include "workbook-view.h"

#include "sheet.h"



USER_OBJECT_ RGnumeric_workbookReference(Workbook *sheet);
Workbook *RGnumeric_resolveWorkbookReference(USER_OBJECT_ sref);

/* S entry points */
USER_OBJECT_ RGnumeric_newWorkbook();
USER_OBJECT_ RGnumeric_sheetInWorkbook(USER_OBJECT_ workbookRef, USER_OBJECT_ index);

/**

 */
USER_OBJECT_
RGnumeric_workbookReference(Workbook *sheet)
{
  USER_OBJECT_ ans, klass;
  double val = (double) (long) sheet;

  PROTECT(ans = NEW_NUMERIC(1));
   NUMERIC_DATA(ans)[0] = val;
  PROTECT(klass = NEW_CHARACTER(1)); 
   SET_STRING_ELT(klass, 0, COPY_TO_USER_STRING("GnumericWorkbookRef"));
  
  SET_CLASS(ans, klass);
  UNPROTECT(2);

  return(ans);
}

/**

 */
Workbook *
RGnumeric_resolveWorkbookReference(USER_OBJECT_ sref)
{
  Workbook *book;
  double val;
  val = NUMERIC_DATA(sref)[0];
  book = (Workbook*) (long) val;

  return(book);
}

/**
 Create a new Gnumeric workbook, optionally reading a file to
 populate it.
 */
USER_OBJECT_
RGnumeric_newWorkbook(USER_OBJECT_ fileName)
{
 WorkbookControl *wbc;

 USER_OBJECT_ ans;

 if(GET_LENGTH(fileName)) {
  wbc = workbook_control_gui_new(NULL, NULL);
#ifdef HAVE_WORKBOOK_READ
  workbook_read(wbc, CHAR_DEREF(STRING_ELT(fileName,0)));
#else
  wb_view_open (wb_control_view (wbc), wbc, CHAR_DEREF(STRING_ELT(fileName,0)), TRUE);
#endif
 } else {
  wbc = workbook_control_gui_new(NULL, workbook_new_with_sheets(1));
 }

 ans = RGnumeric_workbookReference(wb_control_workbook(wbc));

 return(ans);
}

/**
  Get a reference to a particular sheet in a Gnumeric workbook,
  identifying it either by index or by name. If an integer
  is specified, this should be 1-based (rather than 0-based).
 */
USER_OBJECT_
RGnumeric_sheetInWorkbook(USER_OBJECT_ workbookRef, USER_OBJECT_ index)
{
  Workbook *workbook;
  USER_OBJECT_ ans = NULL_USER_OBJECT;
  Sheet *sheet;

  workbook = RGnumeric_resolveWorkbookReference(workbookRef);
  if(IS_INTEGER(index))
    sheet = workbook_sheet_by_index(workbook, INTEGER_DATA(index)[0] - 1);
  else {
    sheet = workbook_sheet_by_name(workbook, CHAR_DEREF(STRING_ELT(index, 0)));
  }
  if(sheet)
    ans = RGnumeric_sheetReference(sheet);

  return(ans);
}

/**
  Get the number of worksheets in the specified workbook.
 */
USER_OBJECT_
RGnumeric_getNumSheetsInWorkbook(USER_OBJECT_ workbookRef)
{
  USER_OBJECT_ ans;
  Workbook *workbook;
   workbook = RGnumeric_resolveWorkbookReference(workbookRef);

  ans = NEW_INTEGER(1);
  INTEGER_DATA(ans)[0] = workbook_sheet_count(workbook);
  
  return(ans);
}

/**
  Get a character vector giving the names of the 
  worksheets within the Gnumeric workbook.
 */
USER_OBJECT_
RGnumeric_getSheetNames(USER_OBJECT_ workbookRef)
{
  Workbook *workbook;
  GList *sheets;
  Sheet *sheet;
  int n, i;
  USER_OBJECT_ ans;

    workbook = RGnumeric_resolveWorkbookReference(workbookRef);
    sheets = workbook_sheets(workbook);
    n = g_list_length(sheets);
    PROTECT(ans = NEW_CHARACTER(n));
    for(i = 0;  i < n ; i++) {
      sheet = (Sheet*) g_list_nth_data(sheets, i);
      SET_STRING_ELT(ans, i, COPY_TO_USER_STRING(sheet->name_unquoted));
    }
    UNPROTECT(1);

    return(ans);
}

/**
 Return a reference to the C-level containing worksheet 
 of the Gnumeric sheet identified in the S reference 
 passed to this routine.
 */
USER_OBJECT_
RGnumeric_getWorkbookFromSheet(USER_OBJECT_ sheetRef)
{
  Sheet *sheet;
  USER_OBJECT_ ans;

   sheet = RGnumeric_resolveSheetReference(sheetRef);
   ans = RGnumeric_workbookReference(sheet->workbook);

  return(ans);
}

/**
 
 */
USER_OBJECT_
RGnumeric_workbookFreeSheetName(USER_OBJECT_ name, USER_OBJECT_ workbookRef,
                                 USER_OBJECT_ suffix, USER_OBJECT_ counter)
{
  USER_OBJECT_ ans;
  char *tmp;
  Workbook *workbook;

   workbook = RGnumeric_resolveWorkbookReference(workbookRef);
   tmp = CHAR_DEREF(STRING_ELT(name, 0));
   tmp = workbook_sheet_get_free_name(workbook, tmp, LOGICAL_DATA(suffix)[0],
                                           LOGICAL_DATA(counter)[0]);
   PROTECT(ans = NEW_CHARACTER(1));
   SET_STRING_ELT(ans, 0, COPY_TO_USER_STRING(tmp));
   UNPROTECT(1);

  return(ans);
}


/**
 Recompute each sheet in a workbook and all  cells in each of these 
 sheets.
 */
USER_OBJECT_
RGnumeric_recalcWorkbook(USER_OBJECT_ workbookRef, USER_OBJECT_ all)
{
 Workbook *book;
 USER_OBJECT_ ans;

  book = RGnumeric_resolveWorkbookReference(workbookRef);

  if(LOGICAL_DATA(all)[0]) 
    workbook_recalc_all(book);
   else
    workbook_recalc(book);

  ans = NEW_LOGICAL(1);
  LOGICAL_DATA(ans)[0] = TRUE;

 return(ans);
}
