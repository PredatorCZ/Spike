#include "../datas/base_128.hpp"
#include "../datas/unit_testing.hpp"

int test_base128() {
  std::stringstream ss;
  BinWritterRef wr(ss);
  BinReaderRef rd(ss);
  bint128 var = 0x7f;

  wr.Write(var);
  auto str = ss.str();
  const uint8 *rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xff);
  TEST_EQUAL(rdt[1], 0);

  rd.Read(var);

  TEST_EQUAL(var, 0x7f);

  var = 0x1234;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x34 | 0x80));
  TEST_EQUAL(rdt[1], ((0x1234 >> 7) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x1234);

  var = 0x77777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x77777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x77777 >> 14) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x77777);

  var = 0x7777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x7777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x7777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x7777777 >> 21) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x7777777);

  var = 0x777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], 0);

  rd.Read(var);

  TEST_EQUAL(var, 0x777777777);

  var = 0x77777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x77777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x77777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x77777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x77777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x77777777777 >> 35) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x77777777777);

  var = 0x7777777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x7777777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x7777777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x7777777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x7777777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x7777777777777 >> 35) & 0xff));
  TEST_EQUAL(rdt[6], ((0x7777777777777 >> 42) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x7777777777777);

  var = 0x777777777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x777777777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x777777777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x777777777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x777777777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x777777777777777 >> 35) & 0xff));
  TEST_EQUAL(rdt[6], ((0x777777777777777 >> 42) & 0xff));
  TEST_EQUAL(rdt[7], ((0x777777777777777 >> 49) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x777777777777777);

  var = 0x7777777777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x7777777777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x7777777777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x7777777777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x7777777777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x7777777777777777 >> 35) & 0xff));
  TEST_EQUAL(rdt[6], ((0x7777777777777777 >> 42) & 0xff));
  TEST_EQUAL(rdt[7], ((0x7777777777777777 >> 49) & 0xff));
  TEST_EQUAL(rdt[8], ((0x7777777777777777 >> 56) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x7777777777777777);

  var = -0x1f;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0x5e);

  rd.Read(var);

  TEST_EQUAL(var, -0x1f);

  var = -0x7f;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xfe);
  TEST_EQUAL(rdt[1], 0x40);

  rd.Read(var);

  TEST_EQUAL(var, -0x7f);

  var = -0x1234;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xb3);
  TEST_EQUAL(rdt[1], ((0x1233 >> 7) | 0x40));

  rd.Read(var);

  TEST_EQUAL(var, -0x1234);

  var = -0x33333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x32 | 0x80));
  TEST_EQUAL(rdt[1], (((0x33333 >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0x33333 >> 14) & 0xff) | 0x40));

  rd.Read(var);

  TEST_EQUAL(var, -0x33333);

  var = -0x3333333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x32 | 0x80));
  TEST_EQUAL(rdt[1], (((0x3333333 >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0x3333333 >> 14) & 0xff) | 0x80));
  TEST_EQUAL(rdt[3], (((0x3333333 >> 21) & 0xff) | 0x40));

  rd.Read(var);

  TEST_EQUAL(var, -0x3333333);

  var = -0x333333333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x32 | 0x80));
  TEST_EQUAL(rdt[1], (((0x333333333 >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0x333333333 >> 14) & 0xff) | 0x80));
  TEST_EQUAL(rdt[3], (((0x333333333 >> 21) & 0xff) | 0x80));
  TEST_EQUAL(rdt[4], (((0x333333333 >> 28) & 0xff) | 0x40));

  rd.Read(var);

  TEST_EQUAL(var, -0x333333333);

  var = -0x33333333333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x32 | 0x80));
  TEST_EQUAL(rdt[1], (((0x33333333333 >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0x33333333333 >> 14) & 0xff) | 0x80));
  TEST_EQUAL(rdt[3], (((0x33333333333 >> 21) & 0xff) | 0x80));
  TEST_EQUAL(rdt[4], (((0x33333333333 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], (((0x33333333333 >> 35) & 0xff) | 0x80));
  TEST_EQUAL(rdt[6], 0x40);

  rd.Read(var);

  TEST_EQUAL(var, -0x33333333333);

  var = -0x3333333333333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x32 | 0x80));
  TEST_EQUAL(rdt[1], (((0x3333333333333 >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0x3333333333333 >> 14) & 0xff) | 0x80));
  TEST_EQUAL(rdt[3], (((0x3333333333333 >> 21) & 0xff) | 0x80));
  TEST_EQUAL(rdt[4], (((0x3333333333333 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], (((0x3333333333333 >> 35) & 0xff) | 0x80));
  TEST_EQUAL(rdt[6], (((0x3333333333333 >> 42) & 0xff) | 0x40));

  rd.Read(var);

  TEST_EQUAL(var, -0x3333333333333);

  var = -0x33333333333333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x32 | 0x80));
  TEST_EQUAL(rdt[1], (((0x33333333333333 >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0x33333333333333 >> 14) & 0xff) | 0x80));
  TEST_EQUAL(rdt[3], (((0x33333333333333 >> 21) & 0xff) | 0x80));
  TEST_EQUAL(rdt[4], (((0x33333333333333 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], (((0x33333333333333 >> 35) & 0xff) | 0x80));
  TEST_EQUAL(rdt[6], (((0x33333333333333 >> 42) & 0xff) | 0x80));
  TEST_EQUAL(rdt[7], (((0x33333333333333 >> 49) & 0xff) | 0x40));

  rd.Read(var);

  TEST_EQUAL(var, -0x33333333333333);

  var = -0x333333333333333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0xcd | 0x80));
  TEST_EQUAL(rdt[1], (((0xFCCCCCCCCCCCCCCD >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0xFCCCCCCCCCCCCCCD >> 14) & 0xff) | 0x80));
  TEST_EQUAL(rdt[3], (((0xFCCCCCCCCCCCCCCD >> 21) & 0xff) | 0x80));
  TEST_EQUAL(rdt[4], (((0xFCCCCCCCCCCCCCCD >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], (((0xFCCCCCCCCCCCCCCD >> 35) & 0xff) | 0x80));
  TEST_EQUAL(rdt[6], (((0xFCCCCCCCCCCCCCCD >> 42) & 0xff) | 0x80));
  TEST_EQUAL(rdt[7], (((0xFCCCCCCCCCCCCCCD >> 49) & 0xff) | 0x80));
  TEST_EQUAL(rdt[8], ((0xFCCCCCCCCCCCCCCD >> 56) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, -0x333333333333333);

  var = -0x3333333333333333;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0xcd | 0x80));
  TEST_EQUAL(rdt[1], (((0xCCCCCCCCCCCCCCCD >> 7) & 0xff) | 0x80));
  TEST_EQUAL(rdt[2], (((0xCCCCCCCCCCCCCCCD >> 14) & 0xff) | 0x80));
  TEST_EQUAL(rdt[3], (((0xCCCCCCCCCCCCCCCD >> 21) & 0xff) | 0x80));
  TEST_EQUAL(rdt[4], (((0xCCCCCCCCCCCCCCCD >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], (((0xCCCCCCCCCCCCCCCD >> 35) & 0xff) | 0x80));
  TEST_EQUAL(rdt[6], (((0xCCCCCCCCCCCCCCCD >> 42) & 0xff) | 0x80));
  TEST_EQUAL(rdt[7], (((0xCCCCCCCCCCCCCCCD >> 49) & 0xff) | 0x80));
  TEST_EQUAL(rdt[8], ((0xCCCCCCCCCCCCCCCD >> 56) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, -0x3333333333333333);

  return 0;
}

int test_ubase128() {
  std::stringstream ss;
  BinWritterRef wr(ss);
  BinReaderRef rd(ss);
  buint128 var = 0x7f;

  wr.Write(var);
  auto str = ss.str();
  const uint8 *rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0x7f);

  rd.Read(var);

  TEST_EQUAL(var, 0x7f);

  var = 0x1234;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], (0x34 | 0x80));
  TEST_EQUAL(rdt[1], ((0x1234 >> 7) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x1234);

  var = 0x77777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x77777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x77777 >> 14) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x77777);

  var = 0x7777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x7777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x7777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x7777777 >> 21) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x7777777);

  var = 0x777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x777777777 >> 28) & 0xff)));

  rd.Read(var);

  TEST_EQUAL(var, 0x777777777);

  var = 0x77777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x77777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x77777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x77777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x77777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x77777777777 >> 35) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x77777777777);

  var = 0x7777777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x7777777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x7777777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x7777777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x7777777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x7777777777777 >> 35) & 0xff));
  TEST_EQUAL(rdt[6], ((0x7777777777777 >> 42) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x7777777777777);

  var = 0x777777777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x777777777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x777777777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x777777777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x777777777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x777777777777777 >> 35) & 0xff));
  TEST_EQUAL(rdt[6], ((0x777777777777777 >> 42) & 0xff));
  TEST_EQUAL(rdt[7], ((0x777777777777777 >> 49) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x777777777777777);

  var = 0x7777777777777777;

  ss = std::stringstream();
  wr.Write(var);
  str = ss.str();
  rdt = reinterpret_cast<const uint8 *>(str.data());

  TEST_EQUAL(rdt[0], 0xf7);
  TEST_EQUAL(rdt[1], ((0x7777777777777777 >> 7) & 0xff));
  TEST_EQUAL(rdt[2], ((0x7777777777777777 >> 14) & 0xff));
  TEST_EQUAL(rdt[3], ((0x7777777777777777 >> 21) & 0xff));
  TEST_EQUAL(rdt[4], (((0x7777777777777777 >> 28) & 0xff) | 0x80));
  TEST_EQUAL(rdt[5], ((0x7777777777777777 >> 35) & 0xff));
  TEST_EQUAL(rdt[6], ((0x7777777777777777 >> 42) & 0xff));
  TEST_EQUAL(rdt[7], ((0x7777777777777777 >> 49) & 0xff));
  TEST_EQUAL(rdt[8], ((0x7777777777777777 >> 56) & 0xff));

  rd.Read(var);

  TEST_EQUAL(var, 0x7777777777777777);

  return 0;
}