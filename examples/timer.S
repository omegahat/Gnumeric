#
# In cell, e.g., C1, call this function as
#
#  =setTimer(500, 1, 1)
#
# And in A2, set the formula to be
#
#  =A1 + 10
#
# Then this will update the sheet every half second with a new random normal
# and recompute all the other cells.
#

setTimer <-
function(interval,r = 1, c = 1, .sheet) {
 library(RGtk)

 f <-
   function() {
     .sheet[r, c] <- rnorm(1, mean = 10)
     recalcBook(getGnumericWorkbook(.sheet))
     return(FALSE)
   }
 
 gtkAddTimeout(interval, f)
}

gnumeric.registerFunction(
      setTimer, "setTimer", "fff", "garbage", "Set a timer to fire each interval")
      
                           
