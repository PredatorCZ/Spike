/*  Contains macros/classes for reflection of classes/enums
    more info in README for PreCore Project

    Copyright 2018-2021 Lukas Cone

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once
#include "macroLoop.hpp"
#include "supercore.hpp"
#include "tchar.hpp"

#include "internal/reflector_class.hpp"
#include "internal/reflector_enum.hpp"
#include "internal/reflector_reg.hpp"
#include "internal/reflector_type.hpp"

// @numFlags: [0,n] or ENUM + numEnumFlags or BITFIELD + numBitFieldFlags
// Usable flags: VARNAMES or EXTENDED, TEMPLATE
// VARNAMES: Saves variable names (do not use with EXTENDED)
// EXTENDED: Each variable must have (<type>, varname, ...) format
// EXTENDED <type>: A (use alias), D (use descriptor), AD (use both),
//   <none> only variable name
// EXTENDED examples:
//   REFLECTOR_CREATE(myClass, 1, EXTENDED,
//     (AD, var1, "var alias", "var descriptor"), (A, var2, "other alias"),
//     (D, var3, "description"), (, var4), <other vars>...)
// TEMPLATE: Allows usage of template arguments for class name.
//   Class name must be in () braces
// TEMPLATE examples:
//   REFLECTOR_CREATE((myClass<int, other>), 1, TEMPLATE, <values>...)
// Usable enum flags: CLASS, enum size (64 or 32 or 16 or 8)
// ENUM examples:
//   REFLECTOR_CREATE(myEnum, ENUM, 1 CLASS, var1, var2, <other vars>...)
// Usable BITFIELD flags: same as for class

#define REFLECTOR_CREATE(classname, numFlags, ...)                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER##numFlags(classname, __VA_ARGS__))

class PC_EXTERN Reflector {
  friend class ReflectorFriend;

public:
  struct KVPairFormat {
    bool aliasName = false;
    bool formatValue = false;

    constexpr KVPairFormat() {}
  };

  struct KVPair {
    es::string_view name;
    std::string value;
  };

  enum class ErrorType {
    None,
    SignMismatch,       // A singed number is assigned to an unsigned.
    OutOfRange,         // A number greater than destination type can hold.
    InvalidFormat,      // Wrong format of input provided.
    InvalidDestination, // Destination not found.
    EmptyInput,         // Input string was empty.
    ShortInput,         // Input string have insufficient number of elements.
  };

private:
  virtual ReflectedInstance GetReflectedInstance() const = 0;
  ReflectedInstance GetReflectedInstance();
  // clang-format off
protected:
  const reflType *GetReflectedType(JenHash hashName) const;
  const reflType *GetReflectedType(size_t ID) const;
  ErrorType SetReflectedValue(reflType type, es::string_view value);
  ErrorType SetReflectedValue(reflType type, es::string_view value, size_t subID);
  ErrorType SetReflectedValue(reflType type, es::string_view value, size_t subID, size_t element);
  ErrorType SetReflectedValueUInt(reflType type, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(reflType type, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(reflType type, double value, size_t subID = 0);

public:
  ErrorType SetReflectedValue(size_t id, es::string_view value);
  ErrorType SetReflectedValue(JenHash hashName, es::string_view value);

  ErrorType SetReflectedValue(size_t id, es::string_view value, size_t subID);
  ErrorType SetReflectedValue(JenHash hashName, es::string_view value, size_t subID);

  ErrorType SetReflectedValue(size_t id, es::string_view value, size_t subID, size_t element);
  ErrorType SetReflectedValue(JenHash hashName, es::string_view value, size_t subID, size_t element);

  ErrorType SetReflectedValueInt(JenHash hashName, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(size_t id, int64 value, size_t subID = 0);

  ErrorType SetReflectedValueUInt(JenHash hashName, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueUInt(size_t id, uint64 value, size_t subID = 0);

  ErrorType SetReflectedValueFloat(JenHash hashName, double value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(size_t id, double value, size_t subID = 0);

  size_t GetNumReflectedValues() const;
  es::string_view GetClassName() const;
  bool UseNames() const;

  std::string GetReflectedValue(size_t id) const;
  std::string GetReflectedValue(JenHash hashName) const;

  std::string GetReflectedValue(size_t id, size_t subID) const;
  std::string GetReflectedValue(JenHash hashName, size_t subID) const;

  std::string GetReflectedValue(size_t id, size_t subID, size_t element) const;
  std::string GetReflectedValue(JenHash hashName, size_t subID, size_t element) const;

  bool IsReflectedSubClass(JenHash hashName) const;
  bool IsReflectedSubClass(size_t id) const;

  bool IsArray(JenHash hashName) const;
  bool IsArray(size_t id) const; 

  ReflectedInstance GetReflectedSubClass(JenHash hashName, size_t subID = 0) const;
  ReflectedInstance GetReflectedSubClass(size_t id, size_t subID = 0) const;

  ReflectedInstance GetReflectedSubClass(JenHash hashName, size_t subID = 0);
  ReflectedInstance GetReflectedSubClass(size_t id, size_t subID = 0);

  KVPair GetReflectedPair(size_t id, const KVPairFormat &settings = {}) const;
  KVPair GetReflectedPair(JenHash hashName, const KVPairFormat &settings = {}) const;
  // clang-format on
};

template <class C> class ReflectorWrap : public Reflector {
  using pure_type = typename std::remove_const<C>::type;
  ReflectedInstance GetReflectedInstance() const override {
    return {GetReflectedClass<pure_type>(), &data};
  }

public:
  C &data;
  ReflectorWrap(C *_data) : data(*_data) {}
  ReflectorWrap(C &_data) : data(_data) {}
  ReflectorWrap() = delete;
};

class ReflectorPureWrap : public Reflector {
  ReflectedInstance GetReflectedInstance() const override { return data; }

public:
  ReflectedInstance &data;
  ReflectorPureWrap(ReflectedInstance *_data) : data(*_data) {}
  ReflectorPureWrap(ReflectedInstance &_data) : data(_data) {}
  ReflectorPureWrap() = delete;
};

template <class C> class ReflectorBase : public Reflector {
public:
  ReflectedInstance GetReflectedInstance() const override {
    return {GetReflectedClass<C>(), {static_cast<const C *>(this)}};
  }
};

#include "internal/reflector.inl"
