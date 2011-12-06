#include "RSheet.h"
#include "RConvert.h"
#include "RUtils.h"

#include "cell.h"
#include "workbook.h"

#include "sheet.h"

/**
  Get the value of one or more cells in a sheet.
  
  @param sheetRef S reference to a Gnumeric sheet, passed to S
  via the <code>.sheet</code> argument 
  @param args  a collection of the ... arguments specified in S
   to identify the cell of interest.
 */
USER_OBJECT_
RGnumeric_getSheetCell(USER_OBJECT_ sheetRef, USER_OBJECT_ args)
{
  Sheet *sheet;
  int nargs;
  int n , i, nrow, ncol, r, c;
  Cell *cell;
  USER_OBJECT_ ans = NULL_USER_OBJECT, tmp;

  sheet = RGnumeric_resolveSheetReference(sheetRef);

  nargs = GET_LENGTH(args);

  if(nargs == 1) {
    /* Getting a row */
  } else if(nargs == 2) {
    USER_OBJECT_ rows, cols;
      rows = VECTOR_ELT(args, 0);
      cols = VECTOR_ELT(args, 1);
      nrow = GET_LENGTH(rows);
      ncol = GET_LENGTH(cols);
     if(nrow && ncol) {
         /* Getting both. */
      n = MAX(nrow, ncol);
      PROTECT(ans = NEW_LIST(n));
      for(i = 0; i < n; i++) {
          /* These are here for debugging purposes for use with gdb/dbx. */
        r = INTEGER_DATA(rows)[i%nrow]-1;
        c =  INTEGER_DATA(cols)[i%ncol]-1;
        cell = sheet_cell_get(sheet, c, r);
        if(cell) {
          tmp =  convertFromGnumericArg(cell->value, NULL, -1);
          SET_VECTOR_ELT(ans, i, tmp);
	}
      }
      UNPROTECT(1);
    } else if(ncol) {
      /* Getting columns, i.e. [,...] */ 

 
    } else if(nrow) {
      /* Getting rows, i.e. [..., ] */ 
 
    }
  }

  return(ans);
}

/**
 Set the value of a cell identified by row and column in the specified sheet
 with the S value that is to be converted to a Gnumeric value.

  @param sheetRef
  @param rows
  @param cols
  @param val
 */
USER_OBJECT_
RGnumeric_setCellValue(USER_OBJECT_ sheetRef, USER_OBJECT_ rows, USER_OBJECT_ cols, 
                        USER_OBJECT_ val)
{
  Sheet *sheet;
  Cell *cell;

  USER_OBJECT_ ans = NULL_USER_OBJECT;

   sheet = RGnumeric_resolveSheetReference(sheetRef);

   cell = sheet_cell_fetch(sheet, INTEGER_DATA(cols)[0]-1, INTEGER_DATA(rows)[0]-1);

 if(IS_CHARACTER(val)) {   
   sheet_cell_set_text(cell, CHAR_DEREF(STRING_ELT(val,0)));
 } else {
   Value *v;
   v = convertToGnumericValue(val);
   sheet_cell_set_value(cell, v, NULL);
 }

 return(ans); 
}


/**
 Get the dimensions of the specified sheet, either
 as the number of rows and columns (if collapse is TRUE), or 
 as a list containing the starting row and column
 and the ending row and column.
 */
USER_OBJECT_
RGnumeric_getSheetDim(USER_OBJECT_ sheetRef, USER_OBJECT_ collapse)
{
 USER_OBJECT_ ans;
 Sheet *sheet;
 Range range;

  sheet = RGnumeric_resolveSheetReference(sheetRef);
  range = sheet_get_extent(sheet
#ifdef EXTENT_SPANS_AND_MERGE
			   , TRUE /* or perhaps false */          
#endif
                          );

  if(LOGICAL_DATA(collapse)[0] == TRUE) {
    ans = NEW_INTEGER(2);

    INTEGER_DATA(ans)[0] = range.end.row - range.start.row + 1;
    INTEGER_DATA(ans)[1] = range.end.col - range.start.col + 1;
  } else {
    USER_OBJECT_ tmp;
    PROTECT(ans = NEW_LIST(2));
    SET_VECTOR_ELT(ans, 0, tmp =  NEW_INTEGER(2));
      INTEGER_DATA(tmp)[0] = range.start.row + 1;
      INTEGER_DATA(tmp)[1] = range.start.col + 1;

    SET_VECTOR_ELT(ans, 1, tmp =  NEW_INTEGER(2));
      INTEGER_DATA(tmp)[0] = range.end.row + 1;
      INTEGER_DATA(tmp)[1] = range.end.col + 1;

    UNPROTECT(1);
  }

  return(ans);
}


