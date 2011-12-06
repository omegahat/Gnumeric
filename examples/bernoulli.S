library(RGnumeric)

gnumeric.registerFunction(function(p){rbinom(1,1, p)},
                        "rbernoulli", 
                        "f" , "probability",
                        "return a value sampled from a Bernoulli random variable.")

gnumeric.registerFunction(function(lambda) {rpois(1,lambda)},
                        "rpois", 
                        "f" , "mean",
                        "return a value sampled from a Poisson random variable.")


gnumeric.registerFunction(function(cmd) {eval(parse(text = cmd))},
                        "reval", 
                        "s" , "command",
                        "evaluate an R expression.")


gnumeric.registerFunction(function(m) {print(dim(m)); TRUE}, 
                          "arrayRef",
                          "a", "array", "check array")

gnumeric.registerFunction(function(x) {print(x); TRUE},
                          "rangeRef", "r", "range","shows the range that was specified.")

# load
gnumeric.registerFunction(load, "load", "s", "file", "loads a previously saved R data object")

# library
gnumeric.registerFunction(library, "library", "s", "package", "attaches the specified library")


gnumeric.registerFunction(function(def, name, args, argNames, help){
                            if(exists(def))
                              func <- get(def)
                            else
                              func <- eval(parse(text=def))

                            gnumeric.registerFunction(func, name, args, argNames, help)
                            return(name)
                          },
                           "define", "sssss", "definition,name,argumentTypes,argNames,help",
                           "define a gnumeric extension function in R")


gnumeric.registerFunction(function(s, .sheet=NULL){print(.sheet)},
                           "testSheet", "s", "garbage", "testing the .sheet argument")

gnumeric.registerFunction(function(s, .sheet=NULL){
                                    x <- .sheet[1,2]
                                    print(x)
                                    print(dim.GnumericSheetRef(.sheet))
                                    .sheet[3,2] <- "A test string"
                                    .sheet[4,2] <- paste("A second string", s)
                                    .sheet[5,2] <- 1.4
                                    .sheet[6,2] <- as.integer(1)
                                    .sheet[7,2] <- TRUE
                                    .sheet[1,4] <- "Increasing the extent"
                                    print(dim.GnumericSheetRef(.sheet))
                                    TRUE
                                   },
                           "testCell", "f", "garbage", "test of cell access")



gnumeric.registerFunction(function(name, .sheet=NULL){newSheet(name, .sheet)},
                           "newSheet", "s", "name of sheet", "testing the newSheet function")

gnumeric.registerFunction(function(name=NULL){newGnumericWorkbook(name)},
                           "newBook", "|s", "name of file to read into new book", 
                           "testing the newGnumericWorkbook function")


# Retrieve information about the registered functions.
#print(length(gnumeric.getFunctions()))
print(names(gnumeric.getFunctions()))

print(search())
print(objects(2, all=TRUE))
# print(objects("pkg:RGnumeric"))


