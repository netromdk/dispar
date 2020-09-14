# Qt 5
set(QT_MINIMUM_VERSION "5.14.0")

# Added some paths to help find the modules on some systems.
if (NOT WIN32)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
    "/usr/local/Cellar/qt"
    "/usr/local/Qt-5.15.1"
    "/usr/local/Qt-5.15.0"
    "/usr/local/Qt-5.14.2"
    "/usr/local/Qt-5.14.1"
    "/usr/local/Qt-5.14.0"
    "$ENV{HOME}/Qt/5.15.1/gcc_64"
    "$ENV{HOME}/Qt/5.15.0/gcc_64"
    "$ENV{HOME}/Qt/5.14.2/gcc_64"
    "$ENV{HOME}/Qt/5.14.1/gcc_64"
    "$ENV{HOME}/Qt/5.14.0/gcc_64"
    "/usr/local/opt"
    "/usr/local/opt/qt5"
    "/opt/local/libexec/qt5"
    "/opt/qt515"
    "/opt/qt514"
    )
else()
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
    "C:/Qt/5.15.1/msvc2017_64/lib/cmake"
    "C:/Qt/5.15.0/msvc2017_64/lib/cmake"
    "C:/Qt/5.14.2/msvc2017_64/lib/cmake"
    "C:/Qt/5.14.1/msvc2017_64/lib/cmake"
    "C:/Qt/5.14.0/msvc2017_64/lib/cmake"
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

  set(version ${${module}_VERSION})
  message(STATUS "Found ${module} with version ${version}")

  set(QT_VERSION ${version} PARENT_SCOPE)
endfunction()

requireQtModule(Qt5Core)
requireQtModule(Qt5Gui)
requireQtModule(Qt5Widgets)

# Required deps of libqcocoa.dylib.
if (APPLE)
  requireQtModule(Qt5PrintSupport)
  requireQtModule(Qt5DBus)

# Required deps of libqxcb.so.
elseif (LINUX)
  requireQtModule(Qt5DBus)
endif()

set(FOUND_QT YES)