/**
  Change the name of the specified sheet to the value given in sname.
 */
USER_OBJECT_
RGnumeric_renameSheet(USER_OBJECT_ sheetRef, USER_OBJECT_ sname)
{
 USER_OBJECT_ ans = NULL_USER_OBJECT;
 Sheet *sheet;

  sheet = RGnumeric_resolveSheetReference(sheetRef);

  sheet_rename(sheet, CHAR_DEREF(STRING_ELT(sname, 0)));

  return(ans);
}

/**
 Clear the entire contents of the sheet.
 */
USER_OBJECT_
RGnumeric_clearSheet(USER_OBJECT_ sheetRef, USER_OBJECT_ sname)
{
 USER_OBJECT_ ans = NULL_USER_OBJECT;
 Sheet *sheet;

  sheet = RGnumeric_resolveSheetReference(sheetRef);

  sheet_destroy_contents(sheet);

  return(ans);
}


/**
 Not implemented yet.
 */
USER_OBJECT_
RGnumeric_clearRegion(USER_OBJECT_ rows, USER_OBJECT_ cols, USER_OBJECT_ sheetRef)
{
 USER_OBJECT_ ans = NULL_USER_OBJECT;
 Sheet *sheet;

  sheet = RGnumeric_resolveSheetReference(sheetRef);
  /* Need to get the WorkbookControl object. */
#if 0
  sheet_clear_region(sheet, 
                      INTEGER_DATA(cols)[0], INTEGER_DATA(rows)[0],
                      INTEGER_DATA(cols)[1], INTEGER_DATA(rows)[1],
                      CLEAR_VALUES);
#endif
  return(ans);
}

/**
 Creates a new sibling sheet in the workbook of the specified
 sheet. The name is used as the tab identifier.
 */

USER_OBJECT_
RGnumeric_newSheet(USER_OBJECT_ sname, USER_OBJECT_ sheetRef)
{
 Sheet *sheet;
 char const *name;
 USER_OBJECT_ ans;
 Workbook *wb;
 sheet = RGnumeric_resolveSheetReference(sheetRef);
 wb = sheet->workbook;

 if(GET_LENGTH(sname))
   name = CHAR_DEREF(STRING_ELT(sname,0));
 else
   name = workbook_sheet_get_free_name (wb, "R", TRUE, TRUE);

 sheet = sheet_new(wb, name);
 if(sheet) {
   ans = RGnumeric_sheetReference(sheet);
   workbook_sheet_attach(wb, sheet, NULL);
 } else
   PROBLEM "Cannot create new sheet\n"
   ERROR;

 return(ans); 
}

/**
 Create an S GnumericSheetRef object as a pointer to 
 a sheet for the duration of this call.
 */
USER_OBJECT_
RGnumeric_sheetReference(Sheet *sheet)
{
  USER_OBJECT_ ans, klass;
  double val = (double) (long) sheet;
  PROTECT(ans = NEW_NUMERIC(1));
   NUMERIC_DATA(ans)[0] = val;
  PROTECT(klass = NEW_CHARACTER(1)); 
   SET_STRING_ELT(klass, 0, COPY_TO_USER_STRING("GnumericSheetRef"));

  SET_CLASS(ans, klass);
  UNPROTECT(2);

  return(ans);
}




/**
 Gets the pointer inside the GnumericSheetRef object.
 */
Sheet *
RGnumeric_resolveSheetReference(USER_OBJECT_ sref)
{
  Sheet *sheet;
  double val;
  val = NUMERIC_DATA(sref)[0];
  sheet = (Sheet*) (long) val;

  return(sheet);
}

