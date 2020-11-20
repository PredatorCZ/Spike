#include "../datas/unit_testing.hpp"
#include "../uni/format.hpp"

using namespace uni;

int test_format_00() {
  auto &&cdec = FormatCodec::Get({FormatType::FLOAT, DataType::R32G32B32A32});
  Vector4 vc0(12.f, 4.85f, 6.75f, -9.14f);
  Vector4A16 out;

  TEST_THROW(std::runtime_error, {
    IVector4A16 outWrong;
    cdec.GetValue(outWrong, nullptr);
  });

  TEST_THROW(std::runtime_error, {
    uni::FormatCodec::ivec outWrong;
    cdec.Sample(outWrong, nullptr, 1);
  });

  uni::FormatCodec::fvec outVec(1);

  cdec.Sample(outVec, reinterpret_cast<const char *>(&vc0), 0);

  TEST_EQUAL(outVec[0], Vector4A16(12.f, 4.85f, 6.75f, -9.14f));

  cdec.Sample(outVec, reinterpret_cast<const char *>(&vc0), 1);

  TEST_EQUAL(outVec[0], Vector4A16(12.f, 4.85f, 6.75f, -9.14f));

  TEST_THROW(std::runtime_error, {
    cdec.Sample(outVec, reinterpret_cast<const char *>(&vc0), 1, 1);
  });

  cdec.GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(12.f, 4.85f, 6.75f, -9.14f));

  FormatCodec::Get({FormatType::FLOAT, DataType::R32G32B32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(12.f, 4.85f, 6.75f, 0));

  FormatCodec::Get({FormatType::FLOAT, DataType::R32G32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(12.f, 4.85f, 0, 0));

  FormatCodec::Get({FormatType::FLOAT, DataType::R32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(12.f, 0, 0, 0));

  USVector4 vc1(0x62a7, 0x1f, 0x7c00, 0xe2a7);

  FormatCodec::Get({FormatType::FLOAT, DataType::R16G16B16A16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(851.5f, 1.847744399e-06f, INFINITY, -851.5f));

  FormatCodec::Get({FormatType::FLOAT, DataType::R16G16B16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(851.5f, 1.847744399e-06f, INFINITY, 0));

  FormatCodec::Get({FormatType::FLOAT, DataType::R16G16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(851.5f, 1.847744399e-06f, 0, 0));

  FormatCodec::Get({FormatType::FLOAT, DataType::R16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(851.5f, 0, 0, 0));

  // TODO: R11G11B10

  return 0;
}

int test_format_01() {
  auto &&cdec = FormatCodec::Get({FormatType::UINT, DataType::R32G32B32A32});
  UIVector4 vc0(123456, 7891011, 1213141516, 1718192021);
  IVector4A16 out;

  TEST_THROW(std::runtime_error, {
    Vector4A16 outWrong;
    cdec.GetValue(outWrong, nullptr);
  });

  TEST_THROW(std::runtime_error, {
    uni::FormatCodec::fvec outWrong;
    cdec.Sample(outWrong, nullptr, 1);
  });

  uni::FormatCodec::ivec outVec(1);

  cdec.Sample(outVec, reinterpret_cast<const char *>(&vc0), 0);

  TEST_EQUAL(outVec[0], IVector4A16(123456, 7891011, 1213141516, 1718192021));

  cdec.Sample(outVec, reinterpret_cast<const char *>(&vc0), 1);

  TEST_EQUAL(outVec[0], IVector4A16(123456, 7891011, 1213141516, 1718192021));

  TEST_THROW(std::runtime_error, {
    cdec.Sample(outVec, reinterpret_cast<const char *>(&vc0), 1, 1);
  });

  FormatCodec::Get({FormatType::UINT, DataType::R32G32B32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, IVector4A16(123456, 7891011, 1213141516, 0));

  FormatCodec::Get({FormatType::UINT, DataType::R32G32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, IVector4A16(123456, 7891011, 0, 0));

  FormatCodec::Get({FormatType::UINT, DataType::R32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, IVector4A16(123456, 0, 0, 0));

  USVector4 vc1(12345, 6789, 10112, 65432);

  FormatCodec::Get({FormatType::UINT, DataType::R16G16B16A16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, 6789, 10112, 65432));

  FormatCodec::Get({FormatType::UINT, DataType::R16G16B16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, 6789, 10112, 0));

  FormatCodec::Get({FormatType::UINT, DataType::R16G16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, 6789, 0, 0));

  FormatCodec::Get({FormatType::UINT, DataType::R16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, 0, 0, 0));

  UCVector4 vc2(250, 120, 5, 67);

  FormatCodec::Get({FormatType::UINT, DataType::R8G8B8A8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(250, 120, 5, 67));

  FormatCodec::Get({FormatType::UINT, DataType::R8G8B8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(250, 120, 5, 0));

  FormatCodec::Get({FormatType::UINT, DataType::R8G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(250, 120, 0, 0));

  FormatCodec::Get({FormatType::UINT, DataType::R8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(250, 0, 0, 0));

  union {
    int32 r;
    struct {
      uint8 _n[3];
      uint8 g;
    } g;
  } vc3;

  vc3.r = 12345678;
  vc3.g.g = 158;

  FormatCodec::Get({FormatType::UINT, DataType::R24G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc3));

  TEST_EQUAL(out, IVector4A16(12345678, 158, 0, 0));

  uint32 vc4 = 999 | (725 << 11) | (482 << 22);

  FormatCodec::Get({FormatType::UINT, DataType::R11G11B10})
      .GetValue(out, reinterpret_cast<const char *>(&vc4));

  TEST_EQUAL(out, IVector4A16(999, 725, 482, 0));

  uint32 vc5 = 510 | (326 << 10) | (290 << 20) | (2U << 30);

  FormatCodec::Get({FormatType::UINT, DataType::R10G10B10A2})
      .GetValue(out, reinterpret_cast<const char *>(&vc5));

  TEST_EQUAL(out, IVector4A16(510, 326, 290, 2));

  uint16 vc6 = 30 | (20 << 5) | (10 << 11);

  FormatCodec::Get({FormatType::UINT, DataType::R5G6B5})
      .GetValue(out, reinterpret_cast<const char *>(&vc6));

  TEST_EQUAL(out, IVector4A16(30, 20, 10, 0));

  uint16 vc7 = 29 | (19 << 5) | (9 << 10) | (1 << 15);

  FormatCodec::Get({FormatType::UINT, DataType::R5G5B5A1})
      .GetValue(out, reinterpret_cast<const char *>(&vc7));

  TEST_EQUAL(out, IVector4A16(29, 19, 9, 1));

  return 0;
}

int test_format_02() {
  auto &&cdec = FormatCodec::Get({FormatType::INT, DataType::R32G32B32A32});
  IVector4 vc0(-123456, 7891011, -1213141516, 1718192021);
  IVector4A16 out;

  cdec.GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, IVector4A16(-123456, 7891011, -1213141516, 1718192021));

  FormatCodec::Get({FormatType::INT, DataType::R32G32B32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, IVector4A16(-123456, 7891011, -1213141516, 0));

  FormatCodec::Get({FormatType::INT, DataType::R32G32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, IVector4A16(-123456, 7891011, 0, 0));

  FormatCodec::Get({FormatType::INT, DataType::R32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, IVector4A16(-123456, 0, 0, 0));

  SVector4 vc1(12345, -6789, 10112, -32109);

  FormatCodec::Get({FormatType::INT, DataType::R16G16B16A16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, -6789, 10112, -32109));

  FormatCodec::Get({FormatType::INT, DataType::R16G16B16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, -6789, 10112, 0));

  FormatCodec::Get({FormatType::INT, DataType::R16G16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, -6789, 0, 0));

  FormatCodec::Get({FormatType::INT, DataType::R16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, IVector4A16(12345, 0, 0, 0));

  CVector4 vc2(-120, -62, 5, 67);

  FormatCodec::Get({FormatType::INT, DataType::R8G8B8A8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(-120, -62, 5, 67));

  FormatCodec::Get({FormatType::INT, DataType::R8G8B8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(-120, -62, 5, 0));

  FormatCodec::Get({FormatType::INT, DataType::R8G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(-120, -62, 0, 0));

  FormatCodec::Get({FormatType::INT, DataType::R8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, IVector4A16(-120, 0, 0, 0));

  union {
    int32 r;
    struct {
      int8 _n[3];
      int8 g;
    } g;
  } vc3;

  vc3.r = -1234567;
  vc3.g.g = -108;

  FormatCodec::Get({FormatType::INT, DataType::R24G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc3));

  TEST_EQUAL(out, IVector4A16(-1234567, -108, 0, 0));

  int32 vc4 = (-500 & 0x7ff) | ((-420 & 0x7ff) << 11) | ((-10 & 0x3ff) << 22);

  FormatCodec::Get({FormatType::INT, DataType::R11G11B10})
      .GetValue(out, reinterpret_cast<const char *>(&vc4));

  TEST_EQUAL(out, IVector4A16(-500, -420, -10, 0));

  int32 vc5 = (-250 & 0x3ff) | ((-110 & 0x3ff) << 10) | ((-50 & 0x3ff) << 20) |
              (-2 << 30);

  FormatCodec::Get({FormatType::INT, DataType::R10G10B10A2})
      .GetValue(out, reinterpret_cast<const char *>(&vc5));

  TEST_EQUAL(out, IVector4A16(-250, -110, -50, -2));

  int16 vc6 = (-15 & 0x1f) | ((-10 & 0x3f) << 5) | ((-5 & 0x1f) << 11);

  FormatCodec::Get({FormatType::INT, DataType::R5G6B5})
      .GetValue(out, reinterpret_cast<const char *>(&vc6));

  TEST_EQUAL(out, IVector4A16(-15, -10, -5, 0));

  int16 vc7 =
      (-14 & 0x1f) | ((-9 & 0x1f) << 5) | ((-6 & 0x1f) << 10) | (-1 << 15);

  FormatCodec::Get({FormatType::INT, DataType::R5G5B5A1})
      .GetValue(out, reinterpret_cast<const char *>(&vc7));

  TEST_EQUAL(out, IVector4A16(-14, -9, -6, -1));

  return 0;
}

int test_format_03() {
  auto &&cdec = FormatCodec::Get({FormatType::UNORM, DataType::R32G32B32A32});
  UIVector4 vc0(123456, 7891011, 1213141516, 1718192021);
  Vector4A16 out;

  cdec.GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(2.87443399e-05f, 0.00183726894f, 0.282456517f,
                             0.400047749f));

  FormatCodec::Get({FormatType::UNORM, DataType::R32G32B32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(2.87443399e-05f, 0.00183726894f, 0.282456517f, 0));

  FormatCodec::Get({FormatType::UNORM, DataType::R32G32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(2.87443399e-05f, 0.00183726894f, 0, 0));

  FormatCodec::Get({FormatType::UNORM, DataType::R32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(2.87443399e-05f, 0, 0, 0));

  USVector4 vc1(12345, 6789, 10112, 65432);

  FormatCodec::Get({FormatType::UNORM, DataType::R16G16B16A16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(
      out, Vector4A16(0.188372627f, 0.103593498f, 0.154299229f, 0.998428345f));

  FormatCodec::Get({FormatType::UNORM, DataType::R16G16B16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(0.188372627f, 0.103593498f, 0.154299229f, 0));

  FormatCodec::Get({FormatType::UNORM, DataType::R16G16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(0.188372627f, 0.103593498f, 0, 0));

  FormatCodec::Get({FormatType::UNORM, DataType::R16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(0.188372627f, 0, 0, 0));

  UCVector4 vc2(250, 120, 5, 67);

  FormatCodec::Get({FormatType::UNORM, DataType::R8G8B8A8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(
      out, Vector4A16(0.980392098f, 0.470588207f, 0.0196078420f, 0.262745082f));

  FormatCodec::Get({FormatType::UNORM, DataType::R8G8B8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, Vector4A16(0.980392098f, 0.470588207f, 0.0196078420f, 0));

  FormatCodec::Get({FormatType::UNORM, DataType::R8G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, Vector4A16(0.980392098f, 0.470588207f, 0, 0));

  FormatCodec::Get({FormatType::UNORM, DataType::R8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, Vector4A16(0.980392098f, 0, 0, 0));

  union {
    int32 r;
    struct {
      uint8 _n[3];
      uint8 g;
    } g;
  } vc3;

  vc3.r = 12345678;
  vc3.g.g = 158;

  FormatCodec::Get({FormatType::UNORM, DataType::R24G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc3));

  TEST_EQUAL(out, Vector4A16(0.735859752f, 0.619607806f, 0, 0));

  uint32 vc4 = 999 | (725 << 11) | (482 << 22);

  FormatCodec::Get({FormatType::UNORM, DataType::R11G11B10})
      .GetValue(out, reinterpret_cast<const char *>(&vc4));

  TEST_EQUAL(out, Vector4A16(0.488031268f, 0.354176849f, 0.470933080f, 0));

  uint32 vc5 = 510 | (326 << 10) | (290 << 20) | (2U << 30);

  FormatCodec::Get({FormatType::UNORM, DataType::R10G10B10A2})
      .GetValue(out, reinterpret_cast<const char *>(&vc5));

  TEST_EQUAL(
      out, Vector4A16(0.498533726f, 0.318670571f, 0.283479959f, 0.666666627f));

  uint16 vc6 = 30 | (20 << 5) | (10 << 11);

  FormatCodec::Get({FormatType::UNORM, DataType::R5G6B5})
      .GetValue(out, reinterpret_cast<const char *>(&vc6));

  TEST_EQUAL(out, Vector4A16(0.952380896f, 0.317460299f, 0.317460299f, 0));

  uint16 vc7 = 29 | (19 << 5) | (9 << 10) | (1 << 15);

  FormatCodec::Get({FormatType::UNORM, DataType::R5G5B5A1})
      .GetValue(out, reinterpret_cast<const char *>(&vc7));

  TEST_EQUAL(out, Vector4A16(0.935483813f, 0.612903237f, 0.290322572f, 1.f));

  return 0;
}

int test_format_04() {
  auto &&cdec = FormatCodec::Get({FormatType::NORM, DataType::R32G32B32A32});
  IVector4 vc0(-123456, 7891011, -1213141516, 1718192021);
  Vector4A16 out;

  cdec.GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(-5.74886799e-05f, 0.00367453834f, -0.564913034f,
                             0.800095499f));

  FormatCodec::Get({FormatType::NORM, DataType::R32G32B32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out,
             Vector4A16(-5.74886799e-05f, 0.00367453834f, -0.564913034f, 0));

  FormatCodec::Get({FormatType::NORM, DataType::R32G32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(-5.74886799e-05f, 0.00367453834f, 0, 0));

  FormatCodec::Get({FormatType::NORM, DataType::R32})
      .GetValue(out, reinterpret_cast<const char *>(&vc0));

  TEST_EQUAL(out, Vector4A16(-5.74886799e-05f, 0, 0, 0));

  SVector4 vc1(12345, -6789, 10112, -32109);

  FormatCodec::Get({FormatType::NORM, DataType::R16G16B16A16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(0.376751006f, -0.207190156f, 0.308603168f,
                             -0.979918838f));

  FormatCodec::Get({FormatType::NORM, DataType::R16G16B16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(0.376751006f, -0.207190156f, 0.308603168f, 0));

  FormatCodec::Get({FormatType::NORM, DataType::R16G16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(0.376751006f, -0.207190156f, 0, 0));

  FormatCodec::Get({FormatType::NORM, DataType::R16})
      .GetValue(out, reinterpret_cast<const char *>(&vc1));

  TEST_EQUAL(out, Vector4A16(0.376751006f, 0, 0, 0));

  CVector4 vc2(-120, -62, 5, 67);

  FormatCodec::Get({FormatType::NORM, DataType::R8G8B8A8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, Vector4A16(-0.944881916f, -0.488188982f, 0.0393700786f,
                             0.527559042f));

  FormatCodec::Get({FormatType::NORM, DataType::R8G8B8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, Vector4A16(-0.944881916f, -0.488188982f, 0.0393700786f, 0));

  FormatCodec::Get({FormatType::NORM, DataType::R8G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, Vector4A16(-0.944881916f, -0.488188982f, 0, 0));

  FormatCodec::Get({FormatType::NORM, DataType::R8})
      .GetValue(out, reinterpret_cast<const char *>(&vc2));

  TEST_EQUAL(out, Vector4A16(-0.944881916f, 0, 0, 0));

  union {
    int32 r;
    struct {
      int8 _n[3];
      int8 g;
    } g;
  } vc3;

  vc3.r = -1234567;
  vc3.g.g = -108;

  FormatCodec::Get({FormatType::NORM, DataType::R24G8})
      .GetValue(out, reinterpret_cast<const char *>(&vc3));

  TEST_EQUAL(out, Vector4A16(-0.147171870f, -0.850393713f, 0, 0));

  int32 vc4 = (-500 & 0x7ff) | ((-420 & 0x7ff) << 11) | ((-10 & 0x3ff) << 22);

  FormatCodec::Get({FormatType::NORM, DataType::R11G11B10})
      .GetValue(out, reinterpret_cast<const char *>(&vc4));

  TEST_EQUAL(out, Vector4A16(-0.488758564f, -0.410557181f, -0.0185728259f, 0));

  int32 vc5 = (-250 & 0x3ff) | ((-110 & 0x3ff) << 10) | ((-50 & 0x3ff) << 20) |
              (-1 << 30);

  FormatCodec::Get({FormatType::NORM, DataType::R10G10B10A2})
      .GetValue(out, reinterpret_cast<const char *>(&vc5));

  TEST_EQUAL(out,
             Vector4A16(-0.489236772f, -0.215264186f, -0.0978473574f, -1.f));

  int16 vc6 = (-15 & 0x1f) | ((-10 & 0x3f) << 5) | ((-5 & 0x1f) << 11);

  FormatCodec::Get({FormatType::NORM, DataType::R5G6B5})
      .GetValue(out, reinterpret_cast<const char *>(&vc6));

  TEST_EQUAL(out, Vector4A16(-0.967741907f, -0.322580636f, -0.290322572f, 0));

  int16 vc7 =
      (-14 & 0x1f) | ((-9 & 0x1f) << 5) | ((-6 & 0x1f) << 10) | (-1 << 15);

  FormatCodec::Get({FormatType::NORM, DataType::R5G5B5A1})
      .GetValue(out, reinterpret_cast<const char *>(&vc7));

  TEST_EQUAL(out,
             Vector4A16(-0.933333278f, -0.599999964f, -0.399999976f, -1.f));

  return 0;
}
