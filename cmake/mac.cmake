set(SDK_MIN "10.11")
set(SDK "10.11")
set(DEV_SDK "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK}.sdk")
SET(DEV_SDK2 "/Developer/SDKs/MacOSX${SDK}.sdk")

if (NOT EXISTS "${DEV_SDK}" AND NOT IS_DIRECTORY "${DEV_SDK}")
  if (NOT EXISTS "${DEV_SDK2}" AND NOT IS_DIRECTORY "${DEV_SDK2}")
    message("Could not find Mac OSX SDK at: ${DEV_SDK} or ${DEV_SDK2}")
    message("Aborting!")
    return()
  else()
    set(DEV_SDK ${DEV_SDK2})
  endif()
endif()

add_definitions(
  -DMAC
  -DGCC_VISIBILITY
  -mmacosx-version-min=${SDK_MIN}
  )

set(CMAKE_OSX_SYSROOT ${DEV_SDK})

# Silence annoying "ranlib: file: ...o has no symbols" warnings! On Mac the ar utility will run
# ranlib, which causes the warning. Using the "Src" argument stops ar from executing ranlib, and
# when ranlib is executed normally we give it "-no_warning_for_no_symbols" to suppress the warning.
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
