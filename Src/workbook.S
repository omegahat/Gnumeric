newGnumericWorkbook <-
#
# Create an entirely new and empty workbook.
# See also newSheet.
function(file=NULL)
{
 book <- .Call("RGnumeric_newWorkbook", as.character(file))

 book
}

"[[.GnumericWorkbookRef" <-
function(x, index)
{
 .Call("RGnumeric_sheetInWorkbook",x, ifelse(is.numeric(index), as.integer(index), as.character(index)))
}

"[.GnumericWorkbookRef" <-
function(x, ...)
{
 args <- list(...)
 vals <- vector("list", length(args))
 ctr <- 1
 for(i in args) {
   vals[[ctr]] <- .Call("RGnumeric_sheetInWorkbook", x, ifelse(is.numeric(i), as.integer(i), as.character(i)))
   ctr <- ctr+ 1
 } 

 if(length(vals) == 1)
   return(vals[[1]])

 vals
}

getGnumericWorkbook <-
function(.sheet)
{
  .Call("RGnumeric_getWorkbookFromSheet", .sheet)
}

length.GnumericWorkbookRef <-
function(x)
{
  .Call("RGnumeric_getNumSheetsInWorkbook", x)
}

names.GnumericWorkbookRef <-
function(x)
{
 .Call("RGnumeric_getSheetNames", x)
}

uniqueSheetName <-
function(name, book, suffix=T, counter=T)
{
 if(inherits(book, "GnumericSheetRef"))
   book <- getGnumericWorkbook(book)

 .Call("RGnumeric_workbookFreeSheetName", name, book, as.logical(suffix), as.logical(counter))
}


recalcBook <-
function(book, all = T)
{
 if(inherits(book, "GnumericSheetRef")) {
    book <- getGnumericWorkbook(book)
 }

 .Call("RGnumeric_recalcWorkbook", book, as.logical(all))
}

