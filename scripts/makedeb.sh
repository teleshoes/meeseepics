#!/bin/sh
project="meeseepics"

/scratchbox/login -d $HOME/$project/build dpkg-buildpackage -j4
