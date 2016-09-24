add_definitions(
  -DWIN
  -D_USE_MATH_DEFINES
  -DNOMINMAX
  -D_UNICODE
  -DUNICODE
  )

macro(prepare_win_executable target)
  if (WIN32)
    append_target_properties(${target} LINK_FLAGS_DEBUG "/subsystem:console")
    append_target_properties(${target} LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
  endif()
endmacro()