macro(LIBIBERTY_ADD_DEFINITIONS)
  # We don't use basename() so skip the extern that fails sometimes!
  add_definitions(
    -DHAVE_DECL_BASENAME
    )
endmacro()

add_library(
  iberty

  cp-demangle.c
  d-demangle.c
  cp-demint.c
  cplus-dem.c
  safe-ctype.c
  xmalloc.c
  xstrdup.c
  xexit.c
  )
