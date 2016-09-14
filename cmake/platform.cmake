include(compilation)
include(deps)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX 1)
endif()

if (APPLE)
  include(mac)
endif()
