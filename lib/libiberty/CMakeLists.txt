add_library(
  iberty

  cp-demangle.c
  d-demangle.c
  cp-demint.c
  cplus-dem.c
  safe-ctype.c
  alloca.c
  xmalloc.c
  xstrdup.c
  xexit.c
  )

# Compile with correct flags to fix warnings.
append_target_properties(
  iberty
  COMPILE_FLAGS "-DHAVE_DECL_BASENAME -DHAVE_STDLIB_H -DHAVE_STRING_H"
  )

# When included in dispar we need to set the following definitions.
macro(LIBIBERTY_ADD_DEFINITIONS)
  # We don't use basename() so skip the extern that fails sometimes!
  add_definitions(
    -DHAVE_DECL_BASENAME
    -DHAVE_STDLIB_H
    -DHAVE_STRING_H
    )
endmacro()
