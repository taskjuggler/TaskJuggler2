" Vim syntax file
" Language:		Taskjuggler
" Maintainer:	Peter Poeml <poeml@suse.de>
" Last Change:	$Id: tjp.vim 1335 2006-09-24 13:49:05Z cs $

setlocal softtabstop=2
setlocal cindent shiftwidth=2
setlocal tabstop=2
setlocal cinoptions=g0,t0,+0,(0,c0,C1
setlocal cinwords=task,resource,account,shift,htmltaskreport,htmlresourcereport,htmlaccountreport
setlocal cinkeys=0{,0},!^F,o,O
setlocal cindent

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


" define the tjp syntax
syn match	tjpinclude		"include.*$"
syn keyword tjpstruct	 	resource task macro account shift
" we could also highlight the tags... but it's against the rules
"syn match 	tjpstruct	 	"task\s*\S*"
"syn match 	tjpstruct	 	"macro\s*\S*"
syn keyword tjpspecial		project
syn match	tjpdelimiter 	contained "[();,~]"
syn match	tjpjump 		contained "!"
syn match	tjpbrace		"{}"
syn match	tjparg			contained "\${.*}"
syn match	tjpoperator	contained "[=|&\*\+\<\>]"
syn match	tjpcomment		"#.*"
" TODO: Implement support for C-style comments

