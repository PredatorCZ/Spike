#include "../datas/vectors_simd.hpp"
#include "../datas/unit_testing.hpp"

int test_vector_simd_00() {
  Vector4A16 vc0;
  Vector4A16 vc1(2.f);
  Vector4A16 vc2(1.f, -2.f, 3.f, 4.f);
  Vector4A16 vc3(Vector(5.f, 6.f, 7.f), 8.f);
  Vector4A16 vc4(vc1._data);
  Vector4A16 vc5(Vector4(9.f, 10.f, 11.f, 12.f));

  TEST_EQUAL(vc0.X, 0);
  TEST_EQUAL(vc0.Y, 0);
  TEST_EQUAL(vc0.Z, 0);
  TEST_EQUAL(vc0.W, 0);

  TEST_EQUAL(vc1.X, 2.f);
  TEST_EQUAL(vc1.Y, 2.f);
  TEST_EQUAL(vc1.Z, 2.f);
  TEST_EQUAL(vc1.W, 2.f);

  TEST_EQUAL(vc2.X, 1.f);
  TEST_EQUAL(vc2.Y, -2.f);
  TEST_EQUAL(vc2.Z, 3.f);
  TEST_EQUAL(vc2.W, 4.f);

  TEST_EQUAL(vc3.X, 5.f);
  TEST_EQUAL(vc3.Y, 6.f);
  TEST_EQUAL(vc3.Z, 7.f);
  TEST_EQUAL(vc3.W, 8.f);

  TEST_EQUAL(vc4.X, 2.f);
  TEST_EQUAL(vc4.Y, 2.f);
  TEST_EQUAL(vc4.Z, 2.f);
  TEST_EQUAL(vc4.W, 2.f);

  TEST_EQUAL(vc5.X, 9.f);
  TEST_EQUAL(vc5.Y, 10.f);
  TEST_EQUAL(vc5.Z, 11.f);
  TEST_EQUAL(vc5.W, 12.f);

  TEST_EQUAL(vc1, vc4);
  TEST_NOT_EQUAL(vc4, vc5);

  TEST_CHECK(vc1.IsSymetrical());
  TEST_NOT_CHECK(vc2.IsSymetrical());

  TEST_EQUAL(vc2.Sign(), -1.f);
  TEST_EQUAL(vc3.Sign(), 1.f);

  return 0;
}

int test_vector_simd_01() {
  Vector4A16 vc0(1.f, 2.f, 3.f, 4.f);
  Vector4A16 vc1(4.f, 3.f, 2.f, 1.f);
  Vector4A16 vc2 = vc0 + vc1;

  TEST_EQUAL(vc2, 5.f);

  Vector4A16 vc3 = vc0 - vc1;

  TEST_EQUAL(vc3, Vector4A16(-3.f, -1.f, 1.f, 3.f));

  Vector4A16 vc4 = vc0 * vc1;

  TEST_EQUAL(vc4, Vector4A16(4.f, 6.f, 6.f, 4.f));

  Vector4A16 vc5 = vc0 / vc1;

  TEST_EQUAL(vc5, Vector4A16(1.f / 4.f, 2.f / 3.f, 3.f / 2.f, 4.f / 1.f));

  Vector4A16 vc6 = vc0 + 1.f;

  TEST_EQUAL(vc6, Vector4A16(2.f, 3.f, 4.f, 5.f));

  Vector4A16 vc7 = vc0 - 1.f;

  TEST_EQUAL(vc7, Vector4A16(0.f, 1.f, 2.f, 3.f));

  Vector4A16 vc8 = vc0 * 10.f;

  TEST_EQUAL(vc8, Vector4A16(10.f, 20.f, 30.f, 40.f));

  Vector4A16 vc9 = vc0 / 10.f;

  TEST_EQUAL(vc9, Vector4A16(.1f, .2f, .3f, .4f));

  vc9 *= 10.f;

  TEST_EQUAL(vc9, vc0);

  vc8 /= 10.f;

  TEST_EQUAL(vc8, vc0);

  vc7 += 1.f;

  TEST_EQUAL(vc7, vc0);

  vc6 -= 1.f;

  TEST_EQUAL(vc6, vc0);

  vc5 *= vc1;

  TEST_EQUAL(vc5, vc0);

  vc4 /= vc1;

  TEST_EQUAL(vc4, vc0);

  vc3 += vc1;

  TEST_EQUAL(vc3, vc0);

  vc2 -= vc1;

  TEST_EQUAL(vc2, vc0);

  Vector4A16 vc10 = -vc0;

  TEST_EQUAL(vc10, Vector4A16(-1.f, -2.f, -3.f, -4.f));

  return 0;
}

int test_vector_simd_02() {
  Vector4A16 vc0(0.125f);
  Vector4A16 vc1(0.0625f);

  Vector4A16 vc2 = vc0 | vc1;

  TEST_EQUAL(vc2, 1.f);

  Vector4A16 vc3 = vc0 & vc1;

  TEST_EQUAL(vc3, 0.0078125f);

  Vector4A16 vc4(2048.f);
  Vector4A16 vc5(1.26217744835e-29f);

  Vector4A16 vc6 = vc4 ^ vc5;

  TEST_EQUAL(vc6, 4194304.0);

  Vector4A16 vc7(2.f);
  Vector4A16 vc8 = ~vc7;

  TEST_EQUAL(vc8, -1.99999988079f);

  return 0;
}

