#!/bin/bash

# Be verbose.
set -x

# Linux
if [[ $TRAVIS_OS_NAME == "linux" ]]; then
    sudo add-apt-repository --yes ppa:beineri/opt-qt-5.14.1-bionic
    sudo apt-get update -qq
fi

# macOS
if [[ $TRAVIS_OS_NAME == "osx" ]]; then
    brew update
fi
