# The property can be: COMPILE_FLAGS, LINK_FLAGS and so on.
function(append_target_properties target property value)
  get_target_property(flags ${target} ${property})
  if (${flags} MATCHES "NOTFOUND")
    SET(flags "")
  endif()
  set_target_properties(
    ${target}
    PROPERTIES
    ${property} "${flags} ${value}"
    )
endfunction()

# Disable all warnings for a target.
function(disable_all_warnings target)
  if (NOT MSVC)
    append_target_properties(${target} COMPILE_FLAGS "-w")
  endif()
endfunction()
