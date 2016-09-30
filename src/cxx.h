#ifndef DISPAR_CXX_H
#define DISPAR_CXX_H

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

#endif // DISPAR_CXX_H
