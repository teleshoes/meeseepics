#!/bin/sh
project="meeseepics"
dir="$HOME/$project/build"
/scratchbox/login mkdir -p "$dir"
/scratchbox/login -d "$dir" rm -f debian
/scratchbox/login -d "$dir" ln -s ../qtc_packaging/debian_harmattan/ debian
/scratchbox/login -d "$dir" qmake ..
