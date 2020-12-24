#include "../datas/bitfield.hpp"
#include "../datas/endian.hpp"
#include "../datas/unit_testing.hpp"

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

  value.value = 0x446b;

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

  TEST_EQUAL(value.value, 0x446b);

  return 0;
}
