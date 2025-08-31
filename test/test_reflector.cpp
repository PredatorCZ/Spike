#include "reflector.inl"
#include "reflector_bindump.inl"
#include "reflector_extended.inl"
#include "reflector_io.inl"
#include "spike/io/stat.hpp"

int main() {
  es::SetupWinApiConsole();
  es::print::AddPrinterFunction(es::Print);

  reflClass rClass = {};

  TEST_CASES(
      int testResult, TEST_FUNC(test_reflector_enum00),
      TEST_FUNC(test_reflector_enum01), TEST_FUNC(test_reflector_enum02),
      TEST_FUNC(test_reflector_enum03), TEST_FUNC(test_reflector_enum04),
      TEST_FUNC(test_reflector_enum05), TEST_FUNC(test_reflector_decl01),
      TEST_FUNC(test_reflector_decl02), TEST_FUNC(test_reflector_decl03),
      TEST_FUNC(test_reflector_decl04), TEST_FUNC(test_reflector_decl05),
      TEST_FUNC(test_reflector_decl06), TEST_FUNC(test_reflector_decl07),
      TEST_FUNC(test_reflector_decl09), TEST_FUNC(test_reflector_decl10),
      TEST_FUNC(test_reflector_decl11),
      TEST_FUNC(test_reflector_container_vector),
      TEST_FUNC(test_reflector, rClass), TEST_FUNC(test_reflector_bool, rClass),
      TEST_FUNC(test_reflector_int8, rClass),
      TEST_FUNC(test_reflector_uint8, rClass),
      TEST_FUNC(test_reflector_int16, rClass),
      TEST_FUNC(test_reflector_uint16, rClass),
      TEST_FUNC(test_reflector_int32, rClass),
      TEST_FUNC(test_reflector_uint32, rClass),
      TEST_FUNC(test_reflector_int64, rClass),
      TEST_FUNC(test_reflector_uint64, rClass),
      TEST_FUNC(test_reflector_float, rClass),
      TEST_FUNC(test_reflector_double, rClass),
      TEST_FUNC(test_reflector_vector, rClass),
      TEST_FUNC(test_reflector_vector2, rClass),
      TEST_FUNC(test_reflector_vector4, rClass),
      TEST_FUNC(test_reflector_vector4A16, rClass),
      TEST_FUNC(test_reflector_arrays, rClass),
      TEST_FUNC(test_reflector_enum00, rClass),
      TEST_FUNC(test_reflector_enum01, rClass),
      TEST_FUNC(test_reflector_enum02, rClass),
      TEST_FUNC(test_reflector_enum03, rClass),
      TEST_FUNC(test_reflector_enumflags00, rClass),
      TEST_FUNC(test_reflector_enumflags01, rClass),
      TEST_FUNC(test_reflector_subclass, rClass),
      TEST_FUNC(test_reflector_array_subclass, rClass),
      TEST_FUNC(test_reflector_bitfield, rClass),
      TEST_FUNC(test_custom_float, rClass),
      TEST_FUNC(test_reflector_bitfield_custom_float, rClass),
      TEST_FUNC(test_reflector_string, rClass), TEST_FUNC(test_reflector_alias),
      TEST_FUNC(test_reflector_desc), TEST_FUNC(test_reflector_io, rClass),
      TEST_FUNC(test_binwritter, rClass), TEST_FUNC(test_binreader, rClass));

  return testResult;
}
