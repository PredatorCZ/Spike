#include "datas/encrypt/blowfish2.h"
#include "datas/tchar.hpp"
#include "datas/unit_testing.hpp"
#include <cstring>

using namespace es::string_view_literals;

constexpr uint64 VECTOR = 0x6556654C704D6153; // SaMpLeVe
constexpr es::string_view KEY = "Sample Key"sv;
constexpr auto TEXT = "This a sample text, idk what to write, lol :)"sv;

int test_blowfish_ECB() {
  BlowfishEncoder bf;
  bf.SetKey(KEY);
  bf.Vector(VECTOR);
  bf.mode = IBlockEncryptor::Mode::ECB;

  uint8 buffer[48]{};
  memcpy(buffer, TEXT.data(), TEXT.size());
  bf.Encode(reinterpret_cast<char *>(buffer), sizeof(buffer));

  const uint8 result[]{
      0xa0, 0x95, 0x81, 0x8b, 0x02, 0x05, 0xfb, 0x4a, 0x0e, 0x5f, 0xd2, 0xac,
      0x16, 0xd0, 0xde, 0x6d, 0x3f, 0x1b, 0xe1, 0x96, 0x8f, 0x8a, 0x5b, 0x59,
      0xef, 0x64, 0x3a, 0x45, 0x20, 0xae, 0xe0, 0x06, 0xcf, 0x43, 0x19, 0x4e,
      0x2f, 0x1f, 0xb1, 0x08, 0xdf, 0x97, 0xd4, 0x6a, 0xe1, 0x3b, 0x9c, 0xca,
  };

  TEST_NOT_CHECK(memcmp(buffer, result, sizeof(result)));
  bf.Decode(reinterpret_cast<char *>(buffer), sizeof(buffer));
  TEST_NOT_CHECK(memcmp(buffer, TEXT.data(), TEXT.size()));

  return 0;
}

int test_blowfish_CBC() {
  BlowfishEncoder bf;
  bf.SetKey(KEY);
  bf.Vector(VECTOR);
  bf.mode = IBlockEncryptor::Mode::CBC;

  uint8 buffer[48]{};
  memcpy(buffer, TEXT.data(), TEXT.size());
  bf.Encode(reinterpret_cast<char *>(buffer), sizeof(buffer));

  const uint8 result[]{
      0xf6, 0xd4, 0x2e, 0x69, 0xc6, 0xf7, 0x41, 0x76, 0x94, 0x33, 0x13, 0x78,
      0x6d, 0x19, 0xe3, 0xd1, 0x84, 0x70, 0x13, 0xcf, 0x74, 0xd1, 0xf4, 0x5c,
      0xd6, 0x59, 0xab, 0x96, 0x45, 0x71, 0xd5, 0x63, 0x6c, 0x80, 0x8d, 0xf7,
      0xc5, 0x6b, 0x1b, 0x8d, 0x3a, 0xf2, 0xdc, 0xd1, 0x53, 0xb2, 0xb3, 0x1d,
  };

  TEST_NOT_CHECK(memcmp(buffer, result, sizeof(result)));
  bf.Decode(reinterpret_cast<char *>(buffer), sizeof(buffer));
  TEST_NOT_CHECK(memcmp(buffer, TEXT.data(), TEXT.size()));

  return 0;
}

int test_blowfish_CFB() {
  BlowfishEncoder bf;
  bf.SetKey(KEY);
  bf.Vector(VECTOR);
  bf.mode = IBlockEncryptor::Mode::CFB;

  uint8 buffer[48]{};
  memcpy(buffer, TEXT.data(), TEXT.size());
  bf.Encode(reinterpret_cast<char *>(buffer), sizeof(buffer));

  const uint8 result[]{
      0xb6, 0x1d, 0xbf, 0x69, 0x5d, 0x51, 0xb0, 0x36, 0x93, 0x22, 0x42, 0x4a,
      0xda, 0x7f, 0x38, 0x40, 0x06, 0x5c, 0xe4, 0xbb, 0x22, 0x37, 0x54, 0x05,
      0x77, 0x25, 0x40, 0xe2, 0x47, 0x01, 0x2e, 0xa5, 0xb8, 0x4c, 0x60, 0x75,
      0x19, 0x6a, 0x53, 0x9c, 0x76, 0x1e, 0x81, 0x10, 0xbd, 0x94, 0xba, 0x9e,
  };

  TEST_NOT_CHECK(memcmp(buffer, result, sizeof(result)));
  bf.Decode(reinterpret_cast<char *>(buffer), sizeof(buffer));
  TEST_NOT_CHECK(memcmp(buffer, TEXT.data(), TEXT.size()));

  return 0;
}

