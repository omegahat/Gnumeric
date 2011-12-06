library(RGnumeric)

gnumeric.lmOutput <-
function(sm, .sheet, r = 0, c = 0) {
 .sheet[1+r, 3 + c] <- "Result of regression"

# r$color$foreground$rgb <- as.integer(c(65535,0,0))
 .sheet[2+r , c]  <- "Residuals"

 .sheet[3+r,c] <- names(summary(sm$residuals))
 .sheet[4+r,c] <- summary(sm$residuals)


 .sheet[6+r,c]  <- "Coefficients"
# equivalent to 
#    .sheet[7+r,c] <- sm$coefficients
 insertMatrix(sm$coefficients, .sheet, c(7+r,1+c))

 .sheet[2 +r, 7+c] <- list("Multiple R-squared", sm$"r.squared")

 .sheet[3 +r, 7+c] <- list("Adjusted R-squared", sm$"adj.r.squared")

 .sheet[5+r,7+c] <- list("F-statistic", sm$fstatistic)
 .sheet[6+r,7+c] <- unlist(list("degrees of freedom", sm$df[3:2]))
}

gnumeric.registerFunction(
  function(cells, .sheet) {

   data <- getSheetRegion(cells$start, cells$end, .sheet)
   data <- as.data.frame(data)
   names(data) <- LETTERS[1:ncol(data)]
   sm <- lm(A ~ ., data = data) 

   w <- getGnumericWorkbook(.sheet)

   if(is.null(w[["lm-output"]])) {
    s <- newSheet("lm-output", .sheet, FALSE)
   } else {
    s <- w[["lm-output"]]
   }


   .sheet = s
# Output to the new sheet, or the existing sheet
# but offset into column 5
#   gnumeric.lmOutput(summary(sm), s)
    gnumeric.lmOutput(summary(sm), .sheet,  c=5)

   r <- .sheet[[1, 8]]
   r$foreground <- c(65535,0, 0)
   r$bg <- c(0, 0, 65535)
   r$fontStyle <- c(bold=F, italic=T, underline=T, strike=F)
   r$font <- list(name="Times", NULL)
   r$halign <- "center"

   plot(sm, ask = FALSE)
   1
  },
  "lm", "r", "data region", "linear model for data region")


gnumeric.registerFunction(
function(cells, .sheet)
{
   library(gtkDevice)
   if(dev.cur() == 1)
     gtk()
   data <- getSheetRegion(cells$start, cells$end, .sheet)
   data <- as.data.frame(data)
  
   plot(data[[1]], data[[2]])

   T
},
 "plot", "r","","")

gnumeric.registerFunction(function(){rnorm(1)}, "rnorm", "", "", "generate standard normal deviate")

print(objects())
print(names(gnumeric.getFunctions()))



