#pragma once
#include "reflector_def.inl"
#include "reflector_decl.inl"

#include "../datas/binreader.hpp"
#include "../datas/binwritter.hpp"

template <class C> int test_class(const reflectorStatic *input) {
  auto orig = GetReflectedClass<C>();

  TEST_EQUAL(input->classHash, orig->classHash);

  if (orig->className) {
    TEST_EQUAL(es::string_view(input->className), orig->className);
  } else {
    TEST_EQUAL(input->className, nullptr);
  }

  TEST_EQUAL(input->nTypes, orig->nTypes);

  if (orig->typeAliases) {
    for (uint32 i = 0; i < orig->nTypes; i++) {
      if (input->typeAliases[i]) {
        TEST_EQUAL(es::string_view(input->typeAliases[i]),
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
      TEST_EQUAL(es::string_view(input->typeNames[i]), orig->typeNames[i]);

  } else {
    TEST_EQUAL(input->typeNames, nullptr);
  }

  for (uint32 i = 0; i < orig->nTypes; i++) {
    TEST_EQUAL(input->types[i].ID, orig->types[i].ID);
    TEST_EQUAL(input->types[i].numItems, orig->types[i].numItems);
    TEST_EQUAL(input->types[i].offset, orig->types[i].offset);
    TEST_EQUAL(input->types[i].subSize, orig->types[i].subSize);
    TEST_EQUAL(input->types[i].subType, orig->types[i].subType);
    TEST_EQUAL(input->types[i].type, orig->types[i].type);
    TEST_EQUAL(input->types[i].typeHash, orig->types[i].typeHash);
    TEST_EQUAL(input->types[i].valueNameHash, orig->types[i].valueNameHash);
  }

  if (orig->typeDescs) {
    for (uint32 i = 0; i < orig->nTypes; i++) {
      TEST_EQUAL(es::string_view(input->typeDescs[i].part1),
                 orig->typeDescs[i].part1);
      TEST_EQUAL(es::string_view(input->typeDescs[i].part2),
                 orig->typeDescs[i].part2);
    }
  } else {
    TEST_EQUAL(input->typeDescs, nullptr);
  }

  return 0;
};

template <class ET> int test_enum(const ReflectedEnum &input) {
  auto orig = GetReflectedEnum<ET>();

  TEST_EQUAL(orig.hash, input.hash);
  TEST_EQUAL(es::string_view(orig.name), input.name);
  TEST_EQUAL(orig.size(), input.size());

  for (size_t i = 0; i < orig.size(); i++) {
    TEST_EQUAL(orig[i], input[i]);
    TEST_EQUAL(orig.values[i], input.values[i]);
  }

  return 0;
}

int test_reflector_decl_io() {
  {
    BinWritter mwr("reflector.io");

    TEST_CHECK(mwr.IsValid());

    ReflectorIO rio;
    rio.AddClass<reflClass>();
    rio.AddClass<subrefl>();
    rio.AddClass<refBasic>();
    rio.AddClass<refTypeNames>();
    rio.AddClass<refTypeNames01>();
    rio.AddClass<roomInfo>();
    rio.AddClass<roomInfo01>();
    rio.AddClass<roomInfo02>();
    rio.AddClass<templatedClass<int, float>>();
    rio.AddEnum<EnumWrap00>();
    rio.AddEnum<EnumWrap01>();
    rio.AddEnum<EnumWrap02>();
    rio.AddEnum<EnumWrap03>();
    TEST_NOT_CHECK(rio.Save(mwr));
  }

  ReflectorIO rio2;
  BinReader mrd("reflector.io");
  TEST_CHECK(mrd.IsValid());
  TEST_NOT_CHECK(rio2.Load(mrd));

  auto classes = rio2.Classes();
  auto enums = rio2.Enums();

  TEST_EQUAL(classes.size(), 9);
  TEST_EQUAL(enums.size(), 4);

  using tclass = templatedClass<int, float>;

  TEST_CASES(int testResult, TEST_FUNC(test_class<reflClass>, classes[0]),
             TEST_FUNC(test_class<subrefl>, classes[1]),
             TEST_FUNC(test_class<refBasic>, classes[2]),
             TEST_FUNC(test_class<refTypeNames>, classes[3]),
             TEST_FUNC(test_class<refTypeNames01>, classes[4]),
             TEST_FUNC(test_class<roomInfo>, classes[5]),
             TEST_FUNC(test_class<roomInfo01>, classes[6]),
             TEST_FUNC(test_class<roomInfo02>, classes[7]),
             TEST_FUNC(test_class<tclass>, classes[8]),
             TEST_FUNC(test_enum<EnumWrap00>, enums[0]),
             TEST_FUNC(test_enum<EnumWrap01>, enums[1]),
             TEST_FUNC(test_enum<EnumWrap02>, enums[2]),
             TEST_FUNC(test_enum<EnumWrap03>, enums[3]));

  return testResult;
}
