#ifndef R_GNUMERIC_SHEET_H
#define R_GNUMERIC_SHEET_H

#include "RGnumeric.h"

USER_OBJECT_ RGnumeric_sheetReference(Sheet *);
Sheet * RGnumeric_resolveSheetReference(USER_OBJECT_ sref);

#endif

