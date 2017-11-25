#!/bin/sh
#
# This script produces a release archive with no git and only the bare essentials to build the
# project.
#
# Usage:
#   ./make_release.sh <path to dispar project root>

if [ ! $# -eq 1 ]; then
    echo "Usage: ./make_release.sh <path to dispar project root>"
    exit -1
fi

SRC_DIR=$1
DIR="dispar"
DEST_DIR="/tmp/${DIR}"
DEST_ZIP="/tmp/${DIR}.zip"

set -x

# Setup
rm -fr ${DEST_DIR}
mkdir -p ${DEST_DIR}

# Copy files
pushd ${SRC_DIR}
cp -r .clang-format *.md *.txt LICENSE cmake .github lib misc scripts src tests ${DEST_DIR}
popd

# Cleanup cruft
pushd ${DEST_DIR}/lib/capstone
rm -fr .git* tests *.sh *.mk xcode suite packages msvc docs bindings contrib
popd
find ${DEST_DIR} -iname .DS_Store | xargs rm -f

# Make archive
rm -f ${DEST_ZIP}
pushd ${DEST_DIR}/..
zip -qr9 ${DEST_ZIP} ${DIR}
popd

# Cleanup cruft
rm -fr ${DEST_DIR}
ls -lh ${DEST_ZIP}
