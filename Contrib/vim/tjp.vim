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

syn keyword tjspkeyword account
syn keyword tjspkeyword accountid
syn keyword tjspkeyword accumulate
syn keyword tjspkeyword alap
syn keyword tjspkeyword all
syn keyword tjspkeyword allocate
syn keyword tjspkeyword alternative
syn keyword tjspkeyword asap
syn keyword tjspkeyword barlabels
syn keyword tjspkeyword booking
syn keyword tjspkeyword caption
syn keyword tjspkeyword celltext
syn keyword tjspkeyword cellurl
syn keyword tjspkeyword columns
syn keyword tjspkeyword complete
syn keyword tjspkeyword completed
syn keyword tjspkeyword completeddown
syn keyword tjspkeyword completedup
syn keyword tjspkeyword containstask
syn keyword tjspkeyword copyright
syn keyword tjspkeyword cost
syn keyword tjspkeyword credit
syn keyword tjspkeyword csvaccountreport
syn keyword tjspkeyword csvresourcereport
syn keyword tjspkeyword csvtaskreport
syn keyword tjspkeyword currency
syn keyword tjspkeyword currencydigits
syn keyword tjspkeyword currencyformat
syn keyword tjspkeyword d
syn keyword tjspkeyword daily
syn keyword tjspkeyword dailyworkinghours
syn keyword tjspkeyword day
syn keyword tjspkeyword days
syn keyword tjspkeyword db
syn keyword tjspkeyword depends
syn keyword tjspkeyword disabled
syn keyword tjspkeyword duration
syn keyword tjspkeyword efficiency
syn keyword tjspkeyword effort
syn keyword tjspkeyword empty
syn keyword tjspkeyword end
syn keyword tjspkeyword endbuffer
syn keyword tjspkeyword endbufferstart
syn keyword tjspkeyword endcredit
syn keyword tjspkeyword enddown
syn keyword tjspkeyword endsAfter
syn keyword tjspkeyword endsBefore
syn keyword tjspkeyword endup
syn keyword tjspkeyword export
syn keyword tjspkeyword extend
syn keyword tjspkeyword finished
syn keyword tjspkeyword flags
syn keyword tjspkeyword follows
syn keyword tjspkeyword fri
syn keyword tjspkeyword fullnamedown
syn keyword tjspkeyword fullnameup
syn keyword tjspkeyword h
syn keyword tjspkeyword headline
syn keyword tjspkeyword hideaccount
syn keyword tjspkeyword hideresource
syn keyword tjspkeyword hidetask
syn keyword tjspkeyword hierarchindex
syn keyword tjspkeyword hierarchno
syn keyword tjspkeyword hours
syn keyword tjspkeyword htmlaccountreport
syn keyword tjspkeyword htmlresourcereport
syn keyword tjspkeyword htmlstatusreport
syn keyword tjspkeyword htmltaskreport
syn keyword tjspkeyword htmlweeklycalendar
syn keyword tjspkeyword id
syn keyword tjspkeyword iddown
syn keyword tjspkeyword idup
syn keyword tjspkeyword include
syn keyword tjspkeyword index
syn keyword tjspkeyword indexdown
syn keyword tjspkeyword indexup
syn keyword tjspkeyword inherit
syn keyword tjspkeyword inprogress
syn keyword tjspkeyword inprogressearly
syn keyword tjspkeyword inprogresslate
syn keyword tjspkeyword isAccount
syn keyword tjspkeyword isAllocated
syn keyword tjspkeyword isAllocatedToProject
syn keyword tjspkeyword isChildOf
syn keyword tjspkeyword isLeaf
syn keyword tjspkeyword isMilestone
syn keyword tjspkeyword isParentOf
syn keyword tjspkeyword isResource
syn keyword tjspkeyword isTask
syn keyword tjspkeyword isTaskOfProject
syn keyword tjspkeyword isTaskStatus
syn keyword tjspkeyword isactualallocated
syn keyword tjspkeyword isplanallocated
syn keyword tjspkeyword issubtaskof
syn keyword tjspkeyword kotrusid
syn keyword tjspkeyword kotrusiddown
syn keyword tjspkeyword kotrusidup
syn keyword tjspkeyword kotrusmode
syn keyword tjspkeyword label
syn keyword tjspkeyword length
syn keyword tjspkeyword load
syn keyword tjspkeyword loadunit
syn keyword tjspkeyword longauto
syn keyword tjspkeyword m
syn keyword tjspkeyword macro
syn keyword tjspkeyword mandatory
syn keyword tjspkeyword maxeffort
syn keyword tjspkeyword maxeffortdown
syn keyword tjspkeyword maxeffortup
syn keyword tjspkeyword maxend
syn keyword tjspkeyword maxloaded
syn keyword tjspkeyword maxstart
syn keyword tjspkeyword milestone
syn keyword tjspkeyword min
syn keyword tjspkeyword mineffort
syn keyword tjspkeyword mineffortdown
syn keyword tjspkeyword mineffortup
syn keyword tjspkeyword minend
syn keyword tjspkeyword minloaded
syn keyword tjspkeyword minstart
syn keyword tjspkeyword minutes
syn keyword tjspkeyword mon
syn keyword tjspkeyword month
syn keyword tjspkeyword monthly
syn keyword tjspkeyword months
syn keyword tjspkeyword name
syn keyword tjspkeyword namedown
syn keyword tjspkeyword nameup
syn keyword tjspkeyword no
syn keyword tjspkeyword nokotrus
syn keyword tjspkeyword note
syn keyword tjspkeyword notimestamp
syn keyword tjspkeyword notstarted
syn keyword tjspkeyword now
syn keyword tjspkeyword numberformat
syn keyword tjspkeyword off
syn keyword tjspkeyword ontime
syn keyword tjspkeyword order
syn keyword tjspkeyword persistent
syn keyword tjspkeyword precedes
syn keyword tjspkeyword priority
syn keyword tjspkeyword prioritydown
syn keyword tjspkeyword priorityup
syn keyword tjspkeyword profit
syn keyword tjspkeyword project
syn keyword tjspkeyword projectid
syn keyword tjspkeyword quarter
syn keyword tjspkeyword quarterly
syn keyword tjspkeyword random
syn keyword tjspkeyword rate
syn keyword tjspkeyword ratedown
syn keyword tjspkeyword rateup
syn keyword tjspkeyword rawhead
syn keyword tjspkeyword rawstylesheet
syn keyword tjspkeyword rawtail
syn keyword tjspkeyword reference
syn keyword tjspkeyword resource
syn keyword tjspkeyword resourceid
syn keyword tjspkeyword resources
syn keyword tjspkeyword responsibilities
syn keyword tjspkeyword responsible
syn keyword tjspkeyword responsibledown
syn keyword tjspkeyword responsibleup
syn keyword tjspkeyword revenue
syn keyword tjspkeyword rollupaccount
syn keyword tjspkeyword rollupresource
syn keyword tjspkeyword rolluptask
syn keyword tjspkeyword sat
syn keyword tjspkeyword scenario
syn keyword tjspkeyword scenarios
syn keyword tjspkeyword schedule
syn keyword tjspkeyword scheduled
syn keyword tjspkeyword scheduling
syn keyword tjspkeyword select
syn keyword tjspkeyword separator
syn keyword tjspkeyword seqno
syn keyword tjspkeyword sequencedown
syn keyword tjspkeyword sequenceup
syn keyword tjspkeyword shift
syn keyword tjspkeyword shortauto
syn keyword tjspkeyword shorttimeformat
syn keyword tjspkeyword showprojectids
syn keyword tjspkeyword sortaccounts
syn keyword tjspkeyword sortresources
syn keyword tjspkeyword sorttasks
syn keyword tjspkeyword start
syn keyword tjspkeyword startbuffer
syn keyword tjspkeyword startbufferend
syn keyword tjspkeyword startcredit
syn keyword tjspkeyword startdown
syn keyword tjspkeyword startsAfter
syn keyword tjspkeyword startsBefore
syn keyword tjspkeyword startup
syn keyword tjspkeyword status
syn keyword tjspkeyword statusdown
syn keyword tjspkeyword statusnote
syn keyword tjspkeyword statusup
syn keyword tjspkeyword subtitle
syn keyword tjspkeyword subtitleurl
syn keyword tjspkeyword sun
syn keyword tjspkeyword supplement
syn keyword tjspkeyword table
syn keyword tjspkeyword task
syn keyword tjspkeyword taskattributes
syn keyword tjspkeyword taskid
syn keyword tjspkeyword taskprefix
syn keyword tjspkeyword taskroot
syn keyword tjspkeyword text
syn keyword tjspkeyword thu
syn keyword tjspkeyword timeformat
syn keyword tjspkeyword timezone
syn keyword tjspkeyword timingresolution
syn keyword tjspkeyword title
syn keyword tjspkeyword titleurl
syn keyword tjspkeyword total
syn keyword tjspkeyword tree
syn keyword tjspkeyword treeLevel
syn keyword tjspkeyword tue
syn keyword tjspkeyword undefined
syn keyword tjspkeyword url
syn keyword tjspkeyword vacation
syn keyword tjspkeyword w
syn keyword tjspkeyword wed
syn keyword tjspkeyword week
syn keyword tjspkeyword weekly
syn keyword tjspkeyword weeks
syn keyword tjspkeyword weekstartsmonday
syn keyword tjspkeyword weekstartssunday
syn keyword tjspkeyword workinghours
syn keyword tjspkeyword xml
syn keyword tjspkeyword xmlreport
syn keyword tjspkeyword y
syn keyword tjspkeyword year
syn keyword tjspkeyword yearly
syn keyword tjspkeyword yearlyworkingdays
syn keyword tjspkeyword years
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