int test_blowfish_OFB() {
  BlowfishEncoder bf;
  bf.SetKey(KEY);
  bf.Vector(VECTOR);
  bf.mode = IBlockEncryptor::Mode::OFB;

  uint8 buffer[48]{};
  memcpy(buffer, TEXT.data(), TEXT.size());
  bf.Encode(reinterpret_cast<char *>(buffer), sizeof(buffer));

  const uint8 result[]{
      0xb6, 0x1d, 0xbf, 0x69, 0x5d, 0x51, 0xb0, 0x36, 0xbb, 0xdf, 0x15, 0xe0,
      0x80, 0x07, 0xda, 0xa2, 0x6e, 0xd8, 0x19, 0x02, 0x1b, 0xc3, 0x55, 0x56,
      0x41, 0xa1, 0xa6, 0x9c, 0xac, 0x71, 0xa4, 0x7e, 0x36, 0x9f, 0x07, 0x32,
      0x5a, 0xb2, 0xf6, 0x41, 0xbb, 0x45, 0xfe, 0x7c, 0x5b, 0xff, 0x56, 0xa2,
  };

  TEST_NOT_CHECK(memcmp(buffer, result, sizeof(result)));
  bf.Decode(reinterpret_cast<char *>(buffer), sizeof(buffer));
  TEST_NOT_CHECK(memcmp(buffer, TEXT.data(), TEXT.size()));

  return 0;
}

int test_blowfish_PCBC() {
  BlowfishEncoder bf;
  bf.SetKey(KEY);
  bf.Vector(VECTOR);
  bf.mode = IBlockEncryptor::Mode::PCBC;

  uint8 buffer[48]{};
  memcpy(buffer, TEXT.data(), TEXT.size());
  bf.Encode(reinterpret_cast<char *>(buffer), sizeof(buffer));

  const uint8 result[]{
      0xf6, 0xd4, 0x2e, 0x69, 0xc6, 0xf7, 0x41, 0x76, 0xdc, 0x54, 0x1c, 0xf0,
      0x3a, 0x3c, 0xcf, 0xdd, 0x2c, 0x8a, 0xb1, 0xe6, 0x2d, 0xa4, 0x1c, 0x32,
      0x70, 0xd3, 0xde, 0x9d, 0x7a, 0x15, 0xfc, 0x41, 0x55, 0x84, 0xa3, 0xb8,
      0xa7, 0x27, 0x7a, 0xfc, 0xa4, 0x45, 0x4e, 0xcd, 0x02, 0x74, 0x09, 0xf5,
  };

  TEST_NOT_CHECK(memcmp(buffer, result, sizeof(result)));
  bf.Decode(reinterpret_cast<char *>(buffer), sizeof(buffer));
  TEST_NOT_CHECK(memcmp(buffer, TEXT.data(), TEXT.size()));

  return 0;
}

int test_blowfish2() {
  BlowfishEncoder2 bf;
  bf.SetKey(KEY);

  uint8 buffer[48]{};
  memcpy(buffer, TEXT.data(), TEXT.size());
  bf.Encode(reinterpret_cast<char *>(buffer), sizeof(buffer));
  bf.Decode(reinterpret_cast<char *>(buffer), sizeof(buffer));
  TEST_NOT_CHECK(memcmp(buffer, TEXT.data(), TEXT.size()));

  return 0;
}

int main() {
  es::print::AddPrinterFunction(UPrintf);

  TEST_CASES(int testResult, TEST_FUNC(test_blowfish_ECB),
             TEST_FUNC(test_blowfish_CBC), TEST_FUNC(test_blowfish_CFB),
             TEST_FUNC(test_blowfish_OFB), TEST_FUNC(test_blowfish_PCBC),
             TEST_FUNC(test_blowfish2));

  return testResult;
}
