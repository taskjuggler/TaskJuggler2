syn region  tjpstring	start=+"+ skip=+\\"+ end=+"+ contains=tjparch 
syn region  tjpstring	start=+`+ skip=+\\'+ end=+'+ contains=tjparch 
syn region  tjpstring	start=+`+ skip=+\\'+ end=+`+ contains=tjparch 

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_tjp_syntax_inits")
  if version < 508
    let did_tjp_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink tjpdelimiter 	Delimiter
  "HiLink tjpoperator	Operator
  HiLink tjpoperator	Delimiter
  HiLink tjpcomment	Comment
  HiLink tjparch		Function
  "HiLink tjpnumber	Number
  "HiLink tjptimes		Constant
  HiLink tjpkeyword	Keyword
  HiLink tjpspecial	Special
  HiLink tjparg		Special
  HiLink tjpstring		String
  HiLink tjpinclude	Include
  HiLink tjpstruct		Structure
  HiLink tjpmilestone 	Error

  "HiLink tjpbrace		Operator
  "HiLink taskfold		Operator

  HiLink tjpjump		Include

  delcommand HiLink
endif

let b:current_syntax = "tjp"

" vim: ts=4


