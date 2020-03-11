#!/bin/sh
SCAN_BUILD=$1
SRC_FLD=$2
BUILD_FLD=$3
QT_DIR=$4
TARGET=$5

set -x

# Hack to make ccc-analyzer find Qt includes properly!
rm -f ${BUILD_FLD}/src/QtCore ${BUILD_FLD}/src/QtGui ${BUILD_FLD}/src/QtWidgets
ln -s ${QT_DIR}/QtCore.framework/Headers/ ${BUILD_FLD}/src/QtCore
ln -s ${QT_DIR}/QtGui.framework/Headers/ ${BUILD_FLD}/src/QtGui
ln -s ${QT_DIR}/QtWidgets.framework/Headers/ ${BUILD_FLD}/src/QtWidgets

${SCAN_BUILD} -v -V -k \
  --exclude ${SRC_FLD}/lib/ --exclude src/QtCore --exclude src/QtGui --exclude src/QtWidgets \
  cmake --build ${BUILD_FLD} --target ${TARGET}
