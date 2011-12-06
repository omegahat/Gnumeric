#ifndef R_GNUMERIC_CONVERT_H
#define R_GNUMERIC_CONVERT_H

#include "RGnumeric.h"

USER_OBJECT_ convertFromGnumericArg(Value *value, FunctionEvalInfo *ei, int which);
USER_OBJECT_ convertGnumericCellRange(ValueRange *value);
USER_OBJECT_ convertGnumericArray(ValueArray *value);

Value *convertToGnumericValue(USER_OBJECT_ value);

#endif
