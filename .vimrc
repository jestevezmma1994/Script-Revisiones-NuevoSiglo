set number
syntax enable
set numberwidth=1
set clipboard=unnamed
set showcmd
set ruler
set encoding=utf-8
set sw=2
set showmatch
set laststatus=2
set noshowmode
set mouse=a

"esto hay q instalarlo para poder añadir plugins
"$ curl -fLo ~/.vim/autoload/plug.vim --create-dirs https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim


call plug#begin('~/.vim/plugged')


" Temas
Plug 'morhetz/gruvbox'


"IDE
Plug 'easymotion/vim-easymotion'
Plug 'christoomey/vim-tmux-navigator'
Plug 'scrooloose/nerdtree'
call plug#end()




let g:rehash256 = 1
let mapleader=" "
let  g:molokai_original = 1

nmap <Leader>s <Plug>(easymotion-s2)

nmap <Leader>nt :NERDTreeFind<CR>
nmap <Leader>w :w<CR>
nmap <Leader>q :q<CR>








