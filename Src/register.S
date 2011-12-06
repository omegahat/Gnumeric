gnumeric.registerFunction <-
#
# If the user gives a function by name (character string),
# we fetch that and also use that name as the Gnumeric name
# if no value for the `name' argument is specified.
#
# If the user specifies a function as the value of a variable (rather than
# the result of an expression or an anymous function definition),
# we use that variable name if no value for `name' is provided.
#
#
# When we move to having XML objects/files for R help
# we will be able to grab the description section of the
# help easily and use this one-liner for the help argument
# if the user does not specify a value for help.

function(func, name, argTypes = "", 
           argNames = paste(names(formals(func)),sep=","), help="", category="R")
{
  if(missing(name) && is.symbol(substitute(func))) {
     name <- substitute(func)
  }

  if(is.character(func)) {
    if(missing(name))
      name <- func
    if(exists(func))
      func <- get(func)
    else
      func <- eval(parse(text = func))
  }

 invisible(.Call("RGnumeric_registerFunction", as.character(name), func, 
                                     as.character(argTypes), as.character(argNames),
                                     as.character(help), as.character(category)))
}


gnumeric.getFunctions <-
#
# Traverses the list of registered functions (funclist)
# and returns a description of each one.
#
function(describe=F)
{
 .Call("RGnumeric_getRegisteredFunctions", as.logical(describe)) 
}

gnumeric.hasSheetArgument <-
function(func)
{
 return(!is.na(match(c(".sheet", ".cell"), names(formals(func)))))
}
