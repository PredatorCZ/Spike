#pragma once
#include "reflector_decl.inl"

int test_reflector_alias() {
  roomInfo cClass{};
  auto refInterface = ReflectorWrap<decltype(cClass)>(cClass);

  TEST_EQUAL(refInterface.GetReflectedValue("room_density"), "0");
  TEST_EQUAL(refInterface.SetReflectedValueFloat("room_density", 50.87),
             Reflector::ErrorType::None);
  TEST_EQUAL(refInterface.GetReflectedValue("room_density"), "50.87");
  TEST_EQUAL(refInterface.GetReflectedValue("roomDensity"), "50.87");

  Reflector::KVPair cPair = refInterface.GetReflectedPair("room_density");
  TEST_EQUAL(cPair.name, "roomDensity");
  TEST_EQUAL(cPair.value, "50.87");

  Reflector::KVPairFormat fmt;
  fmt.aliasName = true;
  cPair = refInterface.GetReflectedPair("room_density", fmt);
  TEST_EQUAL(cPair.name, "room_density");
  TEST_EQUAL(cPair.value, "50.87");

  return 0;
}

int test_reflector_desc() {
  roomInfo01 cClass{};
  auto refInterface = ReflectorWrap<decltype(cClass)>(cClass);

  TEST_EQUAL(refInterface.GetReflectedValue("roomSize"), "0");
  TEST_EQUAL(refInterface.SetReflectedValueFloat("roomSize", 3.5),
             Reflector::ErrorType::None);
  TEST_EQUAL(refInterface.GetReflectedValue("roomSize"), "3.5");

  Reflector::KVPair cPair = refInterface.GetReflectedPair("roomSize");
  TEST_EQUAL(cPair.name, "roomSize");
  TEST_EQUAL(cPair.value, "3.5");

  Reflector::KVPairFormat fmt;
  fmt.aliasName = true;
  fmt.formatValue = true;
  cPair = refInterface.GetReflectedPair("roomSize", fmt);
  TEST_EQUAL(cPair.name, "roomSize");
  TEST_EQUAL(cPair.value, "Size of room is 3.5 m3");

  return 0;
}