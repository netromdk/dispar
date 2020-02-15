# Qt 5
set(QT_MINIMUM_VERSION "5.14.0")

# Added some paths to help find the modules on some systems.
if (NOT WIN32)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
    "/usr/local/Qt-5.6.0"
    "/usr/local/Qt-5.4.1"
    "/usr/local/Qt-5.4.0"
    "/usr/local/opt"
    "/usr/local/opt/qt5"
    "/opt/local/libexec/qt5"
    "/opt/qt56"
    )
else()
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
    "C:/qt-everywhere-opensource-src-5.6.0-w64/qtbase/lib/cmake"
    "C:/qt-everywhere-opensource-src-5.4.1-w64/qtbase/lib/cmake"
    "C:/qt-everywhere-opensource-src-5.4.0-w64/qtbase/lib/cmake"
    )
endif()

set(CMAKE_AUTOMOC ON) # moc when necessary.
set(CMAKE_AUTORCC ON) # rcc when necessary.

# As moc files are generated in the binary dir, tell CMake to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

function (requireQtModule module)
  find_package(${module} REQUIRED)
  if (NOT ${module}_FOUND)
    message(FATAL_ERROR "Could not find module: ${module}!")
    return()
  endif()

  if (${module}_VERSION VERSION_LESS ${QT_MINIMUM_VERSION})
    message(FATAL_ERROR "Minimum supported Qt5 version is ${QT_MINIMUM_VERSION}!")
  endif()

  message(STATUS "Found ${module} with version ${${module}_VERSION}")
endfunction()

requireQtModule(Qt5Core)
requireQtModule(Qt5Gui)
requireQtModule(Qt5Widgets)

set(FOUND_QT YES)
