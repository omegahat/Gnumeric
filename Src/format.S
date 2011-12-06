#
# This relates to formatting a cell
#
#
# alignment;
# font family, style and size; 
# background and foreground color;
# underline, strikethrough, sub and super-script;
# border
# protected
#
#
# This is likely to change quite significantly in implementation.
# We may use a "compiled" format for groups of cells rather than
# change the settings of each one individually.
# However, the effect is the same for the moment and we are experimenting
# with the interface before getting into more sophisticated optimization
# and internals.

formatCell <-
function(row=1, column=1, .sheet, ..., format, copyCell)
{
 if(length(list(...)) > 0)
  format <- createCellFormat(..., format)

  if(is.character(column))
    column <- sapply(column, gnumeric.columnIndex)

  row <- as.integer(range(row))
  column  <- as.integer(range(column))

  if(missing(copyCell))
    copyCell <- c(row[1], column[1])

 .Call("RGnumeric_setCellFormat", row, column, format, .sheet, copyCell)
}

getCellFormat <-
function(row=1, column=1, .sheet)
{
  if(is.character(column))
    column <- gnumeric.columnIndex(column)

 .Call("RGnumeric_getCellFormat", as.integer(c(row, column)), .sheet)
}

createCellFormat <-
function(..., format = NULL, verify = F)
{
 val <- list(...)
 if(!is.null(format)) {
   # merge in the new ones 
  idx <- match(names(format), names(val))
  if(any(is.na(idx))) {
    val[names(format)[is.na(idx)]] <- format[is.na(idx)]
  }
 } 

 if(verify) {
   # check the names and values are ok.
 }

 val
}

