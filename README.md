# PreCore

[![build](https://github.com/PredatorCZ/PreCore/actions/workflows/cmake.yml/badge.svg)](https://github.com/PredatorCZ/PreCore/actions/workflows/cmake.yml)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/50479b1d15d848bcb389dc2166c294e2)](https://www.codacy.com/manual/PredatorCZ/PreCore?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PredatorCZ/PreCore&amp;utm_campaign=Badge_Grade)
[![Coverage Status](https://coveralls.io/repos/github/PredatorCZ/PreCore/badge.svg?branch=master)](https://coveralls.io/github/PredatorCZ/PreCore?branch=master)

PreCore is a library with helper classes.\
Main focus of this library is: Class/Enum RTTI and RTTR, SIMD classes like Vector and Matrix4x4, stream helpers, uni module.\
It also contains logging services, algorhitms, data types, muti-threading queue classes, extensions for other libraries and many more.

Library is made for C++17 standard and it's compilable under Clang 5+, GCC 7+, MSVC 2017+.

## Uses additional libraries

* pugixml (Copyright (C) 2006-2020, by Arseny Kapoulkine)

## Main modules

### RTTI/RTTR (Run Time Type Introspection/Reflection)

**Headers:** reflector.hpp, reflector_io.hpp, reflector_xml.hpp\
**Unit tests:** reflector_*.inl, test_reflector.cpp

Reflector allows simple class reflection and de/serialization.\
It also allows to fully reflect enumerations.

![Reflector Basic Workflow Diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/predatorcz/precore/master/doc/ref_bwd.puml)

![Reflector Relations](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/predatorcz/precore/master/doc/rf_rel.puml)

**Strengths:**

* Easy RTTI creation with ***REFLECT/MAKE_ENUM*** macros
* Loading/saving RTTI
* Built-in XML/Binary de/serializers
* Can apply other de/serializers with ease.
* Doesn't eat any class space (except ReflectorBase)
* Can have member alias and description in RTTI

**Disadvantages, that shall be fixed:**

* No reflection for heap containers (except std::string)
* No run-time class creation
* No RTTI saving for STL containers (possible, but unsafe) [won't be implemented]
* Method calling (like in RPC)

### UNI

**Headers:** in uni folder\
**Unit tests:** uni_\*.inl, test_uni.cpp

Set of abstract interfaces for polymorphic classes.\
Contains python bindings and format codecs.

### Formats

**Headers:** in formats folder\
**Unit tests:** none

Set of headers for format structures and format codecs.

### CMake

Set of helper modules for CMake.

### Bincore

**Headers:** binreader\*.hpp, binwritter\*.hpp\
**Unit Tests:** reflector_bindump.inl, bincore.inl\
**Used in:** Reflector

A set of classes, that helps with stream reading/writing.

### Master Printer

**Headers:** master_printer.hpp\
**Unit Tests:** test_base.cpp (kinda of)\
**Used in:** Reflector

Global logger, that can log into a multiple places at the same time.\
Can log from multiple threads at the same time.

## Other modules

### Allocator hybrid

**Headers:** allocator_hypbrid.hpp\
**Unit Tests:** allocator_hypbrid.inl

A special allocator that allows stl classes to read from constant or already allocated space.

### Deleter hybrid

**Headers:** deleter_hybrid.hpp\
**Unit Tests:** none\
**Used in:** uni::Element

A conditional deleter for smart pointers.

### Endian

**Headers:** endian.hpp\
**Unit Tests:** endian.inl

A fast endianness swapper with SFINAE integration.

### File Info

**Headers:** fileinfo.hpp\
**Unit Tests:** fileinfo.inl

Class for a filepath splitting.

### Flags

**Headers:** flags.hpp\
**Unit Tests:** flags.inl

Allows setting bit flags with ease.\
Supports Reflector.

### Matrix44

**Headers:** matrix44.hpp\
**Unit Tests:** matrix44.inl

A SIMD transform 4x4 matrix.

### Vector4 SIMD

**Headers:** vectors_simd.hpp\
**Unit Tests:** vector_simd.inl\
**Used in:** matrix44, float, simd block decompressors

A SIMD 4 component vector class for float and int.\
Supports Reflector.

### String View

**Headers:** string_view.hpp\
**Unit Tests:** none

String view for C++11 standard.

### Jenkins Hash

**Headers:** jenkinshash.hpp\
**Unit Tests:** none\
**Used in:** Reflector

Compile time Jenkins One at time hash class.
