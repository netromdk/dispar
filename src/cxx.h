#ifndef DISPAR_CXX_H
#define DISPAR_CXX_H

#include <algorithm>
#include <iterator>

namespace cxx {

/// When a member function has muliple overloads and you need to use just one of them.
/** Example:
    connect(cpuTypeBox, Use<int>::overloadOf(&QComboBox::currentIndexChanged), this,
            &DisassemblerDialog::onCpuTypeIndexChanged);
    */
template <typename... Args>
struct Use {
  template <typename Cls, typename Ret>
  static auto overloadOf(Ret (Cls::*MembFunc)(Args...))
  {
    return MembFunc;
  }
};

/// Adding ranged wrappers for std functions.

template <typename Container>
void sort(Container &container)
{
  std::sort(std::begin(container), std::end(container));
}

template <typename Container, typename Compare>
void sort(Container &container, Compare cmp)
{
  std::sort(std::begin(container), std::end(container), cmp);
}

template <typename Container, typename OutputIt>
OutputIt move(Container &src, OutputIt dstFirst)
{
  return std::move(std::begin(src), std::end(src), dstFirst);
}

template <typename Container, typename OutputIt>
OutputIt copy(const Container &src, OutputIt dstFirst)
{
  return std::copy(std::cbegin(src), std::cend(src), dstFirst);
}

} // namespace cxx

#endif // DISPAR_CXX_H
