gnumeric.HAlign <- c(general=1, left=2, right=4, center=8,
                     fill=16, justify=32, 
                     "center across selection"=64)

gnumeric.VAlign <- c(top=1, bottom=2, center=4, justify=8)

gnumeric.getAlignName <-
function(val, horiz = T)
{
  if(as.logical(horiz))
     v <- gnumeric.HAlign
  else
     v <- gnumeric.VAlign

  names(v)[val ==  v]
}



gnumeric.getAlignValue <- 
function(val, horiz = T)
{
  if(as.logical(horiz))
     v <- gnumeric.HAlign
  else
     v <- gnumeric.VAlign

 if(is.integer(val)) {
    # check it is in the appropriate list
  if(!any( val == v))
    stop("Invalid alignment value")

  return(val)
 }

 idx <- pmatch(as.character(val), names(v))
 if(is.na(idx))
    stop("Invalid alignment name")

 as.integer(v[idx])
}
