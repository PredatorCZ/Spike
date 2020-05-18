#include "../datas/supercore.hpp"
#include "../datas/unit_testing.hpp"

ES_STATIC_ASSERT(CompileFourCC("ABCD") == 0x44434241);

constexpr size_t frac00 = GetFraction_t<1>::VALUE;
constexpr size_t frac01 = GetFraction_t<2>::VALUE;
constexpr size_t frac02 = GetFraction_t<3>::VALUE;
constexpr size_t frac03 = GetFraction_t<4>::VALUE;
constexpr size_t frac04 = GetFraction_t<5>::VALUE;
constexpr size_t frac05 = GetFraction_t<6>::VALUE;
constexpr size_t frac06 = GetFraction_t<7>::VALUE;
constexpr size_t frac07 = GetFraction_t<8>::VALUE;
constexpr size_t frac08 = GetFraction_t<9>::VALUE;
constexpr size_t frac09 = GetFraction_t<10>::VALUE;
constexpr size_t frac10 = GetFraction_t<11>::VALUE;
constexpr size_t frac11 = GetFraction_t<12>::VALUE;
constexpr size_t frac12 = GetFraction_t<16>::VALUE;
constexpr size_t frac13 = GetFraction_t<24>::VALUE;
constexpr size_t frac14 = GetFraction_t<32>::VALUE;
constexpr size_t frac15 = GetFraction_t<48>::VALUE;
constexpr size_t frac16 = GetFraction_t<126>::VALUE;

#define GF(frc) reinterpret_cast<const float &>(frc)

int test_supercore_00() {
  TEST_EQUAL(GF(frac00), 1.f);
  TEST_EQUAL(GF(frac01), 0.3333333135f);
  TEST_EQUAL(GF(frac02), 0.1428571343f);
  TEST_EQUAL(GF(frac03), 0.06666666269f);
  TEST_EQUAL(GF(frac04), 1.f / 31.f);
  TEST_EQUAL(GF(frac05), 0.01587301493f);
  TEST_EQUAL(GF(frac06), 1.f / 127.f);
  TEST_EQUAL(GF(frac07), 0.003921568394f);
  TEST_EQUAL(GF(frac08), 1.f / 511.f);
  TEST_EQUAL(GF(frac09), 1.f / 1023.f);
  TEST_EQUAL(GF(frac10), 1.f / 2047.f);
  TEST_EQUAL(GF(frac11), 0.2442002296e-3f);
  TEST_EQUAL(GF(frac12), 1.f / 65535.f);
  TEST_EQUAL(GF(frac13), 5.960464478e-08f);
  TEST_EQUAL(GF(frac14), 2.328306437e-10f);
  TEST_EQUAL(GF(frac15), 3.552713679e-15f);
  TEST_EQUAL(GF(frac16), 1.175494351e-38f);

  return 0;
}