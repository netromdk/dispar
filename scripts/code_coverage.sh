#!/bin/sh
DIR=$1
LLVM_PROFDATA=$2
LLVM_COV=$3
LLVM_PROFILE_DATA=tests.profdata
REPORT_DIR=report.dir

pushd ${DIR}
rm -fr ${REPORT_DIR} *.profraw *.profdata

# Generate profiling data from all tests executables.
for f in $(find "${DIR}/bin" -iname 'test_*'); do
  LLVM_PROFILE_FILE="$(basename $f).profraw" $f >/dev/null 2>/dev/null
  COV_BINARIES="${COV_BINARIES} -object $f"
done

# Merge all raw profile data into one data file.
${LLVM_PROFDATA} merge -sparse *.profraw -o ${LLVM_PROFILE_DATA}

# Generate HTML report.
${LLVM_COV} show -instr-profile=${LLVM_PROFILE_DATA} -format html -o ${REPORT_DIR}\
            -Xdemangler c++filt -Xdemangler -n -show-line-counts-or-regions\
            -ignore-filename-regex="(build|tests)/*"\
            ${COV_BINARIES}

echo "===========================\nReport at: ${REPORT_DIR}/index.html\n==========================="
popd
