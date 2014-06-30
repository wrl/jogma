autocmd BufEnter wscript set sw=4 ts=4 sta et fo=croql
autocmd BufLeave wscript set noet

let g:syntastic_mode_map = {
			\ "mode": "active",
			\ "active_filetypes": [],
			\ "passive_filetypes": []}

let g:syntastic_c_checkers = ['gcc']
let g:syntastic_c_compiler_options = "-std=c99 -D_GNU_SOURCE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers"
let g:syntastic_c_include_dirs = [
			\ "include",
			\ "third-party",
			\ "build"]

set path+=include,build,third-party
