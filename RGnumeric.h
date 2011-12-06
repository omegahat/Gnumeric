#ifndef R_GNUMERIC_H
#define R_GNUMERIC_H


#include <config.h>
#include <gnome.h>
#include <string.h>
#include "gnumeric.h"
#include "func.h"
#include "plugin.h"
#include "plugin-util.h"
#include "error-info.h"
#include "module-plugin-defs.h"
#include "gutils.h"
#include "value.h"
#include "command-context.h"

#ifdef CHAR
#undef CHAR
#endif

#include "RSCommon.h"


void updateSCell(Cell *cell, USER_OBJECT_ cellRef, MStyle *style);
void updateCell(Cell *cell, Sheet *sheet, MStyle *style);


Cell *RGnumeric_resolveCellReference(USER_OBJECT_ scell);
USER_OBJECT_ RGnumeric_createCellReference(Cell *cell, Sheet *sheet);

/* 
 Private!
 */

Rboolean RGnumeric_loadProfile(const char *name);

gboolean RGnumeric_internal_setStyle(MStyle *style, USER_OBJECT_ sstyle);
gboolean Internal_setStyleColors(MStyle *style, USER_OBJECT_ colors);
gboolean Internal_setPattern(MStyle *style, USER_OBJECT_ patternSlot);

gboolean Internal_setWrap(MStyle *style, USER_OBJECT_ wrapSlot);
gboolean Internal_setIndent(MStyle *style, USER_OBJECT_ indentSlot);

gboolean Internal_setAlign(MStyle *style, USER_OBJECT_ alignSlot);
gboolean Internal_setFormat(MStyle *style, USER_OBJECT_ formatSlot);
gboolean Internal_setFontStyles(MStyle *style, USER_OBJECT_ fontStyles);
gboolean Internal_setFont(MStyle *style, USER_OBJECT_ fontSlot);


Rboolean RGnumeric_internal_registerFunction(char *name, USER_OBJECT_ func, char *args, char *arg_names, char **help, char *categoryName);
#if 0
USER_OBJECT_ tryEval(USER_OBJECT_ e, int *ErrorOccurred);
#endif

GList * RGnumeric_getFunctionList();

typedef struct {
	FunctionDefinition *fndef;
	USER_OBJECT_ codeobj;
        char *argTypes;
} FuncData;

typedef FuncData RGnumeric_FuncData;


RGnumeric_FuncData *RGnumeric_unregisterFunctionDef(const gchar *name);
USER_OBJECT_ RGnumeric_getFunctionText(USER_OBJECT_ fun);
RGnumeric_FuncData *RGnumeric_getFunctionDef(const gchar *name, int *n);
Rboolean
RGnumeric_internal_registerFunction(char *name, USER_OBJECT_ func, char *args, char *arg_names,
				    char **help, char *categoryName);

USER_OBJECT_ RGnumeric_getFunctionObject(const char *def);
#endif
