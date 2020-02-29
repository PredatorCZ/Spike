#include "../datas/disabler.hpp"
#include "../datas/unit_testing.hpp"
#include <memory>

struct class01 {
  const int iType = 12;
  const float fType = 1.5f;
};

struct class02 {
  const float fType = 1.1f;
  void noIType();
};

struct class03 {
  const int iType = 10;
  void noFType();
};

struct class04 {
  void noFType();
  void noIType();
};

struct ImasterClass {
  virtual int Func01() = 0;
  virtual float Func02() = 0;
  virtual ~ImasterClass() {}
};

template <class _parent> struct masterClass_t : _parent {
  ADD_DISABLERS(_parent, noIType, noFType);

  enabledFunction(noIType, int) Func01() { return this->iType; }
  disabledFunction(noIType, int) Func01() { return -1; }

  enabledFunction(noFType, float) Func02() { return this->fType; }
  disabledFunction(noFType, float) Func02() { return -1.8f; }
};

template <class _Ty> struct masterClass : ImasterClass {
  typedef masterClass_t<_Ty> typuh;
  typuh data;

  masterClass() {}

  int Func01() override { return data.Func01(); }
  float Func02() override { return data.Func02(); }
};

int test_disabler() {
  std::unique_ptr<ImasterClass> cls01(new masterClass<class01>);
  std::unique_ptr<ImasterClass> cls02(new masterClass<class02>);
  std::unique_ptr<ImasterClass> cls03(new masterClass<class03>);
  std::unique_ptr<ImasterClass> cls04(new masterClass<class04>);

  TEST_EQUAL(cls01->Func01(), 12);
  TEST_EQUAL(cls01->Func02(), 1.5f);

  TEST_EQUAL(cls02->Func01(), -1);
  TEST_EQUAL(cls02->Func02(), 1.1f);

  TEST_EQUAL(cls03->Func01(), 10);
  TEST_EQUAL(cls03->Func02(), -1.8f);

  TEST_EQUAL(cls04->Func01(), -1);
  TEST_EQUAL(cls04->Func02(), -1.8f);

  return 0;
}