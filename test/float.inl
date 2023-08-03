#include "spike/type/float.hpp"
#include "spike/util/unit_testing.hpp"
#include <cmath>

int test_float_00() {
  const uint16 num0 = 0x62a7; // 851.5
  const uint16 num1 = 0x1f;   // denormal, 1.847744e-06
  const uint16 num2 = 0x7c00; // inf
  const uint16 num3 = 0x7c01; // nan
  const uint16 num4 = 0xe2a7; // -851.5

  TEST_EQUAL(float16_codec::ToFloat(num0), 851.5f);
  auto flt0 = 1.847744e-06f;
  TEST_EQUAL(float16_codec::ToFloat(num1), flt0);
  TEST_CHECK(std::isinf(float16_codec::ToFloat(num2)));
  TEST_CHECK(std::isnan(float16_codec::ToFloat(num3)));
  TEST_EQUAL(float16_codec::ToFloat(num4), -851.5f);

  const Vector4A16 rst =
      float16_codec::ToVector4(IVector4A16(num0, num1, num2, num4));

  TEST_EQUAL(rst.X, 851.5f);
  TEST_EQUAL(rst.Y, 1.847744e-06f);
  TEST_CHECK(std::isinf(rst.Z));
  TEST_EQUAL(rst.W, -851.5f);

  return 0;
}

int test_float_01() {
  const float num0 = 851.5f;
  const float num1 = 1.847744e-06f;
  const float num2 = INFINITY;
  const uint32 _num3 = 0x7fffffff;
  const float num3 = reinterpret_cast<const float &>(_num3);
  const float num4 = -851.5f;

  TEST_EQUAL(float16_codec::FromFloat(num0), 0x62a7);
  TEST_EQUAL(float16_codec::FromFloat(num1), 0x1f);
  TEST_EQUAL(float16_codec::FromFloat(num2), 0x7c00);
  TEST_EQUAL(float16_codec::FromFloat(num3), 0x7fff);
  TEST_EQUAL(float16_codec::FromFloat(num4), 0xe2a7);

  const IVector4A16 rst =
      float16_codec::FromVector4(Vector4A16(num0, num1, num2, num4));

  TEST_EQUAL(rst.X, 0x62a7);
  TEST_EQUAL(rst.Y, 0x1f);
  TEST_EQUAL(rst.Z, 0x7c00);
  TEST_EQUAL(rst.W, 0xe2a7);

  return 0;
}
