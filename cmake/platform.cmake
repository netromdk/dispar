include(compilation)
include(deps)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX 1)
endif()

if (APPLE)
  include(mac)
elseif (LINUX)
  include(linux)
elseif (WIN32)
  include(win)
endif()
