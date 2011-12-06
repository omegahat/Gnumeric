#
# Functions to insert an S object into a spreadsheet.
#

insertMatrix <-
#
# Writes the elements of the matrix into a gnumeric sheet
# in the range of cells identified by the `range' argument.
# headers
#
#
#
function(m, .sheet, start = c(1,1), end = NULL, headers=T, rowNames = T)
{
 if(headers)
   .sheet[start[1], start[2]+ as.integer(rowNames)] <- colnames(m)

 if(rowNames)
   .sheet[start[1] + as.integer(headers), start[2], byrow=F] <- rownames(m)

 for(i in 1:nrow(m)) {
   .sheet[start[1] + i-1 + as.integer(headers), start[2] + as.integer(rowNames)] <- m[i,]
 }

 invisible(m)
}

