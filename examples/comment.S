library(RGnumeric)

gnumeric.registerFunction(
  function(r=1, c=1, .sheet) {
    cell <- .sheet[[r, c]]
    k <- cell$comment
    print(k)

    k["comment"]

  }, "getComment", "ff", "row,column", "testing getting a comment")


gnumeric.registerFunction(
  function(msg, author, r=1, c=1, .sheet) {
    cell <- .sheet[[r, c]]
    cell$comment <- c(msg, author)
    k <- cell$comment
    print(k)

    k["comment"]
  }, "setComment", "ssff", "row,column", "testing getting a comment")


