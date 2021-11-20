#!/bin/bash

# Be verbose.
set -x

UNAME=$(command -v uname)
printf 'uname=${UNAME}\n'
case $("${UNAME}" | tr '[:upper:]' '[:lower:]') in
  linux*)
    sudo apt-get update
    sudo apt-get install -qq ninja-build mesa-common-dev libglu1-mesa-dev
    ;;
  darwin*)
    brew install -v ninja
    ;;
  msys*|cygwin*|mingw*|nt|win*)
    printf 'windows\n'
    ;;
esac
