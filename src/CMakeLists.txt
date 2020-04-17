include_directories(
  SYSTEM
  ${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/lib/capstone/include
  )

LIBIBERTY_ADD_DEFINITIONS()

add_library(
  ${libName}

  Util.h
  Util.cc

  Context.h
  Context.cc
  LogHandler.h
  LogHandler.cc
  Project.h
  Project.cc

  Reader.h
  Reader.cc
  CStringReader.h
  CStringReader.cc

  Disassembler.h
  Disassembler.cc

  Section.h
  Section.cc

  CpuType.h
  CpuType.cc
  FileType.h
  FileType.cc

  BinaryObject.h
  BinaryObject.cc
  SymbolEntry.h
  SymbolEntry.cc
  SymbolTable.h
  SymbolTable.cc

  Debugger.h
  Debugger.cc

  widgets/MainWindow.h
  widgets/MainWindow.cc
  widgets/BinaryWidget.h
  widgets/BinaryWidget.cc
  widgets/PersistentSplitter.h
  widgets/PersistentSplitter.cc
  widgets/AboutDialog.h
  widgets/AboutDialog.cc
  widgets/ConversionHelper.h
  widgets/ConversionHelper.cc
  widgets/DisassemblerDialog.h
  widgets/DisassemblerDialog.cc
  widgets/OptionsDialog.h
  widgets/OptionsDialog.cc
  widgets/TagsEdit.h
  widgets/TagsEdit.cc
  widgets/TagItemDelegate.h
  widgets/TagItemDelegate.cc
  widgets/ToggleBox.h
  widgets/ToggleBox.cc
  widgets/DisassemblyEditor.h
  widgets/DisassemblyEditor.cc
  widgets/LineEdit.h
  widgets/LineEdit.cc
  widgets/TreeWidget.h
  widgets/TreeWidget.cc
  widgets/HexEditor.h
  widgets/HexEditor.cc
  widgets/HexEdit.h
  widgets/HexEdit.cc
  widgets/NumberValidator.h
  widgets/NumberValidator.cc
  widgets/AsciiValidator.h
  widgets/AsciiValidator.cc
  widgets/MacSdkVersionsEditor.h
  widgets/MacSdkVersionsEditor.cc
  widgets/LogDialog.h
  widgets/LogDialog.cc
  widgets/OmniSearchDialog.h
  widgets/OmniSearchDialog.cc

  formats/Format.h
  formats/Format.cc
  formats/FormatLoader.h
  formats/FormatLoader.cc
  formats/MachO.h
  formats/MachO.cc

  AddrHexAsciiEncoder.h
  AddrHexAsciiEncoder.cc

  MacSdkVersionPatcher.h
  MacSdkVersionPatcher.cc
  )

target_link_libraries(
  ${libName}
  capstone-static
  iberty
  )

if (LINUX)
  target_link_libraries(
    ${libName}
    pthread
    )
endif()

qt5_use_modules(
  ${libName}
  Core
  Gui
  Widgets
  )

add_dispar_executable(
  ${binName}

  main.cc
  )

# Create bundle on Mac. Note that "misc/MacOSXBundleInfo.plist.in" is used as template for the
# "Info.plist" of the app, including subsituting in MAJOR_VERSION and MINOR_VERSION from this file.
set_target_properties(
  ${binName}
  PROPERTIES MACOSX_BUNDLE TRUE
  )

target_link_libraries(
  ${binName}
  ${libName}
  )

# Installation definitions.
if (APPLE)
  set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/${binName}.app/Contents/MacOS/")
  set(INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${binName}.app")
else()
  set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/${binName}/bin/")
  set(INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${binName}/")
endif()

install(
  TARGETS ${binName}
  BUNDLE DESTINATION . # Mac
  RUNTIME DESTINATION ${INSTALL_BIN_DIR} # Win + Linux
  )

set(INSTALL_TARGET_NAME "${binName}")
set(lib_dir "${Qt5Core_DIR}/../../")
set(bin_dir "${Qt5Core_DIR}/../../../bin/")
set(plugin_dir "${Qt5Core_DIR}/../../../plugins/")

if (APPLE)
  install(
    FILES "${plugin_dir}/platforms/libqcocoa.dylib"
    DESTINATION "${INSTALL_BIN_DIR}/platforms/"
    )
  install(
    FILES "${plugin_dir}/styles/libqmacstyle.dylib"
    DESTINATION "${INSTALL_BIN_DIR}/styles/"
    )
elseif (WIN32)
  install(
    FILES "${plugin_dir}/platforms/qwindows.dll"
    DESTINATION "${INSTALL_BIN_DIR}/platforms/"
    )
  if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    install(
      FILES "${plugin_dir}/platforms/qwindowsd.dll"
      DESTINATION "${INSTALL_BIN_DIR}/platforms/"
      )
  endif()
  install(
    FILES "${plugin_dir}/styles/qwindowsvistastyle.dll"
    DESTINATION "${INSTALL_BIN_DIR}/styles/"
    )
  if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    install(
      FILES "${plugin_dir}/styles/qwindowsvistastyled.dll"
      DESTINATION "${INSTALL_BIN_DIR}/styles/"
      )
  endif()
  install(
    FILES
      "${bin_dir}/Qt5Core.dll"
      "${bin_dir}/Qt5Gui.dll"
      "${bin_dir}/Qt5Widgets.dll"
    DESTINATION "${INSTALL_BIN_DIR}/"
    )
elseif (LINUX)
  install(
    FILES "${plugin_dir}/platforms/libqxcb.so"
    DESTINATION "${INSTALL_BIN_DIR}/platforms/"
    )
  install(
    FILES
      "${lib_dir}/libQt5XcbQpa.so.5"
      "${lib_dir}/libQt5XcbQpa.so.${QT_VERSION}"
      "${lib_dir}/libQt5DBus.so.5"
      "${lib_dir}/libQt5DBus.so.${QT_VERSION}"
    DESTINATION "${INSTALL_BIN_DIR}/"
    PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
    )
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/misc/install.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/install.cmake
  @ONLY
  )

install(
  SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install.cmake
  )
