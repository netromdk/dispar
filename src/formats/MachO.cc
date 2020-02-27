#include <QDebug>
#include <QFile>

#include <cmath>

#include "BinaryObject.h"
#include "Reader.h"
#include "Util.h"
#include "formats/MachO.h"

namespace dispar {

MachO::MachO(const QString &file) : Format(Format::Type::MACH_O), file_{file}
{
}

QString MachO::file() const
{
  return file_;
}

bool MachO::detect()
{
  QFile f{file_};
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  Reader r(f);
  bool ok;
  quint32 magic = r.getUInt32(&ok);
  if (!ok) return false;

  return magic == 0xFEEDFACE || // 32-bit little endian
         magic == 0xFEEDFACF || // 64-bit little endian
         magic == 0xECAFDEEF || // 32-bit big endian
         magic == 0xFCAFDEEF || // 64-bit big endian
         magic == 0xCAFEBABE || // Universal binary little endian
         magic == 0xBEBAFECA;   // Universal binary big endian
}

bool MachO::parse()
{
  QFile f{file_};
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  Reader r(f);
  bool ok;
  quint32 magic = r.getUInt32(&ok);
  if (!ok) return false;

  // Check if this is a universal "fat" binary.
  if (magic == 0xCAFEBABE || magic == 0xBEBAFECA) {
    // Values are saved as big-endian so read as such.
    r.setLittleEndian(false);

    quint32 nfat_arch = r.getUInt32(&ok);
    if (!ok) return false;

    // Read "fat" headers.
    typedef QPair<quint32, quint32> puu;
    QList<puu> archs;
    for (quint32 i = 0; i < nfat_arch; i++) {
      // CPU type.
      r.getUInt32(&ok);
      if (!ok) return false;

      // CPU sub type.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to this object file.
      quint32 offset = r.getUInt32(&ok);
      if (!ok) return false;

      // Size of this object file.
      quint32 size = r.getUInt32(&ok);
      if (!ok) return false;

      // Alignment as a power of 2.
      r.getUInt32(&ok);
      if (!ok) return false;

      archs << puu(offset, size);
    }

    // Parse the actual binary objects.
    for (const auto &arch : archs) {
      if (!parseHeader(arch.first, arch.second, r)) {
        return false;
      }
    }
  }

  // Otherwise, just parse a single object file.
  else {
    return parseHeader(0, 0, r);
  }

  return true;
}

QList<BinaryObject *> MachO::objects() const
{
  QList<BinaryObject *> res;
  for (auto &object : objects_) {
    res << object.get();
  }
  return res;
}

bool MachO::parseHeader(quint32 offset, quint32 size, Reader &r)
{
  auto binaryObject = std::make_unique<BinaryObject>();

  r.seek(offset);
  r.setLittleEndian(true);

  bool ok;
  quint32 magic = r.getUInt32(&ok);
  if (!ok) return false;

  int systemBits{32};
  bool littleEndian{true};
  if (magic == 0xFEEDFACE) {
    systemBits = 32;
    littleEndian = true;
  }
  else if (magic == 0xFEEDFACF) {
    systemBits = 64;
    littleEndian = true;
  }
  else if (magic == 0xECAFDEEF) {
    systemBits = 32;
    littleEndian = false;
  }
  else if (magic == 0xFCAFDEEF) {
    systemBits = 64;
    littleEndian = false;
  }

  binaryObject->setSystemBits(systemBits);
  binaryObject->setLittleEndian(littleEndian);

  // Read info in the endianness of the file.
  r.setLittleEndian(littleEndian);

  quint32 cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags;

  cputype = r.getUInt32(&ok);
  if (!ok) return false;

  cpusubtype = r.getUInt32(&ok);
  if (!ok) return false;

  filetype = r.getUInt32(&ok);
  if (!ok) return false;

  ncmds = r.getUInt32(&ok);
  if (!ok) return false;

  sizeofcmds = r.getUInt32(&ok);
  if (!ok) return false;
  (void) sizeofcmds; // Mark used.

  flags = r.getUInt32(&ok);
  if (!ok) return false;
  (void) flags; // Mark used.

  // Read reserved field.
  if (systemBits == 64) {
    r.getUInt32();
  }

  // Types in /usr/include/mach/machine.h
  CpuType cpuType{CpuType::X86};
  if (cputype == 7) { // CPU_TYPE_X86, CPU_TYPE_I386
    cpuType = CpuType::X86;
  }
  else if (cputype == 7 + 0x01000000) { // CPU_TYPE_X86 | CPU_ARCH_ABI64
    cpuType = CpuType::X86_64;
  }
  else if (cputype == 11) { // CPU_TYPE_HPPA
    cpuType = CpuType::HPPA;
  }
  else if (cputype == 12) { // CPU_TYPE_ARM
    cpuType = CpuType::ARM;
  }
  else if (cputype == 14) { // CPU_TYPE_SPARC
    cpuType = CpuType::SPARC;
  }
  else if (cputype == 15) { // CPU_TYPE_I860
    cpuType = CpuType::I860;
  }
  else if (cputype == 18) { // CPU_TYPE_POWERPC
    cpuType = CpuType::POWER_PC;
  }
  else if (cputype == 18 + 0x01000000) { // CPU_TYPE_POWERPC | CPU_ARCH_ABI64
    cpuType = CpuType::POWER_PC_64;
  }

  binaryObject->setCpuType(cpuType);

  // Subtract 64-bit mask.
  if (systemBits == 64) {
    cpusubtype -= 0x80000000;
  }

  CpuType cpuSubType{CpuType::I386};
  if (cpusubtype == 3) { // CPU_SUBTYPE_386
    cpuSubType = CpuType::I386;
  }
  else if (cpusubtype == 4) { // CPU_SUBTYPE_486
    cpuSubType = CpuType::I486;
  }
  else if (cpusubtype == 4 + (8 << 4)) { // CPU_SUBTYPE_486SX
    cpuSubType = CpuType::I486_SX;
  }
  else if (cpusubtype == 5) { // CPU_SUBTYPE_PENT
    cpuSubType = CpuType::PENTIUM;
  }
  else if (cpusubtype == 6 + (1 << 4)) { // CPU_SUBTYPE_PENTPRO
    cpuSubType = CpuType::PENTIUM_PRO;
  }
  else if (cpusubtype == 6 + (3 << 4)) { // CPU_SUBTYPE_PENTII_M3
    cpuSubType = CpuType::PENTIUM_II_M3;
  }
  else if (cpusubtype == 6 + (5 << 4)) { // CPU_SUBTYPE_PENTII_M5
    cpuSubType = CpuType::PENTIUM_II_M5;
  }
  else if (cpusubtype == 7 + (6 << 4)) { // CPU_SUBTYPE_CELERON
    cpuSubType = CpuType::CELERON;
  }
  else if (cpusubtype == 7 + (7 << 4)) { // CPU_SUBTYPE_CELERON_MOBILE
    cpuSubType = CpuType::CELERON_MOBILE;
  }
  else if (cpusubtype == 8) { // CPU_SUBTYPE_PENTIUM_3
    cpuSubType = CpuType::PENTIUM_3;
  }
  else if (cpusubtype == 8 + (1 << 4)) { // CPU_SUBTYPE_PENTIUM_3_M
    cpuSubType = CpuType::PENTIUM_3_M;
  }
  else if (cpusubtype == 8 + (2 << 4)) { // CPU_SUBTYPE_PENTIUM_3_XEON
    cpuSubType = CpuType::PENTIUM_3_Xeon;
  }
  else if (cpusubtype == 9) { // CPU_SUBTYPE_PENTIUM_M
    cpuSubType = CpuType::PENTIUM_M;
  }
  else if (cpusubtype == 10) { // CPU_SUBTYPE_PENTIUM_4
    cpuSubType = CpuType::PENTIUM_4;
  }
  else if (cpusubtype == 10 + (1 << 4)) { // CPU_SUBTYPE_PENTIUM_4_M
    cpuSubType = CpuType::PENTIUM_4_M;
  }
  else if (cpusubtype == 11) { // CPU_SUBTYPE_ITANIUM
    cpuSubType = CpuType::ITANIUM;
  }
  else if (cpusubtype == 11 + (1 << 4)) { // CPU_SUBTYPE_ITANIUM_2
    cpuSubType = CpuType::ITANIUM_2;
  }
  else if (cpusubtype == 12) { // CPU_SUBTYPE_XEON
    cpuSubType = CpuType::XEON;
  }
  else if (cpusubtype == 12 + (1 << 4)) { // CPU_SUBTYPE_XEON_MP
    cpuSubType = CpuType::XEON_MP;
  }

  binaryObject->setCpuSubType(cpuSubType);

  FileType fileType{FileType::OBJECT};
  if (filetype == 1) { // MH_OBJECT
    fileType = FileType::OBJECT;
  }
  else if (filetype == 2) { // MH_EXECUTE
    fileType = FileType::EXECUTE;
  }
  else if (filetype == 4) { // MH_CORE
    fileType = FileType::CORE;
  }
  else if (filetype == 5) { // MH_PRELOAD
    fileType = FileType::PRELOAD;
  }
  else if (filetype == 6) { // MH_DYLIB
    fileType = FileType::DYLIB;
  }
  else if (filetype == 7) { // MH_DYLINKER
    fileType = FileType::DYLINKER;
  }
  else if (filetype == 8) { // MH_BUNDLE
    fileType = FileType::BUNDLE;
  }

  binaryObject->setFileType(fileType);

  // TODO: Load flags when necessary.

  // Symbol table offset and number of elements in it.
  quint32 symoff{0}, symnum{0};

  // Dynamic (indirect) symbol table offset and number of elements in
  // it.
  quint32 indirsymoff{0}, indirsymnum{0};

  // Parse load commands sequentially. Each consists of the type, size
  // and data.
  for (decltype(ncmds) i = 0; i < ncmds; i++) {
    quint32 type = r.getUInt32(&ok);
    if (!ok) return false;

    quint32 cmdsize = r.getUInt32(&ok);
    if (!ok) return false;

    // LC_SEGMENT or LC_SEGMENT_64
    if (type == 1 || type == 25) {
      QString name{r.read(16)};

      // Memory address of this segment.
      quint64 vmaddr;
      if (systemBits == 32) {
        vmaddr = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        vmaddr = r.getUInt64(&ok);
        if (!ok) return false;
      }
      (void) vmaddr; // Mark used.

      // Memory size of this segment.
      quint64 vmsize;
      if (systemBits == 32) {
        vmsize = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        vmsize = r.getUInt64(&ok);
        if (!ok) return false;
      }
      (void) vmsize; // Mark used.

      // File offset of this segment.
      quint64 fileoff;
      if (systemBits == 32) {
        fileoff = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        fileoff = r.getUInt64(&ok);
        if (!ok) return false;
      }
      (void) fileoff; // Mark used.

      // Amount to map from the file.
      quint64 filesize;
      if (systemBits == 32) {
        filesize = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        filesize = r.getUInt64(&ok);
        if (!ok) return false;
      }
      (void) filesize; // Mark used.

      // Maximum VM protection.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Initial VM protection.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of sections in segment.
      quint32 nsects = r.getUInt32(&ok);
      if (!ok) return false;

      // Flags.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Read sections.
      if (nsects > 0) {
        for (decltype(nsects) j = 0; j < nsects; j++) {
          QString secname{r.read(16)};

          QString segname{r.read(16)};

          // Memory address of this section.
          quint64 addr;
          if (systemBits == 32) {
            addr = r.getUInt32(&ok);
            if (!ok) return false;
          }
          else {
            addr = r.getUInt64(&ok);
            if (!ok) return false;
          }

          // Size in bytes of this section.
          quint64 secsize;
          if (systemBits == 32) {
            secsize = r.getUInt32(&ok);
            if (!ok) return false;
          }
          else {
            secsize = r.getUInt64(&ok);
            if (!ok) return false;
          }

          // File offset of this section.
          quint32 secfileoff = r.getUInt32(&ok);
          if (!ok) return false;

          // Section alignment (power of 2).
          r.getUInt32(&ok);
          if (!ok) return false;

          // File offset of relocation entries.
          r.getUInt32(&ok);
          if (!ok) return false;

          // Number of relocation entries.
          r.getUInt32(&ok);
          if (!ok) return false;

          // Flags.
          r.getUInt32(&ok);
          if (!ok) return false;

          // Reserved fields.
          r.getUInt32();
          r.getUInt32();
          if (systemBits == 64) {
            r.getUInt32();
          }

          // Store needed sections.
          if (segname == "__TEXT") {
            if (secname == "__text") {
              auto sec = std::make_unique<Section>(Section::Type::TEXT, QObject::tr("Program"),
                                                   addr, secsize, offset + secfileoff);
              binaryObject->addSection(std::move(sec));
            }
            else if (secname == "__symbol_stub" || secname == "__stubs") {
              auto sec =
                std::make_unique<Section>(Section::Type::SYMBOL_STUBS, QObject::tr("Symbol Stubs"),
                                          addr, secsize, offset + secfileoff);
              binaryObject->addSection(std::move(sec));
            }
            else if (secname == "__cstring") {
              auto sec = std::make_unique<Section>(Section::Type::CSTRING, QObject::tr("C-Strings"),
                                                   addr, secsize, offset + secfileoff);
              binaryObject->addSection(std::move(sec));
            }
            else if (secname == "__objc_methname") {
              auto sec =
                std::make_unique<Section>(Section::Type::CSTRING, QObject::tr("ObjC Method Names"),
                                          addr, secsize, offset + secfileoff);
              binaryObject->addSection(std::move(sec));
            }
          }
        }
      }
    }

    // LC_DYLD_INFO or LC_DYLD_INFO_ONLY
    else if (type == 0x22 || type == (0x22 | 0x80000000)) {
      // File offset to rebase info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Size of rebase info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to binding info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Size of binding info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to weak binding info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Size of weak binding info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to lazy binding info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Size of lazy binding info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to export info.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Size of export info.
      r.getUInt32(&ok);
      if (!ok) return false;
    }

    // LC_SYMTAB
    else if (type == 2) {
      // Symbol table offset.
      symoff = r.getUInt32(&ok);
      if (!ok) return false;

      // Number of symbol table entries.
      symnum = r.getUInt32(&ok);
      if (!ok) return false;

      // String table offset.
      quint32 stroff = r.getUInt32(&ok);
      if (!ok) return false;

      // String table size in bytes.
      quint32 strsize = r.getUInt32(&ok);
      if (!ok) return false;

      auto sec = std::make_unique<Section>(Section::Type::STRING, QObject::tr("String Table"),
                                           stroff, strsize, offset + stroff);
      binaryObject->addSection(std::move(sec));
    }

    // LC_DYSYMTAB
    else if (type == 0xB) {
      // Index to local symbols.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of local symbols.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Index to externally defined symbols.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of externally defined symbols.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Index to undefined defined symbols.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of undefined defined symbols.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to table of contents.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of entries in the table of contents.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to module table.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of module table entries.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to referenced symbol table.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of referenced symbol table entries.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to indirect symbol table.
      indirsymoff = r.getUInt32(&ok);
      if (!ok) return false;

      // Number of indirect symbol table entries.
      indirsymnum = r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to external relocation entries.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of external relocation entries.
      r.getUInt32(&ok);
      if (!ok) return false;

      // File offset to local relocation entries.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of local relocation entries.
      r.getUInt32(&ok);
      if (!ok) return false;
    }

    // LC_LOAD_DYLIB, LC_ID_DYLIB, LC_LOAD_WEAK_DYLIB, LC_REEXPORT_DYLIB
    else if (type == 0xC || type == 0xD || type == 0x18 + 0x80000000 || type == 0x1F + 0x80000000) {
      // Library path name offset.
      quint32 liboffset = r.getUInt32(&ok);
      if (!ok) return false;

      // Time stamp.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Current version.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Compatibility version.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Library path name.
      r.read(cmdsize - liboffset);
    }

    // LC_LOAD_DYLINKER, LC_ID_DYLINKER or LC_DYLD_ENVIRONMENT
    else if (type == 0xE || type == 0xF || type == 0x27) {
      // Dynamic linker's path name.
      quint32 noffset = r.getUInt32(&ok);
      if (!ok) return false;

      r.read(cmdsize - noffset);
    }

    // LC_UUID
    else if (type == 0x1B) {
      r.read(16);
    }

    // LC_VERSION_MIN_MACOSX, LC_VERSION_MIN_IPHONEOS, LC_VERSION_MIN_WATCHOS, or
    // LC_VERSION_MIN_TVOS
    else if (type == 0x24 || type == 0x25 || type == 0x2F || type == 0x30) {
      // Version (X.Y.Z is encoded in nibbles xxxx.yy.zz)
      const auto targetAddr = r.pos();
      r.getUInt32(&ok);
      if (!ok) return false;

      // SDK version (X.Y.Z is encoded in nibbles xxxx.yy.zz)
      r.getUInt32(&ok);
      if (!ok) return false;

      if (type == 0x24) {
        auto sec = std::make_unique<Section>(Section::Type::LC_VERSION_MIN_MACOSX,
                                             QObject::tr("macOS SDK min version"), targetAddr,
                                             4 * 2, targetAddr);
        binaryObject->addSection(std::move(sec));
      }
      else if (type == 0x25) {
        auto sec = std::make_unique<Section>(Section::Type::LC_VERSION_MIN_IPHONEOS,
                                             QObject::tr("iOS SDK min version"), targetAddr, 4 * 2,
                                             targetAddr);
        binaryObject->addSection(std::move(sec));
      }
      else if (type == 0x2F) {
        auto sec = std::make_unique<Section>(Section::Type::LC_VERSION_MIN_WATCHOS,
                                             QObject::tr("watchOS SDK min version"), targetAddr,
                                             4 * 2, targetAddr);
        binaryObject->addSection(std::move(sec));
      }
      else if (type == 0x30) {
        auto sec = std::make_unique<Section>(Section::Type::LC_VERSION_MIN_TVOS,
                                             QObject::tr("tvOS SDK min version"), targetAddr, 4 * 2,
                                             targetAddr);
        binaryObject->addSection(std::move(sec));
      }
    }

    // LC_SOURCE_VERSION
    else if (type == 0x2A) {
      // Version (A.B.C.D.E packed as a24.b10.c10.d10.e10)
      r.getUInt64(&ok);
      if (!ok) return false;
    }

    // LC_MAIN
    else if (type == (0x28 | 0x80000000)) {
      // File (__TEXT) offset of main()
      r.getUInt64(&ok);
      if (!ok) return false;

      // Initial stack size if not zero.
      r.getUInt64(&ok);
      if (!ok) return false;
    }

    // LC_FUNCTION_STARTS, LC_DYLIB_CODE_SIGN_DRS, LC_SEGMENT_SPLIT_INFO, LC_CODE_SIGNATURE,
    // LC_LINKER_OPTIMIZATION_HINT, LC_DYLD_EXPORTS_TRIE, or LC_DYLD_CHAINED_FIXUPS
    else if (type == 0x26 || type == 0x2B || type == 0x1E || type == 0x1D || type == 0x2E ||
             type == (0x33 | 0x80000000) || type == (0x34 | 0x80000000)) {
      // File offset to data in __LINKEDIT segment.
      quint32 off = r.getUInt32(&ok);
      if (!ok) return false;

      // File size of data in __LINKEDIT segment.
      quint32 siz = r.getUInt32(&ok);
      if (!ok) return false;

      // LC_FUNCTION_STARTS
      if (type == 0x26) {
        auto sec = std::make_unique<Section>(
          Section::Type::FUNC_STARTS, QObject::tr("Function Starts"), off, siz, offset + off);
        binaryObject->addSection(std::move(sec));
      }

      // LC_CODE_SIGNATURE
      else if (type == 0x1D) {
        auto sec = std::make_unique<Section>(Section::Type::CODE_SIG, QObject::tr("Code Signature"),
                                             off, siz, offset + off);
        binaryObject->addSection(std::move(sec));
      }
    }

    // LC_DATA_IN_CODE
    else if (type == 0x29) {
      // From mach_header to start of data range.
      r.getUInt32(&ok);
      if (!ok) return false;

      // Number of bytes in data range.
      r.getUInt16(&ok);
      if (!ok) return false;

      // Dice kind value.
      r.getUInt16(&ok);
      if (!ok) return false;
    }

    // LC_THREAD or LC_UNIXTHREAD
    else if (type == 0x4 || type == 0x5) {
      quint32 flavor = r.getUInt32(&ok);
      if (!ok) return false;

      quint32 count = r.getUInt32(&ok);
      if (!ok) return false;

      // Data.
      r.read(flavor * count);
    }

    // LC_RPATH
    else if (type == 0x1C + 0x80000000) {
      // Name offset.
      quint32 off = r.getUInt32(&ok);
      if (!ok) return false;

      // Name.
      r.read(cmdsize - off);
    }

    // LC_SUB_CLIENT, LC_SUB_UMBRELLA, LC_SUB_FRAMEWORK or LC_SUB_LIBRARY
    else if (type == 0x12 || type == 0x13 || type == 0x14 || type == 0x15) {
      r.read(cmdsize);
    }

    // LC_LOADFVMLIB
    else if (type == 0x6) {
      r.read(cmdsize);
    }

    // LC_IDFVMLIB
    else if (type == 0x7) {
      r.read(cmdsize);
    }

    // LC_IDENT
    else if (type == 0x8) {
      r.read(cmdsize);
    }

    // LC_FVMFILE
    else if (type == 0x9) {
      r.read(cmdsize);
    }

    // LC_PREPAGE
    else if (type == 0xA) {
      r.read(cmdsize);
    }

    // LC_PREBOUND_DYLIB
    else if (type == 0x10) {
      r.read(cmdsize);
    }

    // LC_ROUTINES or LC_ROUTINES_64
    else if (type == 0x11 || type == 0x1A) {
      r.read(cmdsize);
    }

    // LC_TWOLEVEL_HINTS
    else if (type == 0x16) {
      r.read(cmdsize);
    }

    // LC_PREBIND_CKSUM
    else if (type == 0x17) {
      r.read(cmdsize);
    }

    // LC_ENCRYPTION_INFO or LC_ENCRYPTION_INFO_64
    else if (type == 0x21 || type == 0x2C) {
      r.read(cmdsize);
    }

    // LC_LINKER_OPTION
    else if (type == 0x2D) {
      r.read(cmdsize);
    }

    // LC_NOTE
    else if (type == 0x31) {
      r.read(cmdsize);
    }

    // LC_BUILD_VERSION
    else if (type == 0x32) {
      r.read(cmdsize);
    }

    else {
      qWarning() << "What is type=" << type << "Ignoring!";
    }
  }

  // Parse symbol table if found.
  // (/usr/include/macho/nlist.h)
  quint32 symsize{0};
  SymbolTable symTable;
  if (symnum > 0) {
    r.seek(symoff);
    qint64 pos;
    for (decltype(symnum) i = 0; i < symnum; i++) {
      pos = r.pos();

      // Index into the string table.
      quint32 index = r.getUInt32(&ok);
      if (!ok) return false;

      // Type flag.
      r.getUChar(&ok);
      if (!ok) return false;

      // Section number or NO_SECT.
      r.getUChar(&ok);
      if (!ok) return false;

      // Description.
      r.getUInt16(&ok);
      if (!ok) return false;

      // Value of the symbol (or stab offset).
      quint64 value;
      if (systemBits == 32) {
        value = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        value = r.getUInt64(&ok);
        if (!ok) return false;
      }

      symTable.addSymbol(SymbolEntry(index, value));
      symsize += (r.pos() - pos);
    }

    auto sec = std::make_unique<Section>(Section::Type::SYMBOLS, QObject::tr("Symbol Table"),
                                         symoff, symsize, offset + symoff);
    binaryObject->addSection(std::move(sec));
  }

  // Parse dynamic symbol table if found. Store the offsets into the
  // symbol table for later updating.
  quint32 dynsymsize{0};
  SymbolTable dynsymTable;
  if (indirsymnum > 0) {
    r.seek(indirsymoff);
    qint64 pos;
    for (decltype(indirsymnum) i = 0; i < indirsymnum; i++) {
      pos = r.pos();

      quint32 num = r.getUInt32(&ok);
      if (!ok) return false;

      dynsymTable.addSymbol(SymbolEntry(num, 0));
      dynsymsize += (r.pos() - pos);
    }

    auto sec =
      std::make_unique<Section>(Section::Type::DYN_SYMBOLS, QObject::tr("Dynamic Symbol Table"),
                                indirsymoff, dynsymsize, offset + indirsymoff);
    binaryObject->addSection(std::move(sec));
  }

  // Fill data of stored sections.
  for (auto &sec : binaryObject->sections()) {
    r.seek(sec->offset());
    sec->setData(r.read(sec->size()));
  }

  // If symbol table loaded then merge string table entries into it.
  if (symnum > 0) {
    auto strTable = binaryObject->section(Section::Type::STRING);
    if (strTable) {
      auto &data = strTable->data();
      auto &symbols = symTable.symbols();
      for (std::size_t h = 0; h < symbols.size(); h++) {
        auto &symbol = symbols[h];
        QByteArray tmp;
        for (int i = symbol.index(); i < data.size(); i++) {
          char c = data[i];
          tmp += c;
          if (c == 0) break;
        }
        symbol.setString(QString::fromUtf8(tmp));
      }
    }

    // This table isn't moved because it is still needed below.
    binaryObject->setSymbolTable(symTable);
  }

  // If dynamic symbol table loaded then merge data from symbol table and symbol stubs into it.
  if (indirsymnum > 0 && symnum > 0) {
    auto stubs = binaryObject->section(Section::Type::SYMBOL_STUBS);
    if (stubs) {
      quint64 stubAddr = stubs->address();
      const auto &symbols = symTable.symbols();
      auto &dynsymbols = dynsymTable.symbols();
      for (std::size_t h = 0; h < dynsymbols.size(); h++) {
        auto &symbol = dynsymbols[h];
        auto idx = symbol.index();
        if (idx < symnum) {
          // The index corresponds to the index in the symbol table.
          symbol.setString(symbols[idx].string());

          // Each symbol stub takes up 6 bytes.
          symbol.setValue(stubAddr + h * 6);
        }
      }
    }
    binaryObject->setDynSymbolTable(std::move(dynsymTable));
  }

  objects_.emplace_back(std::move(binaryObject));
  return true;
}

} // namespace dispar
