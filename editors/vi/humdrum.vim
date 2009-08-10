" Vim 5.6 syntax file
" Language:	 Humdrum file
" Programmer:	 Craig Stuart Sapp <craig@ccrma.stanford.edu>
" Web Page:      http://www.ccarh.org/software/humdrum/vi/syntax
" Creation Date: Fri Dec  8 11:41:45 PST 2000
" Last Change:	 Fri Dec  8 13:57:53 PST 2000
"

" Remove any old syntax stuff hanging around
syntax clear


syntax match Data           "^[^!\*]"            contains=BadTabbing,Chord
syntax match Chord          "[^\t][^\t]* [^\t]*" contains=ExtraSpace
syntax match ExtraSpace     "^ "
syntax match ExtraSpace     " $"
syntax match ExtraSpace     "  *\t"
syntax match ExtraSpace     "\t  *"
syntax match ExtraSpace     "   *"
syntax match BadTabbing     "\t\t\t*" 
syntax match BadTabbing     "^\t"
syntax match BadTabbing     "\t$"
syntax match GlobalComment  "^!![^!].*$"
syntax match BibRecord      "^!!![^ ].*:.*$"
syntax match Interpretation "^\*.*$"             contains=BadTabbing,Exclusive
syntax match Measure        "^=[^\t]*[\t]?"      contains=BadTabbing
syntax match Measure        "=[^\t]*[\t]"        contains=BadTabbing
syntax match Measure        "=[^\t]*$"           contains=BadTabbing
syntax match LocalComment   "^![^!].*$"          contains=BadTabbing
syntax match Exclusive      "\*\*[^\t]*"  


if !exists("did_humdrum_syntax_inits")
  let did_humdrum_syntax_inits = 1
  " The default methods for highlighting.  Can be overridden later
  hi darkgreen ctermfg=darkgreen   guifg=darkgreen
  hi darkblue  ctermfg=darkblue    guifg=darkblue
  hi red       ctermfg=red         guifg=red
  hi yellow    ctermfg=yellow      guifg=yellow
  hi white    ctermfg=white      guifg=white
  hi cyan      ctermfg=cyan        guifg=cyan
  hi magenta   ctermfg=magenta     guifg=magenta
  hi measure   ctermfg=darkgrey ctermbg=lightgrey guifg=lightgrey guibg=black
  hi space     ctermbg=darkblue    guibg=darkblue
  hi link GlobalComment     cyan
  hi link LocalComment      darkblue
  hi link Interpretation    magenta
  hi link Measure           measure   
  hi link Exclusive         red
  hi link Chord             white
  hi link BadTabbing        Error
  hi link ExtraSpace        space
  hi link BibRecord         darkgreen
endif


let b:current_syntax = "humdrum"


