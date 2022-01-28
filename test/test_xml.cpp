#include "datas/binreader.hpp"
#include "datas/reflector_io.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/unit_testing.hpp"
#include "datas/stat.hpp"
#include "pugixml.hpp"
#include "reflector_def.inl"

int test_reflector_xml_v1() {
  BinReader mrd("reflected.bin");
  reflClass rClass2 = {};

  TEST_NOT_CHECK(ReflectorBinUtil::Load(rClass2, mrd));

  pugi::xml_document doc;
  ReflectorXMLUtil::Save(rClass2, doc, true);
  reflClass rClass = {};
  ReflectorXMLUtil::Load(rClass, doc, true);

  return compare_classes(rClass2, rClass);
}

int test_reflector_xml_v2() {
  BinReader mrd("reflected.bin");
  reflClass rClass2 = {};

  TEST_NOT_CHECK(ReflectorBinUtil::Load(rClass2, mrd));

  pugi::xml_document doc;
  ReflectorXMLUtil::SaveV2(rClass2, doc, true);
  reflClass rClass = {};
  ReflectorXMLUtil::LoadV2(rClass, doc, true);

  return compare_classes(rClass2, rClass);
}

int main() {
  es::SetupWinApiConsole();
  es::print::AddPrinterFunction(es::Print);

  RegisterReflectedTypes<subrefl, reflClass, EnumWrap00, EnumWrap01, EnumWrap02,
                         EnumWrap03, EnumWrap04, BitTypeRefl,
                         BFVectorR10G11B10Type>();

  TEST_CASES(int testResult, TEST_FUNC(test_reflector_xml_v1),
             TEST_FUNC(test_reflector_xml_v2));

  return testResult;
}
