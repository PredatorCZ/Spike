#include "allocator_hybrid.inl"
#include "disabler.inl"
#include "endian.inl"
#include "fileinfo.inl"
#include "flags.inl"
#include "float.inl"
#include "matrix44.inl"

#include "bincore.inl"
#include "reflector_extended.inl"
#include "reflector_io.inl"

// Compile only
#include "../formats/BlockDecoder.h"
#include "../formats/BlockDecoder.inl"
#include "../formats/DDS.hpp"
#include "../formats/FWSE.hpp"
#include "../formats/MSF.hpp"
#include "../formats/WAVE.hpp"

#include "../uni/format.hpp"

int main() {
  printer.AddPrinterFunction(UPrintf);

  printline("Compiler info:\n\tLittle Endian: "
            << ES_LITTLE_ENDIAN << "\n\tX64: " << ES_X64
            << "\n\tClass padding optimalization: " << ES_REUSE_PADDING);

  printerror("I am error and I'm red.");
  printwarning("I am warning and I'm yellow.");
  SetConsoleTextColor(127, 127, 255);
  printline("I'm blue, da ri di danu da.") RestoreConsoleTextColor();

  RegisterLocalEnums();
  REGISTER_CLASSES(subrefl, reflClass, roomInfo, roomInfo01);

  reflClass rClass = {};

  TEST_CASES(int testResult, TEST_FUNC(test_reflector_enum00),
             TEST_FUNC(test_reflector_enum01), TEST_FUNC(test_reflector_enum02),
             TEST_FUNC(test_reflector_enum03), TEST_FUNC(test_reflector_enum04),
             TEST_FUNC(test_reflector_decl00), TEST_FUNC(test_reflector_decl01),
             TEST_FUNC(test_reflector_decl02), TEST_FUNC(test_reflector_decl03),
             TEST_FUNC(test_reflector_decl04), TEST_FUNC(test_reflector_decl05),
             TEST_FUNC(test_reflector_decl_io),
             TEST_FUNC(test_reflector, rClass),
             TEST_FUNC(test_reflector_bool, rClass),
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
             TEST_FUNC(test_reflector_string, rClass),
             TEST_FUNC(test_reflector_alias), TEST_FUNC(test_reflector_desc),
             TEST_FUNC(test_reflector_io, rClass), TEST_FUNC(test_alloc_hybrid),
             TEST_FUNC(test_disabler), TEST_FUNC(test_fileinfo),
             TEST_FUNC(test_endian), TEST_FUNC(test_flags_00),
             TEST_FUNC(test_flags_01), TEST_FUNC(test_binwritter, rClass),
             TEST_FUNC(test_binreader, rClass), TEST_FUNC(test_bincore_00),
             TEST_FUNC(test_bincore_01), TEST_FUNC(test_bincore_02),
             TEST_FUNC(test_matrix44_00), TEST_FUNC(test_matrix44_01),
             TEST_FUNC(test_matrix44_02), TEST_FUNC(test_matrix44_03),
             TEST_FUNC(test_float_00), TEST_FUNC(test_float_01));

  return testResult;
}