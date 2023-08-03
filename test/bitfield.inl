#include "spike/type/bitfield.hpp"
#include "spike/util/endian.hpp"
#include "spike/util/unit_testing.hpp"

int test_bf_00() {
  using member0 = BitMemberDecl<0, 2>;
  using member1 = BitMemberDecl<1, 5>;
  using member2 = BitMemberDecl<2, 3>;
  using member3 = BitMemberDecl<3, 1>;
  using member4 = BitMemberDecl<4, 2>;
  using member5 = BitMemberDecl<5, 3>;
  using BitType = BitFieldType<uint16, member0, member1, member2, member3,
                               member4, member5>;

  BitType value;
  value.value = 0x6c89;

  TEST_EQUAL(value.Get<member0>(), 1);
  TEST_EQUAL(value.Get<member1>(), 2);
  TEST_EQUAL(value.Get<member2>(), 1);
  TEST_EQUAL(value.Get<member3>(), 1);
  TEST_EQUAL(value.Get<member4>(), 1);
  TEST_EQUAL(value.Get<member5>(), 3);

  value.value = 0;

  value.Set<member0>(1);
  value.Set<member1>(2);
  value.Set<member2>(1);
  value.Set<member3>(1);
  value.Set<member4>(1);
  value.Set<member5>(3);

  TEST_EQUAL(value.value, 0x6c89);

  value.value = 0x6b44;

  FByteswapper(value);

  TEST_EQUAL(value.Get<member0>(), 1);
  TEST_EQUAL(value.Get<member1>(), 2);
  TEST_EQUAL(value.Get<member2>(), 1);
  TEST_EQUAL(value.Get<member3>(), 1);
  TEST_EQUAL(value.Get<member4>(), 1);
  TEST_EQUAL(value.Get<member5>(), 3);

  value.value = 0;

  value.Set<member0>(1);
  value.Set<member1>(2);
  value.Set<member2>(1);
  value.Set<member3>(1);
  value.Set<member4>(1);
  value.Set<member5>(3);

  FByteswapper(value, true);

  TEST_EQUAL(value.value, 0x6b44);

  return 0;
}

int test_bf_01() {
  using member0 = BitMemberDecl<0, 4>;
  using member1 = BitMemberDecl<1, 2>;
  using member2 = BitMemberDecl<2, 4>;
  using member3 = BitMemberDecl<3, 4>;
  using member4 = BitMemberDecl<4, 4>;
  using member5 = BitMemberDecl<5, 4>;
  using member6 = BitMemberDecl<6, 4>;
  using member7 = BitMemberDecl<7, 4>;
  using member8 = BitMemberDecl<8, 2>;
  using BitType = BitFieldType<uint32, member0, member1, member2, member3,
                               member4, member5, member6, member7, member8>;

  BitType value;
  value.value = 0x48484483;

  TEST_EQUAL(value.Get<member0>(), 3);
  TEST_EQUAL(value.Get<member1>(), 0);
  TEST_EQUAL(value.Get<member2>(), 2);
  TEST_EQUAL(value.Get<member3>(), 1);
  TEST_EQUAL(value.Get<member4>(), 1);
  TEST_EQUAL(value.Get<member5>(), 2);
  TEST_EQUAL(value.Get<member6>(), 1);
  TEST_EQUAL(value.Get<member7>(), 2);
  TEST_EQUAL(value.Get<member8>(), 1);

  value.value = 0;

  value.Set<member0>(3);
  value.Set<member1>(0);
  value.Set<member2>(2);
  value.Set<member3>(1);
  value.Set<member4>(1);
  value.Set<member5>(2);
  value.Set<member6>(1);
  value.Set<member7>(2);
  value.Set<member8>(1);

  TEST_EQUAL(value.value, 0x48484483);

  value.value = 0x49488430;

  FByteswapper(value);

  TEST_EQUAL(value.Get<member0>(), 3);
  TEST_EQUAL(value.Get<member1>(), 0);
  TEST_EQUAL(value.Get<member2>(), 2);
  TEST_EQUAL(value.Get<member3>(), 1);
  TEST_EQUAL(value.Get<member4>(), 1);
  TEST_EQUAL(value.Get<member5>(), 2);
  TEST_EQUAL(value.Get<member6>(), 1);
  TEST_EQUAL(value.Get<member7>(), 2);
  TEST_EQUAL(value.Get<member8>(), 1);

  value.value = 0;

  value.Set<member0>(3);
  value.Set<member1>(0);
  value.Set<member2>(2);
  value.Set<member3>(1);
  value.Set<member4>(1);
  value.Set<member5>(2);
  value.Set<member6>(1);
  value.Set<member7>(2);
  value.Set<member8>(1);

  FByteswapper(value, true);

  TEST_EQUAL(value.value, 0x49488430);

  return 0;
}
