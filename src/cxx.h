#ifndef DISPAR_CXX_H
#define DISPAR_CXX_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>

#ifndef NDEBUG
#define ASSERT_X(cond, what)                                                                       \
  ((cond) ? static_cast<void>(0) : dispar::cxx::assert_x(__FUNCTION__, what, __FILE__, __LINE__))
#else
#define ASSERT_X(cond, what) static_cast<void>(0)
#endif

namespace dispar {
namespace cxx {

static inline void assert_x(const char *where, const char *what, const char *file, int line)
{
  std::cerr << "ASSERT failure in " << where << ": " << what << ", file " << file << ", line "
            << line << std::endl;
  std::abort();
}

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

template <typename Container, typename OutputIt, typename UnaryPredicate>
OutputIt copy_if(const Container &src, OutputIt dstFirst, UnaryPredicate pred)
{
  return std::copy_if(std::cbegin(src), std::cend(src), dstFirst, pred);
}

template <typename Container, typename UnaryPredicate>
bool any_of(const Container &src, UnaryPredicate pred)
{
  return std::any_of(std::cbegin(src), std::cend(src), pred);
}

template <typename Container, typename UnaryPredicate>
auto find_if(const Container &src, UnaryPredicate pred)
{
  return std::find_if(std::cbegin(src), std::cend(src), pred);
}

} // namespace cxx
} // namespace dispar

#endif // DISPAR_CXX_H
