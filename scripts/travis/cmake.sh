cmake --system-information info.txt && cat info.txt

mkdir build
cd build

# Use address sanitizer with Clang on macOS.
if [[ $TRAVIS_OS_NAME == "osx" ]]; then
  cmake -DBUILD_TESTS=ON -DADDRESS_SANITIZER=ON ..
else
  cmake -DBUILD_TESTS=ON ..
fi
