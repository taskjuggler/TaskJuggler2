" Vim syntax file
" Language:		Taskjuggler
" Maintainer:	Peter Poeml <poeml@suse.de>
" Last Change:	$Id

setlocal softtabstop=2
setlocal cindent shiftwidth=2
setlocal tabstop=2
setlocal cinoptions=g0,t0,+0,(0,c0,C1
setlocal cinwords=task,resource,account,shift,htmltaskreport,htmlresourcereport,htmlaccountreport
setlocal cinkeys=0{,0},!^F,o,O
setlocal cindent
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
syn match	tjspinclude		"include.*$"
syn keyword tjspstruct	 	resource task macro account shift
" we could also highlight the tags... but it's against the rules
"syn match 	tjspstruct	 	"task\s*\S*"
"syn match 	tjspstruct	 	"macro\s*\S*"
syn keyword tjspspecial		project
syn match	tjspdelimiter 	contained "[();,~]"
syn match	tjspjump 		contained "!"
syn match	tjspbrace		"{}"
syn match	tjsparg			contained "\${.*}"
syn match	tjspoperator	contained "[=|&\*\+\<\>]"
syn match	tjspcomment		"#.*"
" TODO: Implement support for C-style comments
syn keyword tjspkeyword accumulate
syn keyword tjspkeyword actualduration
syn keyword tjspkeyword actualeffort
syn keyword tjspkeyword actualend
syn keyword tjspkeyword actuallength
syn keyword tjspkeyword actualstart
syn keyword tjspkeyword alap
syn keyword tjspkeyword allocate
syn keyword tjspkeyword alternative
syn keyword tjspkeyword asap
syn keyword tjspkeyword caption
syn keyword tjspkeyword columns
syn keyword tjspkeyword complete
syn keyword tjspkeyword copyright
syn keyword tjspkeyword cost
syn keyword tjspkeyword credit
syn keyword tjspkeyword currency
syn keyword tjspkeyword currencydigits
syn keyword tjspkeyword depends
syn keyword tjspkeyword duration
syn keyword tjspkeyword efficiency
syn keyword tjspkeyword effort
syn keyword tjspkeyword end
syn keyword tjspkeyword endcredit
syn keyword tjspkeyword enddown
syn keyword tjspkeyword endup
syn keyword tjspkeyword export
syn keyword tjspkeyword flags
syn keyword tjspkeyword fullnamedown
syn keyword tjspkeyword fullnameup
syn keyword tjspkeyword headline
syn keyword tjspkeyword hideaccount
syn keyword tjspkeyword hideplan
syn keyword tjspkeyword hideresource
syn keyword tjspkeyword hidetask
syn keyword tjspkeyword htmlaccountreport
syn keyword tjspkeyword htmlresourcereport
syn keyword tjspkeyword htmltaskreport
syn keyword tjspkeyword iddown
syn keyword tjspkeyword idup
syn keyword tjspkeyword indexdown
syn keyword tjspkeyword indexup
syn keyword tjspkeyword kotrusid
syn keyword tjspkeyword kotrusiddown
syn keyword tjspkeyword kotrusidup
syn keyword tjspkeyword kotrusmode
syn keyword tjspkeyword length
syn keyword tjspkeyword load
syn keyword tjspkeyword maxeffort
syn keyword tjspkeyword maxeffortdown
syn keyword tjspkeyword maxeffortup
syn keyword tjspkeyword maxend
syn keyword tjspkeyword maxstart
syn keyword tjspkeyword milestone
syn keyword tjspkeyword min
syn keyword tjspkeyword mineffort
syn keyword tjspkeyword mineffortdown
syn keyword tjspkeyword mineffortup
syn keyword tjspkeyword minend
syn keyword tjspkeyword minstart
syn keyword tjspkeyword namedown
syn keyword tjspkeyword nameup
syn keyword tjspkeyword nokotrus
syn keyword tjspkeyword note
syn keyword tjspkeyword now
syn keyword tjspkeyword off
syn keyword tjspkeyword persistent
syn keyword tjspkeyword preceeds
syn keyword tjspkeyword priority
syn keyword tjspkeyword prioritydown
syn keyword tjspkeyword priorityup
syn keyword tjspkeyword projectid
syn keyword tjspkeyword rate
syn keyword tjspkeyword ratedown
syn keyword tjspkeyword rateup
syn keyword tjspkeyword responsible
syn keyword tjspkeyword responsibledown
syn keyword tjspkeyword responsibleup
syn keyword tjspkeyword revenue
syn keyword tjspkeyword rollupaccount
syn keyword tjspkeyword rollupresource
syn keyword tjspkeyword rolluptask
syn keyword tjspkeyword sat
syn keyword tjspkeyword scheduling
syn keyword tjspkeyword showactual
syn keyword tjspkeyword showprojectids
syn keyword tjspkeyword sortaccounts
syn keyword tjspkeyword sortresources
syn keyword tjspkeyword sorttasks
syn keyword tjspkeyword start
syn keyword tjspkeyword startcredit
syn keyword tjspkeyword startdown
syn keyword tjspkeyword startup
syn keyword tjspkeyword supplement
syn keyword tjspkeyword timingresolution
syn keyword tjspkeyword tree
syn keyword tjspkeyword vacation
syn keyword tjspkeyword wed
syn keyword tjspkeyword workinghours
syn keyword tjspkeyword xml
syn keyword tjspkeyword xmltaskreport

"syn keyword tjspmilestone 	contained	  milestone
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
