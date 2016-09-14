# Qt 5
if (NOT WIN32)
  # Added some paths to help find the modules on some systems.
  set(CMAKE_PREFIX_PATH
    ${CMAKE_PREFIX_PATH}
    "/usr/local/opt"
    "/usr/local/opt/qt5"
    "/usr/local/Qt-5.6.0"
    "/usr/local/Qt-5.4.1"
    )
endif()

set(CMAKE_AUTOMOC ON) # Moc when necessary.

# As moc files are generated in the binary dir, tell CMake to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
#find_package(Qt5Gui REQUIRED)
#find_package(Qt5Widgets REQUIRED)
