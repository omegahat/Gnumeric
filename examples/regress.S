gnumeric.registerFunction(function(cells, headers=T,.sheet) {
                             # get the data in the 
cat("in regress\n")
                            data <- getSheetRegion(cells[[1]]+c(1,0), cells[[2]],.sheet)
                            data <- as.data.frame(data)
                            cols <- seq(cells[[1]][2], cells[[2]][2])
                            headers <- .sheet[cells[[1]][1], cols]
                            names(data) <- as.character(unlist(headers))
                            fit <- lm( ,data=data)  
                            # create a new sheet
                            out <- newSheet("regress", .sheet)
                            # 
                          }, "regress","rb","data,variableNames",
                          "regress first column on subsequent columns and output in new sheet")