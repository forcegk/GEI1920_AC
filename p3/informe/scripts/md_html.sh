#!/bin/sh

# YA NO USO ESTE
# # generate-md is package markdown-styles from AUR or from
# # https://github.com/mixu/markdown-styles
# generate-md --layout github --input *.md --output .

# ESTA ES LA NUEVA HERRAMIENTA QUE USO
# https://github.com/KrauseFx/markdown-to-html-github-style
cp ../*.md ./markdown-to-html-github-style/README.md
cd markdown-to-html-github-style
npm install
node convert.js informe_p3
cp README.html ../../informe_p3.html
cd ../..
