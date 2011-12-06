"$.GnumericCellRef" <-
function(x, name)
{
# foreground, background, font, fontStyle, wrap
# indent, border 
# pattern

 attributeNames <-  c(font=1, 
                      fontStyle=2,
                      background=3, bg=3,
                      foreground=4, fg=4,
                      wrap=5, 
                      border=6,
                      format=7,
                      indent=8,
                      pattern=9)

 mode(attributeNames) <- "integer"

 which <- match(name, names(attributeNames))
 if(!is.na(which))
  return(.Call("RGnumeric_getCellFormatSetting", x, which))

 if(name == "value") {
   l <- x$pos
   return(getGnumericSheet(x)[l[1], l[2]][[1]])
 } else if(name == "pos") {
   z <- .Call("RGnumeric_getCellPosition", x) 
   z + 1
 } else if(name == "halign") {
   x <- .Call("RGnumeric_getCellAlign", x, T)
   names(x) <- gnumeric.getAlignName(x)
   x
 } else if(name == "valign") {
   x <- .Call("RGnumeric_getCellAlign", x, F)
   names(x) <- gnumeric.getAlignName(x)
   x
 } else if(name == "author")
   .Call("RGnumeric_getCellComment", x)$author
 else if(name == "comment")
   .Call("RGnumeric_getCellComment", x)
 else
   return(NULL)
}

"$<-.GnumericCellRef" <-
function(x, name, value)
{
 # indent, border, pattern

 if(name == "value") {
   l <- x$pos
   sheet <- getGnumericSheet(x)
   sheet[l[1], l[2]] <- value
  } else if(name == "halign") {
   .Call("RGnumeric_setCellAlign", x, gnumeric.getAlignValue(value), T)
 } else if(name == "valign") {
   .Call("RGnumeric_setCellAlign", x, gnumeric.getAlignValue(value, F), F)
 } else if(name == "foreground" || name == "fg") {
   .Call("RGnumeric_setCellForeground", x, as.integer(value), T)
 } else if(name == "background" || name == "bg") {
   .Call("RGnumeric_setCellForeground", x, as.integer(value), F)
 } else if(name == "font") {
   .Call("RGnumeric_setCellFont", x, value, NULL)
 } else if(name == "fontStyle") {
   .Call("RGnumeric_setCellFont", x, NULL, value)
 } else if(name == "wrap") {
   .Call("RGnumeric_setCellWrap", x, as.logical(value))
 } else if(name == "format") {
   .Call("RGnumeric_setCellFormat", x, as.character(value), x[[2]])
 } else if(name == "indent") {
   .Call("RGnumeric_setCellIndent", x, value)
 } else if(name == "pattern") {
   .Call("RGnumeric_setCellPattern", x, value)
 } else if(name == "comment") {
    setCellComment(x, value)
 } else if(name == "author") {
    setCellComment(x, author=value)
 }

 x
}

getCellComment <-
function(i, j, .sheet)
{
 if(!is(i, "GnumericCellRef"))
   i <- .sheet[[i, j]]
   
 .Call("RGnumeric_getCellComment", i)
}

setCellComment <-
function(x, value = NULL, author)
{
  if(missing(value) && missing(author))
    stop("Must specify the comment value or author")

  if(!missing(value))
    value <- as.character(value)

  if(missing(author))
    author <- ifelse(length(value) > 1, value[2], NULL)

  .Call("RGnumeric_setCellComment", x, as.character(value), as.character(author))
}
