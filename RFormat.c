/**
  Routines for querying and setting the format of a cell.
  @see RCell.c

 */
#include "RGnumeric.h"
#include "RSheet.h"
#include "RUtils.h" /* for RGnumeric_setNames() */

#include "sheet.h"
#include "sheet-style.h"

#include "style.h"
#include "mstyle.h"
#include "cell.h"

#include "style-border.h"
#include "style-color.h"
#include "format.h"


USER_OBJECT_ RGnumeric_internal_copyStyle(MStyle *style, Sheet *sheet);

USER_OBJECT_ RGnumeric_internal_getBorder(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getStyleBorder(StyleBorder *border, MStyle *style);
USER_OBJECT_ RGnumeric_internal_getFormat(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getIndent(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getFont(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getColors(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getPattern(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getWrap(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getAlign(MStyle *style);
USER_OBJECT_ RGnumeric_internal_getFontStyles(MStyle *style);

/***********/


/***********/

USER_OBJECT_ RGnumeric_internal_getColor(StyleColor *color, MStyle *style);


/**
 Set the entire format of a region of cells.
 */
USER_OBJECT_
RGnumeric_setCellFormat(USER_OBJECT_ rows, USER_OBJECT_ cols,
                          USER_OBJECT_ sstyle, USER_OBJECT_ sheetRef,
                          USER_OBJECT_ copyCell)
{
  MStyle *style; 
 
  Cell *cell;
  Sheet *sheet;
  Range range;

  range.start.row = INTEGER_DATA(rows)[0] - 1;
  range.start.col = INTEGER_DATA(cols)[0] - 1;
  range.end.row =  INTEGER_DATA(rows)[1] - 1;
  range.end.col =  INTEGER_DATA(cols)[1] - 1;

  sheet = RGnumeric_resolveSheetReference(sheetRef);

  cell = sheet_cell_get(sheet, INTEGER_DATA(copyCell)[1]-1, INTEGER_DATA(copyCell)[0]-1);
  if(cell != NULL) {
     style = cell_get_mstyle(cell);
     style = mstyle_copy(style);
  } else
      style = mstyle_new_default();


  if(RGnumeric_internal_setStyle(style, sstyle)) {
     mstyle_ref(style);
     sheet_style_apply_range(sheet, &range, style);
  }

 return(sstyle);
}

enum { BORDER_SLOT, COLOR_SLOT, PATTERN_SLOT, 
       FONT_SLOT, FONT_STYLES_SLOT, FORMAT_SLOT, WRAP_SLOT, 
       ALIGN_SLOT, INDENT_SLOT, NUM_STYLE_SLOTS};

enum { COLOR_NAME_SLOT, RGB_SLOT, COLOR_INDEX, NUM_COLOR_SLOTS} ;

enum { BOLD_SLOT, ITALIC_SLOT, UNDERLINE_SLOT, STRIKE_SLOT};

enum {FONT_NAME_SLOT, FONT_DIM_SLOT, NUM_FONT_SLOTS};

gboolean
RGnumeric_internal_setStyle(MStyle *style, USER_OBJECT_ sstyle)
{
 gboolean ans = FALSE;
   //border
   ans = Internal_setStyleColors(style, VECTOR_ELT(sstyle, COLOR_SLOT));
   ans = ans || Internal_setPattern(style, VECTOR_ELT(sstyle, PATTERN_SLOT));
   ans = ans || Internal_setFont(style, VECTOR_ELT(sstyle, FONT_SLOT));
   ans = ans || Internal_setFormat(style, VECTOR_ELT(sstyle, FORMAT_SLOT));
   ans = ans || Internal_setWrap(style, VECTOR_ELT(sstyle, WRAP_SLOT));
   ans = ans || Internal_setAlign(style, VECTOR_ELT(sstyle, ALIGN_SLOT));
   ans = ans || Internal_setIndent(style, VECTOR_ELT(sstyle, INDENT_SLOT));
   ans = ans || Internal_setFontStyles(style, VECTOR_ELT(sstyle, FONT_STYLES_SLOT));
 return(ans);
}


gboolean
Internal_setFontStyles(MStyle *style, USER_OBJECT_ fontStyles)
{
  gboolean val = TRUE;
   mstyle_set_font_bold(style, LOGICAL_DATA(fontStyles)[BOLD_SLOT]);
   mstyle_set_font_italic(style, LOGICAL_DATA(fontStyles)[ITALIC_SLOT]);
   mstyle_set_font_uline(style, LOGICAL_DATA(fontStyles)[UNDERLINE_SLOT]);
   mstyle_set_font_strike(style, LOGICAL_DATA(fontStyles)[STRIKE_SLOT]);
  return(val);
}

gboolean
Internal_setFormat(MStyle *style, USER_OBJECT_ formatSlot)
{
  gboolean val = FALSE;
  StyleFormat *fmt;
  char *tmp;

  if(GET_LENGTH(formatSlot)) {
    tmp = CHAR_DEREF(STRING_ELT(formatSlot, 0));
    if(tmp && tmp[0]) {
      fmt = style_format_new_XL(tmp, TRUE);
      mstyle_set_format(style, fmt);
      val = TRUE;
    }
  }
 return(val);
}


gboolean
Internal_setFont(MStyle *style, USER_OBJECT_ fontSlot)
{
  gboolean val = FALSE;
  char *tmp;

  if(GET_LENGTH(VECTOR_ELT(fontSlot, FONT_DIM_SLOT))) {
    int cur = mstyle_get_font_size(style);
    int newVal = INTEGER_DATA(VECTOR_ELT(fontSlot, FONT_DIM_SLOT))[1];
    if(cur != newVal) {
      mstyle_set_font_size(style, newVal);
      val = TRUE;
    }
  }

  if(GET_LENGTH(VECTOR_ELT(fontSlot, FONT_NAME_SLOT))) {
    tmp = CHAR_DEREF(STRING_ELT(VECTOR_ELT(fontSlot, FONT_NAME_SLOT), 0));
    if(tmp && tmp[0]) {
      const char *orig;
      orig = mstyle_get_font_name(style);
      if(strcmp(orig, tmp) != 0) {
      fprintf(stderr, "Changing font family to %s (from %s)\n", tmp, orig);
        mstyle_set_font_name(style, tmp);
        val = TRUE;
      }
    }
  }

 return(val);
}


gboolean
Internal_setWrap(MStyle *style, USER_OBJECT_ wrapSlot)
{
  gboolean val = FALSE;
  if(mstyle_get_wrap_text(style) != LOGICAL_DATA(wrapSlot)[0]) {
     mstyle_set_wrap_text(style, LOGICAL_DATA(wrapSlot)[0]);
     val = TRUE;
  }
 return(val);
}

gboolean
Internal_setAlign(MStyle *style, USER_OBJECT_ alignSlot)
{
  gboolean val = FALSE;
  if(mstyle_get_align_h(style) != INTEGER_DATA(alignSlot)[0]) {
     mstyle_set_align_h(style, INTEGER_DATA(alignSlot)[0]);
     val = TRUE;
  }
  if(mstyle_get_align_v(style) != INTEGER_DATA(alignSlot)[1]) {
     mstyle_set_align_v(style, INTEGER_DATA(alignSlot)[1]);
     val = TRUE;
  }

 return(val);
}

gboolean
Internal_setIndent(MStyle *style, USER_OBJECT_ indentSlot)
{
  gboolean val = FALSE;
  if(mstyle_get_indent(style) != INTEGER_DATA(indentSlot)[0]) {
     mstyle_set_indent(style, INTEGER_DATA(indentSlot)[0]);
     val = TRUE;
  }

  return(val);
}

gboolean
Internal_setPattern(MStyle *style, USER_OBJECT_ patternSlot)
{
  gboolean val = FALSE;
  if(mstyle_get_pattern(style) != INTEGER_DATA(patternSlot)[0]) {
     mstyle_set_pattern(style, INTEGER_DATA(patternSlot)[0]);
     val = TRUE;
  }

  return(val);
}


gboolean
Internal_setStyleColors(MStyle *style, USER_OBJECT_ colors)
{
 StyleColor *color;
 USER_OBJECT_ tmp;
 int i, which[] ={MSTYLE_COLOR_FORE, MSTYLE_COLOR_BACK, MSTYLE_COLOR_PATTERN};
 for(i = 0; i < 3; i++) {
  tmp = VECTOR_ELT(VECTOR_ELT(colors, i), RGB_SLOT);
  color = style_color_new(INTEGER_DATA(tmp)[0], INTEGER_DATA(tmp)[1], INTEGER_DATA(tmp)[2]);
  mstyle_set_color(style, which[i], color);
 }

 return(TRUE);
}

USER_OBJECT_
RGnumeric_getCellFormat(USER_OBJECT_ which, USER_OBJECT_ sheetRef)
{
  Sheet *sheet;
  MStyle *style; 
  Cell *cell;
  USER_OBJECT_ ans;

  sheet = RGnumeric_resolveSheetReference(sheetRef);
  cell = sheet_cell_get(sheet, INTEGER_DATA(which)[1]-1, INTEGER_DATA(which)[0]-1);
  style = cell_get_mstyle(cell);

  ans = RGnumeric_internal_copyStyle(style, sheet);
  return(ans);
}


enum {S_FONT = 1, S_FONTSTYLE, S_BACKGROUND, S_FOREGROUND, S_WRAP, S_BORDER,
      S_FORMAT, S_INDENT, S_PATTERN};

USER_OBJECT_
RGnumeric_getCellFormatSetting(USER_OBJECT_ cellRef, USER_OBJECT_ setting)
{
  MStyle *style; 
  USER_OBJECT_ ans;
  Cell *cell;
  StyleColor *color;

  cell = RGnumeric_resolveCellReference(cellRef);
  style = cell_get_mstyle(cell);
  if(!style) {
    PROBLEM "Cell has no style!"
    ERROR;
  }

  switch(INTEGER_DATA(setting)[0]) {
      case S_FONT:
          ans = RGnumeric_internal_getFont(style);
	  break;
      case S_FONTSTYLE:
          ans = RGnumeric_internal_getFontStyles(style);
	  break;
      case S_BACKGROUND:
          color = mstyle_get_color(style,MSTYLE_COLOR_BACK);
          ans = RGnumeric_internal_getColor(color, style);
	  break;
      case S_FOREGROUND:
          color = mstyle_get_color(style,MSTYLE_COLOR_FORE);
          ans = RGnumeric_internal_getColor(color, style);
	  break;
      case S_WRAP:
          ans = RGnumeric_internal_getWrap(style);
	  break;
      case S_BORDER:
          ans = RGnumeric_internal_getBorder(style);
	  break;
      case S_FORMAT:
          ans = RGnumeric_internal_getFormat(style);
	  break;
      case S_INDENT:
          ans = RGnumeric_internal_getIndent(style);
	  break;
      case S_PATTERN:
          ans = RGnumeric_internal_getPattern(style);
	  break;
      default:
          ans = NULL_USER_OBJECT;
  }

  return(ans);
}



USER_OBJECT_
RGnumeric_internal_copyStyle(MStyle *style, Sheet *sheet)
{
 USER_OBJECT_ ans;
 int  numEls;
 const char *names[] = {"border", "color", "pattern", "font", 
                        "styles",
                        "format", "wrap", "align", "indent"};

 numEls = sizeof(names)/sizeof(names[0]);
 PROTECT(ans = NEW_LIST(numEls));

 SET_VECTOR_ELT(ans, BORDER_SLOT, RGnumeric_internal_getBorder(style));
 SET_VECTOR_ELT(ans, COLOR_SLOT, RGnumeric_internal_getColors(style));
 SET_VECTOR_ELT(ans, PATTERN_SLOT, RGnumeric_internal_getPattern(style));
 SET_VECTOR_ELT(ans, FONT_SLOT, RGnumeric_internal_getFont(style));
 SET_VECTOR_ELT(ans, FONT_STYLES_SLOT, RGnumeric_internal_getFontStyles(style));
 SET_VECTOR_ELT(ans, FORMAT_SLOT, RGnumeric_internal_getFormat(style));
 SET_VECTOR_ELT(ans, WRAP_SLOT, RGnumeric_internal_getWrap(style));
 SET_VECTOR_ELT(ans, ALIGN_SLOT, RGnumeric_internal_getAlign(style));
 SET_VECTOR_ELT(ans, INDENT_SLOT, RGnumeric_internal_getIndent(style));

 Internal_setNames(names, sizeof(names)/sizeof(names[0]), ans);

 UNPROTECT(1);
 return(ans);
}

USER_OBJECT_
RGnumeric_internal_getBorder(MStyle *style)
{
 USER_OBJECT_ ans, snames;
 StyleBorder *border ;
 int i, numTypes;
 char *names[] = {"top", "bottom", "left", "right", "revDiagonal","diagonal"};
  
  numTypes = sizeof(names)/sizeof(names[0]);
  PROTECT(ans = NEW_LIST(numTypes));
  PROTECT(snames = NEW_CHARACTER(numTypes));
  for(i = 0; i < numTypes; i++) {
    border = mstyle_get_border(style, MSTYLE_BORDER_TOP + i);    
    SET_VECTOR_ELT(ans, i, RGnumeric_internal_getStyleBorder(border, style));
    SET_STRING_ELT(snames, i, COPY_TO_USER_STRING(names[i]));
  }

  SET_NAMES(ans, snames);
  UNPROTECT(2);

 return(ans);
}

enum {BORDER_LINE_TYPE_SLOT, BORDER_MARGIN_SLOT,  BORDER_COLOR_SLOT};
USER_OBJECT_
RGnumeric_internal_getStyleBorder(StyleBorder *border, MStyle *style)
{
  USER_OBJECT_ ans, tmp;
  const char *marginNames[] = {"begin", "end", "width"};
  const char *names[] = {"lineType", "margin", "color"};
  PROTECT(ans = NEW_LIST(3));
   
  SET_VECTOR_ELT(ans, BORDER_LINE_TYPE_SLOT, tmp = NEW_INTEGER(1));
   INTEGER_DATA(tmp)[0] = border->line_type;
  SET_VECTOR_ELT(ans, BORDER_MARGIN_SLOT, tmp = NEW_INTEGER(3));
   INTEGER_DATA(tmp)[0] = border->begin_margin;
   INTEGER_DATA(tmp)[1] = border->end_margin;
   INTEGER_DATA(tmp)[2] = border->width;
  Internal_setNames(marginNames, 3, tmp);

  SET_VECTOR_ELT(ans, BORDER_COLOR_SLOT, RGnumeric_internal_getColor(border->color, style));

  Internal_setNames(names, sizeof(names)/sizeof(names[0]), ans);


  UNPROTECT(1);
return(ans);
}




USER_OBJECT_
RGnumeric_internal_getPattern(MStyle *style)
{
  USER_OBJECT_ ans;

  ans = NEW_INTEGER(1);
  INTEGER_DATA(ans)[0] = mstyle_get_pattern(style);

 return(ans);
}

#ifdef FONT_WIDTH_POINTS
#define STYLE_FONT_GET_WIDTH style_font_get_width_pts
#else
#define STYLE_FONT_GET_WIDTH style_font_get_width
#endif

USER_OBJECT_
RGnumeric_internal_getFont(MStyle *style)
{
  USER_OBJECT_ ans, tmp;
  const char *names[] = {"name", "dimensions"};
  const char *dimNames[] = {"width", "height"};
  StyleFont *font;
  font = mstyle_get_font(style, 1.0);
  PROTECT(ans = NEW_LIST(NUM_FONT_SLOTS));
  SET_VECTOR_ELT(ans, FONT_NAME_SLOT, tmp = NEW_CHARACTER(1));
  SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(font->font_name));

  SET_VECTOR_ELT(ans, FONT_DIM_SLOT, tmp = NEW_INTEGER(2));
   INTEGER_DATA(tmp)[0] = STYLE_FONT_GET_WIDTH(font);
   INTEGER_DATA(tmp)[1] = style_font_get_height(font);
   Internal_setNames(dimNames, sizeof(dimNames)/sizeof(dimNames[0]), tmp);

  Internal_setNames(names, sizeof(names)/sizeof(names[0]), ans);
  UNPROTECT(1);

 return(ans);
}


USER_OBJECT_
RGnumeric_internal_getWrap(MStyle *style)
{
  USER_OBJECT_ ans;

  ans = NEW_LOGICAL(1);
  LOGICAL_DATA(ans)[0] = mstyle_get_wrap_text(style);

 return(ans);
}


USER_OBJECT_
RGnumeric_internal_getFormat(MStyle *style)
{
  USER_OBJECT_ ans;
  StyleFormat *fmt;
  PROTECT(ans = NEW_CHARACTER(1));
  fmt = mstyle_get_format(style);
  if(fmt) {
    char *tmp = style_format_as_XL(fmt, TRUE);
    if(tmp) {
      SET_STRING_ELT(ans, 0, COPY_TO_USER_STRING(tmp));
      g_free(tmp);
    }
  }
  UNPROTECT(1);

  return(ans);
}

USER_OBJECT_
RGnumeric_internal_getAlign(MStyle *style)
{
  USER_OBJECT_ ans;
  const char *names[] = {"horizontal","vertical"};
  PROTECT(ans = NEW_INTEGER(2));
   INTEGER_DATA(ans)[0] = mstyle_get_align_h(style);
   INTEGER_DATA(ans)[1] = mstyle_get_align_v(style);

  Internal_setNames(names, 2, ans);
  UNPROTECT(1);
  return(ans);
}


USER_OBJECT_
RGnumeric_internal_getIndent(MStyle *style)
{
  USER_OBJECT_ ans;
  ans = NEW_INTEGER(1);
  INTEGER_DATA(ans)[0] = mstyle_get_indent(style);

  return(ans);
}



USER_OBJECT_
RGnumeric_internal_getColors(MStyle *style)
{
 StyleColor *color;
 USER_OBJECT_ ans;
 const char *names[] = {"foreground", "background", "pattern"};
 PROTECT(ans = NEW_LIST(sizeof(names)/sizeof(names[0])));

   color = mstyle_get_color(style,MSTYLE_COLOR_FORE);
   SET_VECTOR_ELT(ans, 0, RGnumeric_internal_getColor(color, style));
   color = mstyle_get_color(style,MSTYLE_COLOR_BACK);
   SET_VECTOR_ELT(ans, 1, RGnumeric_internal_getColor(color, style));
   color = mstyle_get_color(style,MSTYLE_COLOR_PATTERN);
   SET_VECTOR_ELT(ans, 2, RGnumeric_internal_getColor(color, style));

  Internal_setNames(names, sizeof(names)/sizeof(names[0]), ans);

 UNPROTECT(1);
 return(ans);
}

USER_OBJECT_
RGnumeric_internal_getColor(StyleColor *color, MStyle *style)
{
  USER_OBJECT_ ans, tmp;
  const char *names[] = {"name", "rgb", "index"};
  PROTECT(ans = NEW_LIST(NUM_COLOR_SLOTS));

  SET_VECTOR_ELT(ans, RGB_SLOT, tmp = NEW_INTEGER(3));
    INTEGER_DATA(tmp)[0] = color->red;
    INTEGER_DATA(tmp)[1] = color->green;
    INTEGER_DATA(tmp)[2] = color->blue;

  SET_VECTOR_ELT(ans, COLOR_NAME_SLOT, tmp = NEW_CHARACTER(1));
  if(color->name)
    SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(color->name));

#if 0
  SET_VECTOR_ELT(ans, COLOR_INDEX, tmp = NEW_NUMERIC(2));
    NUMERIC_DATA(tmp)[0] = (double) (color->color);
    NUMERIC_DATA(tmp)[1] = (double) (color->selected_color);
#endif

  Internal_setNames(names, NUM_COLOR_SLOTS, ans);
  UNPROTECT(1);

  return(ans);
}



USER_OBJECT_ 
RGnumeric_internal_getFontStyles(MStyle *style)
{
 USER_OBJECT_ ans;
 const char *names[] = {"bold", "italic", "underline", "strike"};
  PROTECT(ans = NEW_LOGICAL(sizeof(names)/sizeof(names[0])));

  LOGICAL_DATA(ans)[BOLD_SLOT] = mstyle_get_font_bold(style);
  LOGICAL_DATA(ans)[ITALIC_SLOT] = mstyle_get_font_italic(style);
  LOGICAL_DATA(ans)[UNDERLINE_SLOT] = mstyle_get_font_uline(style);
  LOGICAL_DATA(ans)[STRIKE_SLOT] = mstyle_get_font_strike(style);

  Internal_setNames(names, sizeof(names)/sizeof(names[0]), ans);
  UNPROTECT(1);

 return(ans);
}


MStyle *
createStyle(USER_OBJECT_ vals)
{
 MStyle *style;

 style = mstyle_new();

   mstyle_set_font_bold(style, TRUE);
   mstyle_set_font_italic(style, TRUE);

 return(style);
}
