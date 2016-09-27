#!/bin/sh
BUILD_FLD=$1
QT_DIR=$2
TARGET=$3

# Hack to make ccc-analyzer find Qt includes properly!
rm -f ${BUILD_FLD}/src/QtCore ${BUILD_FLD}/src/QtGui ${BUILD_FLD}/src/QtWidgets
ln -s ${QT_DIR}/QtCore.framework/Headers/ ${BUILD_FLD}/src/QtCore
ln -s ${QT_DIR}/QtGui.framework/Headers/ ${BUILD_FLD}/src/QtGui
ln -s ${QT_DIR}/QtWidgets.framework/Headers/ ${BUILD_FLD}/src/QtWidgets

scan-build -v -V -k ninja ${TARGET}
