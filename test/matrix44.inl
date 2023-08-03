#include "spike/type/matrix44.hpp"
#include "spike/util/unit_testing.hpp"

int test_matrix44_00() {
  es::Matrix44 mtx({1, 2, 3, 0}, {5, 6, 7, 0}, {9, 10, 11, 0},
                   {13, 14, 15, 16});
  mtx.Transpose();

  TEST_EQUAL(mtx.r1(), Vector4A16(1, 5, 9, 0));
  TEST_EQUAL(mtx.r2(), Vector4A16(2, 6, 10, 0));
  TEST_EQUAL(mtx.r3(), Vector4A16(3, 7, 11, 0));
  TEST_EQUAL(mtx.r4(), Vector4A16(13, 14, 15, 16));

  mtx.MakeIdentity();

  TEST_EQUAL(mtx.r1(), Vector4A16(1, 0, 0, 0));
  TEST_EQUAL(mtx.r2(), Vector4A16(0, 1, 0, 0));
  TEST_EQUAL(mtx.r3(), Vector4A16(0, 0, 1, 0));
  TEST_EQUAL(mtx.r4(), Vector4A16(0, 0, 0, 1));

  Vector4A16 qt, pos, scale;

  mtx.Decompose(pos, qt, scale);

  TEST_EQUAL(qt, Vector4A16(0, 0, 0, 1));
  TEST_EQUAL(pos, Vector4A16(0, 0, 0, 1));
  TEST_EQUAL(scale, Vector4A16(1, 1, 1, 0));

  pos = Vector4A16(10, 20, 30, 1);
  scale = Vector4A16(2, 3, 4, 0);

  mtx.Compose(pos, qt, scale);

  TEST_EQUAL(mtx.r1(), Vector4A16(2, 0, 0, 0));
  TEST_EQUAL(mtx.r2(), Vector4A16(0, 3, 0, 0));
  TEST_EQUAL(mtx.r3(), Vector4A16(0, 0, 4, 0));
  TEST_EQUAL(mtx.r4(), Vector4A16(10, 20, 30, 1));

  return 0;
}

int test_matrix44_01() {
  Vector4A16 qt(-0.3062f, 0.1768f, -0.3062f, 0.8839f);
  es::Matrix44 mtx(qt);
  mtx.Transpose();
  Vector4A16::SetEpsilon(0.001f);

  TEST_EQUAL(mtx.r1(), Vector4A16(0.7497f, 0.4329f, 0.5003f, 0));
  TEST_EQUAL(mtx.r2(), Vector4A16(-0.6497f, 0.6247f, 0.4329f, 0));
  TEST_EQUAL(mtx.r3(), Vector4A16(-0.1251f, -0.6497f, 0.7497f, 0));
  TEST_EQUAL(mtx.r4(), Vector4A16(0, 0, 0, 1));

  return 0;
}

int test_matrix44_02() {
  es::Matrix44 mtx(Vector4A16(-0.5797f, 0.0955f, -0.53f, 0.6115f));

  auto rotated = Vector4A16{1.f, 1.f, 1.f, 1.f} * mtx;
  TEST_EQUAL(rotated, Vector4A16(0.1587f, -0.5066f, 1.6486f, 1.f));

  mtx = es::Matrix44({0.353041, 0.418703, 0.83669, 0.f},
                     {-0.933951, 0.21091, 0.288535, 0.f},
                     {-0.0556561, -0.883292, 0.465508, 0.f},
                     {-19.9345, 20.1798, -7.52409, 1.f});
  return 0;
}
