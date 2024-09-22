#pragma once
#include "reflector_decl.inl"

int test_reflector_alias() {
  roomInfo cClass{};
  auto refInterface = ReflectorWrap<decltype(cClass)>(cClass);
  ReflectorMember member = refInterface["room_density"];

  TEST_EQUAL(member.ReflectedValue(), "0");
  TEST_EQUAL(member.ReflectValue(50.87), ReflectorMember::ErrorType::None);
  TEST_EQUAL(member.ReflectedValue(), "50.87");
  member = refInterface["room_density"];
  TEST_EQUAL(member.ReflectedValue(), "50.87");

  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "roomDensity");
  TEST_EQUAL(cPair.value, "50.87");

  ReflectorMember::KVPairFormat fmt;
  fmt.aliasName = true;
  cPair = member.ReflectedPair(fmt);
  TEST_EQUAL(cPair.name, "room_density");
  TEST_EQUAL(cPair.value, "50.87");

  return 0;
}

int test_reflector_desc() {
  roomInfo01 cClass{};
  auto refInterface = ReflectorWrap<decltype(cClass)>(cClass);
  ReflectorMember member = refInterface["roomSize"];

  TEST_EQUAL(member.ReflectedValue(), "0");
  TEST_EQUAL(member.ReflectValue(3.5), ReflectorMember::ErrorType::None);
  TEST_EQUAL(member.ReflectedValue(), "3.5");

  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "roomSize");
  TEST_EQUAL(cPair.value, "3.5");

  ReflectorMember::KVPairFormat fmt;
  fmt.aliasName = true;
  fmt.formatValue = true;
  cPair = member.ReflectedPair(fmt);
  TEST_EQUAL(cPair.name, "roomSize");
  TEST_EQUAL(cPair.value, "Size of room is 3.5 m3");

  return 0;
}

int test_reflector_container_vector() {
  ReflVector vClass;
  auto refInterface = ReflectorWrap<ReflVector>(vClass);
  ReflectorMember member = refInterface["flVector"];
  member.ReflectValue("{10, 20, 30, 45}");
  TEST_EQUAL(member.ReflectedValue(), "{10, 20, 30, 45}");
  TEST_EQUAL(member.ReflectedValue(0), "10");
  TEST_EQUAL(member.ReflectedValue(1), "20");
  TEST_EQUAL(member.ReflectedValue(2), "30");
  TEST_EQUAL(member.ReflectedValue(3), "45");
  TEST_EQUAL(vClass.flVector.at(0), 10);
  TEST_EQUAL(vClass.flVector.at(1), 20);
  TEST_EQUAL(vClass.flVector.at(2), 30);
  TEST_EQUAL(vClass.flVector.at(3), 45);

  member.ReflectValue(106 ,6);
  TEST_EQUAL(member.ReflectedValue(4), "0");
  TEST_EQUAL(member.ReflectedValue(5), "0");
  TEST_EQUAL(member.ReflectedValue(6), "106");
  TEST_EQUAL(vClass.flVector.at(4), 0);
  TEST_EQUAL(vClass.flVector.at(5), 0);
  TEST_EQUAL(vClass.flVector.at(6), 106);

  member = refInterface["vectots"];
  ReflectorPureWrap sClass = member.ReflectedSubClass(4);
  member = sClass["vec"];
  member.ReflectValue("[1, 2, 3]");

  TEST_EQUAL(vClass.vectots.at(0).vec, Vector{});
  TEST_EQUAL(vClass.vectots.at(1).vec, Vector{});
  TEST_EQUAL(vClass.vectots.at(2).vec, Vector{});
  TEST_EQUAL(vClass.vectots.at(3).vec, Vector{});
  TEST_EQUAL(vClass.vectots.at(4).vec, Vector(1, 2, 3));

  member = refInterface["vectots"];
  TEST_EQUAL(member.ReflectedSubClass(0)["vec"].ReflectedValue(), "[0, 0, 0]");
  TEST_EQUAL(member.ReflectedSubClass(4)["vec"].ReflectedValue(), "[1, 2, 3]");

  return 0;
}
