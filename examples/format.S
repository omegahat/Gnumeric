# Not necessary since this is automatically loaded
# by the plugin before any initialization scripts are 
# read, (except the one R_PROFILE)!

library(RGnumeric)

gnumeric.registerFunction(
      function(r, c, .sheet) {
        cell <- .sheet[[r,c]]
cat("Cell attributes\n")
cat("font\n")
        print(cell$font)
cat("fontStyle\n")
        print(cell$fontStyle)
cat("background\n")
        print(cell$background)
cat("foreground\n")
        print(cell$foreground)
cat("wrap\n")
        print(cell$wrap)
cat("border\n")
        print(cell$border)
cat("format\n")
        print(cell$format)
cat("indent\n")
        print(cell$indent)
cat("pattern\n")
        print(cell$pattern)
cat("value\n")
        print(cell$value)
cat("pos\n")
        print(cell$pos)
cat("halign\n")
        print(cell$halign)
cat("valign\n")
        print(cell$valign)
cat("comment\n")
        print(cell$comment)
cat("author\n")
        print(cell$author)

        T
      }, "getFormat", "ff", "row,column", "test getting format information")

