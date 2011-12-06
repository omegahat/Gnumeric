#include "RGnumeric.h"
#include "RUtils.h"
#include "RSheet.h"

#include <sheet.h>

#include <mstyle.h>
#include <style-color.h>
#include <sheet-style.h>
#include <cell.h>


/**
 Get a reference to a Cell in the specified sheet
 and return it as an S object. This contains the pointer
 to the internal C value. It can be used in subsequent
 calls to identify the cell. Under no circumstances
 should this be saved and reused across R sessions.
 */
USER_OBJECT_
RGnumeric_getCellReference(USER_OBJECT_ sheetRef, USER_OBJECT_ row, USER_OBJECT_ col)
{
 Sheet *sheet;
 Cell *cell;
 USER_OBJECT_ ans;

  sheet = RGnumeric_resolveSheetReference(sheetRef);
  cell = sheet_cell_get(sheet, INTEGER_DATA(col)[0] -1, INTEGER_DATA(row)[0] - 1);
  if(cell) {
    ans = RGnumeric_createCellReference(cell, sheet);
  } else {
    PROBLEM "No such cell @ %d, %d",
             INTEGER_DATA(col)[0] -1, INTEGER_DATA(row)[0] - 1
    ERROR;
  }
  return(ans);
}

/**
 Get either the cell horizontal or vertical alignment setting for the specified cell.
 This just returns the value of the alignment setting. We put a symbolic name 
 on it in S.
 */
USER_OBJECT_
RGnumeric_getCellAlign(USER_OBJECT_ scell, USER_OBJECT_ horizontal)
{
  USER_OBJECT_ ans;
  MStyle *style;
  Cell *cell;
   cell = RGnumeric_resolveCellReference(scell);
   style = cell_get_mstyle(cell);

   ans = NEW_INTEGER(1);
   INTEGER_DATA(ans)[0] = LOGICAL_DATA(horizontal)[0] ? 
                              mstyle_get_align_h(style) : mstyle_get_align_v(style);

 return(ans);
}

/**
  Get the row and column indices for the specified 
  GnumericCellRef object.
 */
USER_OBJECT_
RGnumeric_getCellPosition(USER_OBJECT_ scell)
{
  USER_OBJECT_ ans;
  Cell *cell;
   cell = RGnumeric_resolveCellReference(scell);

  ans = NEW_INTEGER(2);
  INTEGER_DATA(ans)[0] = cell->pos.row;
  INTEGER_DATA(ans)[1] = cell->pos.col;
 
 return(ans);
}

USER_OBJECT_
RGnumeric_setCellForeground(USER_OBJECT_ cellRef, USER_OBJECT_ val, USER_OBJECT_ isFg)
{
  MStyle *style;
  Cell *cell;
  enum _MStyleElementType attr;
  int *v = INTEGER_DATA(val);

   attr = LOGICAL_DATA(isFg)[0] ? MSTYLE_COLOR_FORE : MSTYLE_COLOR_BACK;

   cell = RGnumeric_resolveCellReference(cellRef);
   style = cell_get_mstyle(cell);

   style = mstyle_copy(style);
   mstyle_set_color(style, attr, 
                       style_color_new(v[0], v[1], v[2]));

   updateSCell(cell, cellRef, style);

   return(NULL_USER_OBJECT);
}


void
updateSCell(Cell *cell, USER_OBJECT_ cellRef, MStyle *style)
{
 Sheet *sheet; 
   sheet = RGnumeric_resolveSheetReference(VECTOR_ELT(cellRef, 1));
   updateCell(cell, sheet, style);
}

void
updateCell(Cell *cell, Sheet *sheet, MStyle *style)
{
 Range range;
   mstyle_ref(style);
   range.start = cell->pos;
   range.end = cell->pos;

   sheet_style_apply_range(sheet, &range, style);
}


USER_OBJECT_
RGnumeric_setCellAlign(USER_OBJECT_ scell, USER_OBJECT_ value, USER_OBJECT_ horizontal)
{
  MStyle *style;
  Cell *cell;

   cell = RGnumeric_resolveCellReference(scell);
   style = cell_get_mstyle(cell);

   if(LOGICAL_DATA(horizontal)[0]) 
    mstyle_set_align_h(style, INTEGER_DATA(value)[0]) ;
   else
    mstyle_set_align_v(style, INTEGER_DATA(value)[0]);


   updateSCell(cell, scell, style);

 return(value);
}


USER_OBJECT_
RGnumeric_setCellFont(USER_OBJECT_ scell, USER_OBJECT_ value, USER_OBJECT_ styles)
{
  MStyle *style;
  Cell *cell;

   cell = RGnumeric_resolveCellReference(scell);
   style = cell_get_mstyle(cell);

    if(GET_LENGTH(value)) {
	Internal_setFont(style, value);
    }
    if(GET_LENGTH(styles)) {
	Internal_setFontStyles(style, styles);
    }

   updateSCell(cell, scell, style);

   return(value);
}


