set(SDK_MIN "10.11")
set(SDK "10.11")
set(DEV_SDK "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK}.sdk")

if (NOT EXISTS "${DEV_SDK}" AND NOT IS_DIRECTORY "${DEV_SDK}")
  MESSAGE("Could not find Mac OSX SDK at: ${DEV_SDK}")
  MESSAGE("Aborting!")
  RETURN()
endif()

add_definitions(
  -DMAC
  -DGCC_VISIBILITY
  -mmacosx-version-min=${SDK_MIN}
  )

set(CMAKE_OSX_SYSROOT ${DEV_SDK})
