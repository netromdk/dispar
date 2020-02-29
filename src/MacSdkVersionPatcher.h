#ifndef SRC_MACSDKVERSIONPATCHER_H
#define SRC_MACSDKVERSIONPATCHER_H

#include <tuple>

namespace dispar {

class Section;

class MacSdkVersionPatcher {
public:
  using Version = std::tuple<int, int>;

  MacSdkVersionPatcher(Section &section);

  bool valid() const;

  Version target() const;
  bool setTarget(const Version &version);

  Version sdk() const;
  bool setSdk(const Version &version);

private:
  void parse();
  void patch(const Version &newVersion, int pos);

  Section &section;
  bool valid_ = false;
  Version target_, sdk_;
};

} // namespace dispar

#endif // SRC_MACSDKVERSIONPATCHER_H
