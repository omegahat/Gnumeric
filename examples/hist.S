library(RGnumeric)
gnumeric.registerFunction(function(r, .sheet) {
         val <- getSheetRegion(r[[1]], r[[2]], .sheet)
         hist(val[[1]])
         T}, "hist", "r", "range of cells containing values","Draws a histogram")

print(names(gnumeric.getFunctions()))
