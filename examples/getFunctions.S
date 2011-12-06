library(RGnumeric)

gtmp <- dirname(Sys.getenv("R_GNUMERIC_PROFILE"))
source(file.path(gtmp, "ExampleProfile"))
gnumeric.registerFunction(function(.sheet){
                           Names <- names(gnumeric.getFunctions())
                           .sheet[,2] <- Names
                           length(Names)
                          }, "showFunctions", "", "","")