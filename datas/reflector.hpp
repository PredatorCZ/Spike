/*  Contains macros/classes for reflection of classes/enums
    more info in README for PreCore Project

    Copyright 2018-2020 Lukas Cone

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

#ifndef ES_REFLECTOR_DEFINED
#define ES_REFLECTOR_DEFINED
#include "macroLoop.hpp"
#include "supercore.hpp"
#include "tchar.hpp"

#include "internal/reflector_class.hpp"
#include "internal/reflector_enum.hpp"
#include "internal/reflector_reg.hpp"
#include "internal/reflector_type.hpp"

// @numFlags: [0,n] or ENUM + numEnumFlags
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
#define REFLECTOR_CREATE(classname, numFlags, ...)                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER##numFlags(classname, __VA_ARGS__))

class Reflector {
  friend class ReflectorBinUtil;
  friend class ReflectorXMLUtil;

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

  struct SubClass {
    reflectorInstance inst;
    reflectorInstanceConst instc;
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
  virtual reflectorInstanceConst GetReflectedInstance() const = 0;
  virtual reflectorInstance GetReflectedInstance() = 0;
  // clang-format off
protected:
  const reflType *GetReflectedType(JenHash hash) const;
  const reflType *GetReflectedType(es::string_view name) const;
  const reflType *GetReflectedType(size_t ID) const;
  ErrorType SetReflectedValue(reflType type, es::string_view value);
  ErrorType SetReflectedValueUInt(reflType type, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(reflType type, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(reflType type, double value, size_t subID = 0);

public:
  ErrorType SetReflectedValue(es::string_view name, es::string_view value);
  ErrorType SetReflectedValue(JenHash hash, es::string_view value);
  ErrorType SetReflectedValue(size_t id, es::string_view value);

  ErrorType SetReflectedValueInt(es::string_view name, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(JenHash hash, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(size_t id, int64 value, size_t subID = 0);

  ErrorType SetReflectedValueUInt(es::string_view name, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueUInt(JenHash hash, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueUInt(size_t id, uint64 value, size_t subID = 0);

  ErrorType SetReflectedValueFloat(es::string_view name, double value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(JenHash hash, double value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(size_t id, double value, size_t subID = 0);

  size_t GetNumReflectedValues() const;
  es::string_view GetClassName() const;
  bool UseNames() const;

  std::string GetReflectedValue(size_t id) const;
  std::string GetReflectedValue(JenHash hash) const;
  std::string GetReflectedValue(es::string_view name) const;

  bool IsReflectedSubClass(es::string_view name) const;
  bool IsReflectedSubClass(JenHash hash) const;
  bool IsReflectedSubClass(size_t id) const;

  bool IsArray(es::string_view name) const;
  bool IsArray(JenHash hash) const;
  bool IsArray(size_t id) const; 

  const SubClass GetReflectedSubClass(es::string_view name, size_t subID = 0) const;
  const SubClass GetReflectedSubClass(JenHash hash, size_t subID = 0) const;
  const SubClass GetReflectedSubClass(size_t id, size_t subID = 0) const;

  const SubClass GetReflectedSubClass(es::string_view name, size_t subID = 0);
  const SubClass GetReflectedSubClass(JenHash hash, size_t subID = 0);
  const SubClass GetReflectedSubClass(size_t id, size_t subID = 0);

  KVPair GetReflectedPair(size_t id, const KVPairFormat &settings = {}) const;
  KVPair GetReflectedPair(JenHash hash, const KVPairFormat &settings = {}) const;
  KVPair GetReflectedPair(es::string_view name, const KVPairFormat &settings = {}) const;
  // clang-format on
};

template <class C> struct ReflectorInterface;

template <class C> class ReflectorWrap : public Reflector {
  reflectorInstanceConst GetReflectedInstance() const override {
    return static_cast<const ReflectorInterface<C> *>(data)
        ->GetReflectedInstance();
  }
  reflectorInstance GetReflectedInstance() override {
    return data->GetReflectedInstance();
  }

public:
  ReflectorInterface<C> *data;
  ReflectorWrap(ReflectorInterface<C> *_data) : data(_data) {}
  ReflectorWrap(ReflectorInterface<C> &_data) : data(&_data) {}
  ReflectorWrap() = delete;
};

template <class C> class ReflectorWrapConst : public Reflector {
  reflectorInstanceConst GetReflectedInstance() const override {
    return data->GetReflectedInstance();
  }

  reflectorInstance GetReflectedInstance() override {
    static const reflectorStatic nullRefType = {_RTag<void>{}};
    return {&nullRefType, nullptr};
  }

public:
  const ReflectorInterface<C> *data;
  ReflectorWrapConst(const ReflectorInterface<C> *_data) : data(_data) {}
  ReflectorWrapConst(const ReflectorInterface<C> &_data) : data(&_data) {}
  ReflectorWrapConst() = delete;
};

class ReflectorSubClass : public Reflector {
  reflectorInstanceConst GetReflectedInstance() const override {
    return data.instc;
  }
  reflectorInstance GetReflectedInstance() override { return data.inst; }

public:
  SubClass data;
  ReflectorSubClass(const SubClass &_data) : data(_data) {}
};

template <class C> class ReflectorBase : public Reflector {
public:
  static const reflectorStatic *GetReflector() {
    return ReflectorInterface<C>::GetReflector();
  }
  reflectorInstanceConst GetReflectedInstance() const override {
    return {GetReflector(), this};
  }
  reflectorInstance GetReflectedInstance() override {
    return {GetReflector(), this};
  }
};

#include "internal/reflector.inl"
#endif