/**
 Get the name of the specified sheet.
 */
USER_OBJECT_
RGnumeric_getSheetName(USER_OBJECT_ sheetRef)
{
  USER_OBJECT_ ans = NULL_USER_OBJECT;
  Sheet *sheet;

    sheet = RGnumeric_resolveSheetReference(sheetRef); 
    if(!sheet) {
      PROBLEM "sheet reference doesn't correspond to a Gnumeric worksheet"
      ERROR;
    }

      PROTECT(ans = NEW_CHARACTER(1));
      SET_STRING_ELT(ans, 0, COPY_TO_USER_STRING(sheet->name_unquoted));
      UNPROTECT(1);

    return(ans);
}

/**
  Remove the specified sheet from the Gnumeric workbook.
 */
USER_OBJECT_
RGnumeric_removeSheet(USER_OBJECT_ sheetRef)
{
 Sheet *sheet;
 USER_OBJECT_ ans;
  
  sheet = RGnumeric_resolveSheetReference(sheetRef); 
  workbook_sheet_detach(sheet->workbook, sheet);

  /*  */
  ans = NEW_LOGICAL(1);
  LOGICAL_DATA(ans)[0] = TRUE;
 return(ans);
}


/**
 Check whether the cell in the position (r, c) in the given Gnumeric
 worksheet has a value or not.
 */
USER_OBJECT_
RGnumeric_isCellEmpty(USER_OBJECT_ r, USER_OBJECT_ c, USER_OBJECT_ sheetRef)
{
  Sheet *sheet;
  Range range;
  gboolean isEmpty;
  USER_OBJECT_ ans;

    sheet = RGnumeric_resolveSheetReference(sheetRef); 
   
    range.start.row = INTEGER_DATA(r)[0] -1;
    range.start.col = INTEGER_DATA(c)[0] -1;

    range.end.row = INTEGER_DATA(r)[1] -1;
    range.end.col = INTEGER_DATA(c)[1] -1;

    isEmpty = sheet_is_region_empty(sheet, (Range const *) &r);

    ans = NEW_LOGICAL(1);
    LOGICAL_DATA(ans)[0] = isEmpty;
    return(ans);

/* sheet_is_cell_empty(INTEGER_DATA(pos)[1], INTEGER_DATA(pos)[0]) */
}

/**
  Set the location of the cursor in the given sheet.
 */
USER_OBJECT_
RGnumeric_setCursor(USER_OBJECT_ r, USER_OBJECT_ c, USER_OBJECT_ ssheet)
{
 Sheet *sheet;
 USER_OBJECT_ ans;

  sheet = RGnumeric_resolveSheetReference(ssheet);
  sheet_cursor_set(sheet, 
                    INTEGER_DATA(c)[0]-1, INTEGER_DATA(r)[0]-1,
                    INTEGER_DATA(c)[0]-1, INTEGER_DATA(r)[0]-1,
                    INTEGER_DATA(c)[0]-1, INTEGER_DATA(r)[0]-1
#ifdef RANGE_FOR_CURSOR_SET
		   , NULL
#endif
                  );

  ans = NEW_LOGICAL(1);
  LOGICAL_DATA(ans)[0] = TRUE;
  return(ans);
}


/**

 */
USER_OBJECT_
RGnumeric_setVisibleCell(USER_OBJECT_ r, USER_OBJECT_ c, USER_OBJECT_ ssheet)
{
 Sheet *sheet;
 USER_OBJECT_ ans;

  sheet = RGnumeric_resolveSheetReference(ssheet);
/*XX This is deprecated in Gnumeric now. */
#ifdef SHEET_SET_CELL_VISIBLE_WITH_PANES
  sheet_make_cell_visible(sheet, INTEGER_DATA(c)[0]-1, INTEGER_DATA(r)[0]-1, FALSE);
#else
  sheet_make_cell_visible(sheet, INTEGER_DATA(c)[0]-1, INTEGER_DATA(r)[0]-1);
#endif


  ans = NEW_LOGICAL(1);
  LOGICAL_DATA(ans)[0] = TRUE;
  return(ans);
}
