#!/bin/sh
DIR=$1
BINARY_FILE=$2
LLVM_PROFDATA=$3
LLVM_COV=$4
LLVM_PROFILE_FILE=tests.profraw
LLVM_PROFILE_DATA=tests.profdata
REPORT_DIR=report.dir

pushd ${DIR}
rm -fr ${REPORT_DIR}
"${BINARY_FILE}" >/dev/null && \
    ${LLVM_PROFDATA} merge -sparse ${LLVM_PROFILE_FILE} -o ${LLVM_PROFILE_DATA} && \
    ${LLVM_COV} show "${BINARY_FILE}" -instr-profile=${LLVM_PROFILE_DATA} -format html -o ${REPORT_DIR} -Xdemangler c++filt -Xdemangler -n -show-line-counts-or-regions && \
    ${LLVM_COV} report "${BINARY_FILE}" -instr-profile=${LLVM_PROFILE_DATA} -Xdemangler c++filt -Xdemangler -n -use-color && \
    echo "============================\nReport at: ${REPORT_DIR}/index.html\n============================"
popd