USER_OBJECT_
RGnumeric_setCellWrap(USER_OBJECT_ scell, USER_OBJECT_ value)
{
  MStyle *style;
  Cell *cell;

   cell = RGnumeric_resolveCellReference(scell);
   style = cell_get_mstyle(cell);

   Internal_setWrap(style, value);

   updateSCell(cell, scell, style);

   return(value);
}


USER_OBJECT_
RGnumeric_setCellPattern(USER_OBJECT_ scell, USER_OBJECT_ value)
{
  MStyle *style;
  Cell *cell;

   cell = RGnumeric_resolveCellReference(scell);
   style = cell_get_mstyle(cell);

   Internal_setPattern(style, value);

   updateSCell(cell, scell, style);

   return(value);
}


USER_OBJECT_
RGnumeric_setCellIndent(USER_OBJECT_ scell, USER_OBJECT_ value)
{
  MStyle *style;
  Cell *cell;

   cell = RGnumeric_resolveCellReference(scell);
   style = cell_get_mstyle(cell);

   Internal_setIndent(style, value);

   updateSCell(cell, scell, style);

   return(value);
}


USER_OBJECT_
RGnumeric_createCellReference(Cell *cell, Sheet *sheet)
{
  USER_OBJECT_ ans, klass;
  const char *elNames[] = {"cell", "sheet"};
  PROTECT(ans = NEW_LIST(2));
    SET_VECTOR_ELT(ans, 0, RGnumeric_createInternalReference((void*)cell, "GnumericCellPtr"));
    SET_VECTOR_ELT(ans, 1, RGnumeric_sheetReference(sheet));
    Internal_setNames(elNames, 2, ans);
  PROTECT(klass = NEW_CHARACTER(1));
  SET_STRING_ELT(klass, 0, COPY_TO_USER_STRING("GnumericCellRef"));
  SET_CLASS(ans, klass);
  UNPROTECT(2);
  return(ans);
}


Cell *
RGnumeric_resolveCellReference(USER_OBJECT_ scell)
{
  Cell *cell = NULL;
  if(IsInstanceOf(scell, "GnumericCellRef")) {
     cell = (Cell*) RGnumeric_resolveInternalReference(VECTOR_ELT(scell, 0), (char *)NULL);
  } else {
      PROBLEM "Object not of GnumericCellRef class passed to resolveCellReference"
      ERROR;
  }

  return(cell);
}

#include <sheet-object-cell-comment.h>

USER_OBJECT_
RGnumeric_getCellComment(USER_OBJECT_ scell)
{
 Cell *cell;
 CellComment *comment;
 char const *tmp;
 USER_OBJECT_ ans;
 const char *slotNames[] = {"comment", "author"};

  cell = RGnumeric_resolveCellReference(scell);

  if(!cell) {
    PROBLEM "cell is empty"
    ERROR;
  }

  comment = cell_has_comment(cell);

  if(comment == NULL) {
      return(NULL_USER_OBJECT);
  }

  PROTECT(ans = NEW_CHARACTER(2));
  tmp = cell_comment_text_get(comment);
  if(tmp) {
      SET_STRING_ELT(ans, 0, COPY_TO_USER_STRING(tmp));
  }

  tmp = cell_comment_author_get(comment);
  if(tmp) {
      SET_STRING_ELT(ans, 1, COPY_TO_USER_STRING(tmp));
  }
  Internal_setNames(slotNames, 2, ans);

  UNPROTECT(1);
  return(ans);
}

USER_OBJECT_
RGnumeric_setCellComment(USER_OBJECT_ scell, USER_OBJECT_ stext, USER_OBJECT_ sauthor)
{
  char *text = NULL, *author = NULL;
  CellComment *comment;
  Cell *cell;
  USER_OBJECT_ ans = NULL_USER_OBJECT;

  cell = RGnumeric_resolveCellReference(scell);
  if(!cell)
    PROBLEM "invalid cell reference object"
    ERROR;

  if(GET_LENGTH(stext))
    text = CHAR_DEREF(STRING_ELT(stext, 0));
  if(GET_LENGTH(sauthor))
    author = CHAR_DEREF(STRING_ELT(sauthor, 0));

  comment = cell_has_comment(cell); 
  if(!comment) {
      Sheet *sheet;
        sheet = RGnumeric_resolveSheetReference(VECTOR_ELT(scell, 1));
        cell_set_comment(sheet, &(cell->pos), (const char*) author, (const char *)text);
  } else {
      if(text)
        cell_comment_text_set(comment, text);
      if(author)
        cell_comment_author_set(comment, author);
  }


  return(ans);
}
