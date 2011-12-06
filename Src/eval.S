parseEval <-
#
# Parse and evaluate an S expression given as a string.
#
function(cmd)
{
  eval(parse(text = cmd))
}


getFunctionString <-
#
# Returns the definition of the specified function
# as a string, having first removed the environment 
# so that it isn't displayed in the text representation.
#
function(fun)
{
  environment(fun) <- globalenv()
  zz <- textConnection(".gnumericFunDump", "w")
  sink(zz)
  print(fun)
  sink()
  close(zz)

  paste(c("", .gnumericFunDump,""), collapse="\n")
}


dputEnvironmentObject <-
#
# gets the specified object in the given environment 
# and returns a string containing it in dput() format
# and a second string giving its type.
#
function(name, env)
{
   zz <- textConnection(".dputEnvironmentObject", "w")
   val <- get(name, envir=env)
   dput(val, zz)
   close(zz)
   c(paste(c("", .dputEnvironmentObject, ""), collapse="\n"), 
     typeof(val))
}
