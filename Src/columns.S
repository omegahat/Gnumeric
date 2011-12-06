gnumeric.columnIndex <- 
# This computes the index of a column
# given by name
# A..Z, AA...AZ, BA...BZ, ... , ZA..ZZ
# AAA...AAZ, etc.

function(id)
{
 els <- strsplit(id, "")[[1]]
 x <- 26^((length(els)-1):0)
 sum(match(els, LETTERS)*x) 
}
