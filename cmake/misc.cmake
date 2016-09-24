# The property can be: COMPILE_FLAGS, LINK_FLAGS and so on.
macro(append_target_properties target property value)
  get_target_property(flags ${target} ${property})
  if (${flags} MATCHES "NOTFOUND")
    SET(flags "")
  endif()
  set_target_properties(
    ${target}
    PROPERTIES
    ${property} "${flags} ${value}"
    )
endmacro()