int test_vector_simd_03() {
  Vector4A16 vc0(1.f, 2.f, 3.f, 4.f);
  const float vc0Len = vc0.Length();

  TEST_EQUAL(vc0Len, 5.477226f);

  vc0.Normalize();

  TEST_EQUAL(vc0, Vector4A16(0.182574f, 0.365148f, 0.547723f, 0.730297f));

  Vector4A16 vc1(0.125f);
  Vector4A16 vc2 = vc1.QConjugate();

  TEST_EQUAL(vc2, Vector4A16(-0.125f, -0.125f, -0.125f, 0.125f));

  Vector4A16 vc3(0, 0.707107f, 0, 0);
  Vector4A16 vc4 = vc3.QComputeElement();

  TEST_EQUAL(vc4, Vector4A16(0, 0.707107f, 0, 0.707107f));

  Vector4A16 vc5(0, 0, 0, 0.707107f);
  Vector4A16 vc6 = vc5.QComputeElement<1>();

  TEST_EQUAL(vc6, Vector4A16(0, 0.707107f, 0, 0.707107f));

  return 0;
}

int test_vector_simd_10() {
  IVector4A16 vc0;
  IVector4A16 vc1(2);
  IVector4A16 vc2(1, -2, 3, 4);
  UIVector4A16 vc3(vc2);
  IVector4A16 vc4(vc1);
  IVector4A16 vc5(IVector4(9, 10, 11, 12));

  TEST_EQUAL(vc0.X, 0);
  TEST_EQUAL(vc0.Y, 0);
  TEST_EQUAL(vc0.Z, 0);
  TEST_EQUAL(vc0.W, 0);

  TEST_EQUAL(vc1.X, 2);
  TEST_EQUAL(vc1.Y, 2);
  TEST_EQUAL(vc1.Z, 2);
  TEST_EQUAL(vc1.W, 2);

  TEST_EQUAL(vc2.X, 1);
  TEST_EQUAL(vc2.Y, -2);
  TEST_EQUAL(vc2.Z, 3);
  TEST_EQUAL(vc2.W, 4);

  TEST_EQUAL(vc3.X, 1);
  TEST_EQUAL(vc3.Y, 0xfffffffe);
  TEST_EQUAL(vc3.Z, 3);
  TEST_EQUAL(vc3.W, 4);

  TEST_EQUAL(vc4.X, 2);
  TEST_EQUAL(vc4.Y, 2);
  TEST_EQUAL(vc4.Z, 2);
  TEST_EQUAL(vc4.W, 2);

  TEST_EQUAL(vc5.X, 9);
  TEST_EQUAL(vc5.Y, 10);
  TEST_EQUAL(vc5.Z, 11);
  TEST_EQUAL(vc5.W, 12);

  TEST_EQUAL(vc1, vc4);
  TEST_NOT_EQUAL(vc4, vc5);

  return 0;
}

int test_vector_simd_11() {
  IVector4A16 vc0(1, 2, 3, 4);
  IVector4A16 vc1(4, 3, 2, 1);
  IVector4A16 vc2 = vc0 + vc1;

  TEST_EQUAL(vc2, 5);

  IVector4A16 vc3 = vc0 - vc1;

  TEST_EQUAL(vc3, IVector4A16(-3, -1, 1, 3));

  IVector4A16 vc4 = vc0 * vc1;

  TEST_EQUAL(vc4, IVector4A16(4, 6, 6, 4));

  IVector4A16 vc6 = vc0 + 1;

  TEST_EQUAL(vc6, IVector4A16(2, 3, 4, 5));

  IVector4A16 vc7 = vc0 - 1;

  TEST_EQUAL(vc7, IVector4A16(0, 1, 2, 3));

  IVector4A16 vc8 = vc0 * 10;

  TEST_EQUAL(vc8, IVector4A16(10, 20, 30, 40));

  vc8 *= 2;

  TEST_EQUAL(vc8, IVector4A16(20, 40, 60, 80));

  vc7 += 1;

  TEST_EQUAL(vc7, vc0);

  vc6 -= 1;

  TEST_EQUAL(vc6, vc0);

  vc3 += vc1;

  TEST_EQUAL(vc3, vc0);

  vc2 -= vc1;

  TEST_EQUAL(vc2, vc0);

  return 0;
}

int test_vector_simd_12() {
  IVector4A16 vc0(3);
  IVector4A16 vc1(5);

  IVector4A16 vc2 = vc0 | vc1;

  TEST_EQUAL(vc2, 7);

  IVector4A16 vc3 = vc0 & vc1;

  TEST_EQUAL(vc3, 1);

  IVector4A16 vc6 = vc0 ^ vc1;

  TEST_EQUAL(vc6, 6);

  IVector4A16 vc8 = ~vc0;

  TEST_EQUAL(vc8, 0xfffffffc);

  IVector4A16 vc9 = vc8 >> 2;

  TEST_EQUAL(vc9, 0xffffffff);

  IVector4A16 vc10 = UIVector4A16(vc8) >> 2;

  TEST_EQUAL(vc10, 0x3fffffff);

  IVector4A16 vc11 = vc0 << 2;

  TEST_EQUAL(vc11, 0xc);

  return 0;
}