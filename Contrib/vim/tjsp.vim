" Vim syntax file
" Language:		Taskjuggler
" Maintainer:	Peter Poeml <poeml@suse.de>
" Last Change:	2002 Jan 23

setlocal softtabstop=2
setlocal cindent shiftwidth=2
setlocal tabstop=2
setlocal cinoptions=g0t0+0(0c0,C1
setlocal cinwords="task,resource,account,htmltaskreport,htmlresourcereport,htmlaccountreport"
setlocal cinkeys="0{,0},!^F,o,O"
setlocal makeprg=./tj

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn case ignore

if version >= 600 
  sy  region  taskfold  start="{"  end="}" transparent fold contains=ALL
  sy  region  macrofold start="\[" end="\]" transparent fold contains=ALL
  syn sync fromstart
  set foldmethod=syntax
endif


" define the tjsp syntax
syn match	tjspinclude		"^include.*$"
syn keyword tjspstruct	 	resource task macro account
" we could also highlight the tags... but it's against the rules
"syn match 	tjspstruct	 	"task\s*\S*"
"syn match 	tjspstruct	 	"macro\s*\S*"
syn keyword tjspspecial		project projectid copyright 
syn match	tjspdelimiter 	contained "[();,~]"
syn match	tjspjump 		contained "!"
syn match	tjspbrace		"{}"
syn match	tjsparg			contained "\${.*}"
syn match	tjspoperator	contained "[=|&\*\+\<\>]"
syn match	tjspcomment		"#.*"
"syn match	tjsparch		contained display "\<ppc\|axp\|i386\|s\/390\|\<\(ip\|i\|p\|z\)\(-\)\{0,1}.eries\|alpha\|intel\|sparc\|amd\|apple\>" 
syn keyword	tjspkeyword		vacation effort maxeffort flags rate currency allocate cost
syn keyword	tjspkeyword		depends preceeds proceeds length note load startcredit priority
syn keyword	tjspkeyword		rolluptask sorttasks accumulate
syn keyword	tjspkeyword		no name start end minstart maxstart
syn keyword	tjspkeyword		columns headline caption
syn keyword	tjspkeyword		hidetask hideresource
syn keyword	tjspkeyword		kotrusmode 
syn keyword	tjspkeyword		daily weekly monthly
syn keyword	tjspkeyword		htmlresourcereport htmltaskreport htmlaccountreport
syn keyword tjspmilestone 	contained	  milestone
syn region  tjspstring	start=+"+ skip=+\\"+ end=+"+ contains=tjsparch 
syn region  tjspstring	start=+`+ skip=+\\'+ end=+'+ contains=tjsparch 
syn region  tjspstring	start=+`+ skip=+\\'+ end=+`+ contains=tjsparch 

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_tjsp_syntax_inits")
  if version < 508
    let did_tjsp_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink tjspdelimiter 	Delimiter
  "HiLink tjspoperator	Operator
  HiLink tjspoperator	Delimiter
  HiLink tjspcomment	Comment
  HiLink tjsparch		Function
  "HiLink tjspnumber	Number
  "HiLink tjsptimes		Constant
  HiLink tjspkeyword	Keyword
  HiLink tjspspecial	Special
  HiLink tjsparg		Special
  HiLink tjspstring		String
  HiLink tjspinclude	Include
  HiLink tjspstruct		Structure
  HiLink tjspmilestone 	Error

  "HiLink tjspbrace		Operator
  "HiLink taskfold		Operator

  HiLink tjspjump		Include

  delcommand HiLink
endif

let b:current_syntax = "tjsp"

" vim: ts=4
