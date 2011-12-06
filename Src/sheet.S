isEmpty.GnumericSheetRef <-
#
# Determines if a range or single cell
# consists of empty cells, i.e. cells with 
# no contents.

function(sheet, i, j)
{
  if(is.character(j))
    j <- gnumeric.columnIndex(j)

  i <- range(as.integer(i))
  j <- range(as.integer(j))

 .Call("RGnumeric_isCellEmpty", i, j, sheet)
}

redraw.GnumericSheetRef <-
function(cells, headers=FALSE, recalc=FALSE)
{
 if(missing(cells)) {
  return(.Call("RGnumeric_redrawSheet"))
 }

 # Now if we have a cell region, range or individual cell
 
}

setDirty.GnumericSheetRef  <-
function(x)
{

}

"[.GnumericSheetRef" <-
#
# Get elements from a sheet, either rows or columns
# or an individual cell
#
#  sheet[1,]
#  sheet[,1]
#  sheet[1,2]
#  sheet[c(1,3,8),]
#  sheet[,c(1,3,8)]
#  sheet[c(1,3,8),c(1,2)]
#
function(x,...)
{
 args <- list(...)
  # Have to handle column names specially, but later.

 if(length(args) > 1 && is.character(args[[2]]))
   args[[2]] <- sapply(args[[2]], gnumeric.columnIndex)
  
 
 args <- lapply(args, as.integer)

 .Call("RGnumeric_getSheetCell", x, args)
}

"[[.GnumericSheetRef" <-
function(x, i, j)
{
 if(is.character(j))
   j <- gnumeric.columnIndex(j)

 .Call("RGnumeric_getCellReference", x, as.integer(i),as.integer(j))
}

"[<-.GnumericSheetRef" <-
#
# We may allow formatting options to be added here.
#
function(x, r=integer(0), c=integer(0), value,  byrow = T)
{
# args <- list(...)
# args <- lapply(args, as.integer)

 isList <- is.list(value)

 if(is.character(c))
   c <- gnumeric.columnIndex(c)

 if(is.matrix(value)) {
  row <- ifelse(length(r) == 0, 1, r[1])
  col <- ifelse(length(c) == 0, 1, c[1])
  insertMatrix(value, .sheet = x, start=c(row,col), headers=T, rowNames=T)
  return(x)
 } 


 if(length(r) == 1 && length(c) == 1) {
  if(length(value) == 1) {
    .Call("RGnumeric_setCellValue", x, as.integer(r), as.integer(c), value)
  } else {
     for(k in 1:length(value)) {
      if(byrow)
       .Call("RGnumeric_setCellValue", x, as.integer(r), as.integer(c+k-1), 
                                         ifelse(isList, value[[k]], value[k]))
      else
       .Call("RGnumeric_setCellValue", x, as.integer(r + k -1), as.integer(c), 
                                         ifelse(isList, value[[k]], value[k]))
     }
  }
  return(x)
 }


 if(length(c) == 0) {
   if(length(r) == 1) {
     for(k in 1:length(value)) {
       .Call("RGnumeric_setCellValue", x, as.integer(r), k, 
                                         ifelse(isList, value[[k]], value[k]))
      }
   } else {
     for(k in 1:min(length(value), length(r))) {
       .Call("RGnumeric_setCellValue", x, as.integer(r[k]), k, 
                                         ifelse(isList, value[[k]], value[k]))
      }
   }

  return(x)  
 }


 if(length(r) == 0) {
   if(length(c) == 1) {
     for(k in 1:length(value)) {
       .Call("RGnumeric_setCellValue", x, k, as.integer(c), 
                                         ifelse(isList, value[[k]], value[k]))
      }
   } else {
     for(k in 1:min(length(value), length(c))) {
       .Call("RGnumeric_setCellValue", x, k, as.integer(c[k]),  
                                         ifelse(isList, value[[k]], value[k]))
      }
   }

  return(x)  
 }




# .Call("RGnumeric_setCellValue", x, as.integer(i), as.integer(j), value)
 x
}


dim.GnumericSheetRef <-
function(x, collapse=T)
{
 collapse <- as.logical(collapse)
 d <- .Call("RGnumeric_getSheetDim",x, collapse)

 if(collapse == FALSE) {
   names(d) <- c("start", "end")
   names(d[[1]]) <- c("row", "col")
   names(d[[2]]) <- c("row", "col")
 }
 
 d   
}

sheetExtent <-
function(.sheet)
{
 dim.GnumericSheetRef(.sheet, collapse=F)
}

newSheet <-
function(name, .sheet, checkName = T)
{
 if(as.logical(checkName)) {
   name <- uniqueSheetName(name, getGnumericWorkbook(.sheet))
 }
 .Call("RGnumeric_newSheet", as.character(name), .sheet)
}

clearSheet <-
function(rows=NULL, cols = NULL, how=-1, .sheet)
{
 if(missing(rows) && missing(cols))
   .Call("RGnumeric_clearSheet", .sheet)
 else {
  d <- dim.GnumericSheetRef(.sheet)
  if(missing(cols)) {
    cols <- c(1, d[2])
  }
  if(missing(rows)) {
    rows <- c(1, d[1])
  }

  stop ("Need to fix the C code to get WorkbookControl object")
  .Call("RGnumeric_clearRegion", as.integer(rows), as.integer(cols), .sheet)
 }
}

sheetDelete <-
function(rows, columns)
{
 stop("Not implemented yet")
}

renameSheet <-
function(name, .sheet)
{
 .Call("RGnumeric_renameSheet", .sheet, as.character(name))
}


getSheetRegion <-
function(start, end, .sheet)
{
  numCols <- end[2] - start[2] + 1
  numRows <- end[1] - start[1] + 1

  data <- vector("list", numCols)
  ctr <- 1
  for(i in seq(start[2], end[2])) {
    data[[ctr]] <- getSheetColumn(i, seq(start[1], end[1]), .sheet)
    ctr <- ctr + 1
  }


  data
}

getSheetColumn <-
function(col, rows, .sheet)
{
 vals  <- .sheet[as.integer(rows), as.integer(col)]
 vals <- unlist(vals)
 vals
}

removeSheet <-
function(.sheet)
{
 if(!inherits(.sheet, "GnumericSheetRef"))
  stop("removeSheet must be called with a GnumericSheetRef object")

 .Call("RGnumeric_removeSheet", .sheet)
}




getGnumericSheet <-
function(x)
{
 UseMethod("getGnumericSheet")
}

getGnumericSheet.default <-
function(x)
{
 stop("Don't be silly.")
}

getGnumericSheet.GnumericCellRef <-
function(x)
{
  x[["sheet"]]
}

getSheetName <-
function(.sheet)
{
 .Call("RGnumeric_getSheetName", .sheet)
}

setVisibleCell <-
function(i, j, .sheet)
{
 if(!inherits(i, "GnumericCellRef")) {
   .sheet <- i[[2]]
   j <- j$pos[2]
   i <- i$pos[1]
 }

 .Call("RGnumeric_setVisibleCell", as.integer(i), as.integer(j), .sheet)
}


setCursor <-
function(i, j, .sheet)
{
 if(inherits(i, "GnumericCellRef")) {
   .sheet <- i[[2]]
   j <- j$pos[2]
   i <- i$pos[1]
 }

 if(is.character(j))
   j  <- gnumeric.columnIndex(j)

 .Call("RGnumeric_setCursor",  as.integer(i), as.integer(j), .sheet)
}

