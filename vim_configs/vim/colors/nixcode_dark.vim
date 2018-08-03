set background=dark
highlight clear
if exists("syntax_on")
  syntax reset
endif
let colors_name = "nixcode_dark"

hi Normal ctermbg=Black ctermfg=LightGrey guibg=#1f201f guifg=#e1f4f8

" General items
hi Identifier ctermbg=NONE ctermfg=Blue guibg=NONE guifg=#00b8ff
hi Statement ctermbg=NONE ctermfg=Green guibg=NONE guifg=#7ebc3d
hi PreProc ctermbg=NONE ctermfg=Magenta guibg=NONE guifg=#ff75be
hi Type ctermbg=NONE ctermfg=Cyan guibg=NONE guifg=#47bdb2
hi Special ctermbg=NONE ctermfg=Red guibg=NONE guifg=#ff6a74
hi Comment ctermbg=NONE ctermfg=LightGrey guibg=NONE guifg=#9ca3b3
hi Todo ctermbg=NONE ctermfg=LightGrey cterm=Bold guibg=NONE guifg=#9ca3b3 gui=Bold

" Literals
hi Constant ctermbg=NONE ctermfg=Yellow guibg=NONE guifg=#e8b038
hi String ctermbg=NONE ctermfg=Brown guibg=NONE guifg=#ff8751
hi Character ctermbg=NONE ctermfg=Brown guibg=NONE guifg=#ff8751
hi Number ctermbg=NONE ctermfg=Yellow guibg=NONE guifg=#e8b038
hi Boolean ctermbg=NONE ctermfg=Blue guibg=NONE guifg=#00b8ff
hi Float ctermbg=NONE ctermfg=Yellow guibg=NONE guifg=#e8b038

" Special highlights
hi Underlined ctermbg=NONE ctermfg=DarkBlue guibg=NONE guifg=#7e9af6
hi Ignore ctermbg=NONE ctermfg=NONE guibg=NONE guifg=NONE
hi Error ctermbg=NONE ctermfg=Red cterm=Bold guibg=NONE guifg=#ff6a74 gui=Bold

" Periphery
hi StatusLine ctermbg=bg ctermfg=fg cterm=Reverse guibg=bg guifg=fg gui=Reverse
hi CursorLine ctermbg=DarkGrey ctermfg=NONE guibg=#3e3e3d guifg=NONE
hi Visual ctermbg=DarkGrey ctermfg=NONE guibg=#595959 guifg=NONE
hi LineNr ctermbg=Black ctermfg=LightGrey guibg=#1f201f guifg=#9ca3b3
hi ColorColumn ctermbg=Black ctermfg=NONE guibg=#000000 guifg=NONE
hi MatchParen ctermbg=DarkGrey ctermfg=NONE guibg=#595959 guifg=NONE
hi Folded ctermbg=NONE ctermfg=NONE cterm=Italic guibg=NONE guifg=NONE gui=Italic
hi FoldColumn ctermbg=Black ctermfg=LightGrey guibg=#1f201f guifg=#9ca3b3

" Spell-check highlights
hi SpellBad ctermbg=NONE ctermfg=NONE cterm=Underline guibg=NONE guifg=NONE guisp=#ff6a74 gui=Undercurl
hi SpellCap ctermbg=NONE ctermfg=NONE cterm=Underline guibg=NONE guifg=NONE guisp=#7ebc3d gui=Undercurl
hi SpellRare ctermbg=NONE ctermfg=NONE cterm=Underline guibg=NONE guifg=NONE guisp=#e8b038 gui=Undercurl
hi SpellLocal ctermbg=NONE ctermfg=NONE cterm=Underline guibg=NONE guifg=NONE guisp=#7e9af6 gui=Undercurl
