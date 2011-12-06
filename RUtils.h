#ifndef R_GNUMERIC_UTILS_H
#define R_GNUMERIC_UTILS_H

#include "RGnumeric.h"

USER_OBJECT_ RGnumeric_createInternalReference(void *ptr, const char *className);
void * RGnumeric_resolveInternalReference(USER_OBJECT_, const char *checkClass);
Rboolean IsInstanceOf(USER_OBJECT_ sobj, const char *className);
void Internal_setNames(const char ** const names, int num, USER_OBJECT_ ans);
USER_OBJECT_ RGnumeric_getCellInfo(const EvalPos * const pos);
#endif
