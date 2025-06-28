#pragma once
#include "reflector_decl.inl"
#include "reflector_def.inl"

#include "spike/io/binreader.hpp"
#include "spike/io/binwritter.hpp"

template <class C> int test_class(const reflectorStatic *input) {
  auto orig = GetReflectedClass<C>();

  TEST_EQUAL(input->classHash, orig->classHash);

  if (orig->className) {
    TEST_EQUAL(std::string_view(input->className), orig->className);
  } else {
    TEST_EQUAL(input->className, nullptr);
  }

  TEST_EQUAL(input->nTypes, orig->nTypes);

  if (orig->typeAliases) {
    for (uint32 i = 0; i < orig->nTypes; i++) {
      if (input->typeAliases[i]) {
        TEST_EQUAL(std::string_view(input->typeAliases[i]),
                   orig->typeAliases[i]);
      } else {
        TEST_EQUAL(input->typeAliases[i], nullptr);
      }
    }

  } else {
    TEST_EQUAL(input->typeAliases, nullptr);
  }

  if (orig->typeNames) {
    for (uint32 i = 0; i < orig->nTypes; i++)
      if (auto inputName = input->typeNames[i]) {
        TEST_EQUAL(std::string_view(input->typeNames[i]), orig->typeNames[i]);
      }

  } else {
    TEST_EQUAL(input->typeNames, nullptr);
  }

  for (uint32 i = 0; i < orig->nTypes; i++) {
    TEST_EQUAL(input->types[i].index, orig->types[i].index);
    TEST_EQUAL(input->types[i].size, orig->types[i].size);
    TEST_EQUAL(input->types[i].offset, orig->types[i].offset);
    TEST_EQUAL(input->types[i].raw[0], orig->types[i].raw[0]);
    TEST_EQUAL(input->types[i].raw[1], orig->types[i].raw[1]);
    TEST_EQUAL(input->types[i].type, orig->types[i].type);
    TEST_EQUAL(input->types[i].valueNameHash, orig->types[i].valueNameHash);
  }

  if (orig->typeDescs) {
    for (uint32 i = 0; i < orig->nTypes; i++) {
      TEST_EQUAL(std::string_view(input->typeDescs[i].part1),
                 orig->typeDescs[i].part1);
      TEST_EQUAL(std::string_view(input->typeDescs[i].part2),
                 orig->typeDescs[i].part2);
    }
  } else {
    TEST_EQUAL(input->typeDescs, nullptr);
  }

  return 0;
};

template <class ET> int test_enum(const ReflectedEnum *input) {
  auto orig = GetReflectedEnum<ET>();

  TEST_EQUAL(orig->enumHash, input->enumHash);
  TEST_EQUAL(std::string_view(orig->enumName), input->enumName);
  TEST_EQUAL(orig->numMembers, input->numMembers);

  for (size_t i = 0; i < input->numMembers; i++) {
    TEST_EQUAL(std::string_view(orig->names[i]), input->names[i]);
    TEST_EQUAL(orig->values[i], input->values[i]);
  }

  return 0;
}
