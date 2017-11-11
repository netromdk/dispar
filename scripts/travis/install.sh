#!/bin/bash

# Be verbose.
set -x

# Linux
if [[ $TRAVIS_OS_NAME == "linux" ]]; then
    sudo apt-get install -qq qt56base
fi

# macOS
if [[ $TRAVIS_OS_NAME == "osx" ]]; then
    brew install -v qt5
fi
