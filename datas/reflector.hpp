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
#include "internal/reflector_type.hpp"
#include "internal/reflector_reg.hpp"

// Usable flags: VARNAMES, TEMPLATE, SUBCLASS
// Usable enum flags: CLASS, EXTERN, size (64B or 32B or 16B or 8B)
//@numFlags: [0,n] or ENUM, numEnumFlags
#define REFLECTOR_CREATE(classname, numFlags, ...)                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER##numFlags(classname, __VA_ARGS__))

#define DECLARE_REFLECTOR                                                      \
  static const reflectorStatic *__rfPtrStatic;                                 \
  const reflectorInstanceConst _rfRetreive() const {                           \
    return {__rfPtrStatic, this};                                              \
  }                                                                            \
  const reflectorInstance _rfRetreive() { return {__rfPtrStatic, this}; }      \
  static void _rfInit();                                                       \
  auto GetReflector()                                                          \
      ->ReflectorWrap<typename std::remove_pointer<decltype(this)>::type> {    \
    return {this};                                                             \
  }                                                                            \
  auto GetReflector() const->ReflectorWrapConst<                               \
      typename std::remove_pointer<decltype(this)>::type> {                    \
    return {this};                                                             \
  }

static const reflectorStatic __null_statical = {0, 0, 0, 0, 0};

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
  virtual const reflectorInstanceConst _rfRetreive() const {
    return {&__null_statical, nullptr};
  }
  virtual const reflectorInstance _rfRetreive() {
    return {&__null_statical, nullptr};
  }
  // clang-format off
protected:
  const reflType *GetReflectedType(JenHashStrong hash) const;
  const reflType *GetReflectedType(es::string_view name) const;
  const reflType *GetReflectedType(size_t ID) const;
  ErrorType SetReflectedValue(reflType type, es::string_view value);
  ErrorType SetReflectedValueUInt(reflType type, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(reflType type, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(reflType type, double value, size_t subID = 0);

public:
  ErrorType SetReflectedValue(es::string_view name, es::string_view value);
  ErrorType SetReflectedValue(JenHashStrong hash, es::string_view value);
  ErrorType SetReflectedValue(size_t id, es::string_view value);

  ErrorType SetReflectedValueInt(es::string_view name, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(JenHashStrong hash, int64 value, size_t subID = 0);
  ErrorType SetReflectedValueInt(size_t id, int64 value, size_t subID = 0);

  ErrorType SetReflectedValueUInt(es::string_view name, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueUInt(JenHashStrong hash, uint64 value, size_t subID = 0);
  ErrorType SetReflectedValueUInt(size_t id, uint64 value, size_t subID = 0);

  ErrorType SetReflectedValueFloat(es::string_view name, double value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(JenHashStrong hash, double value, size_t subID = 0);
  ErrorType SetReflectedValueFloat(size_t id, double value, size_t subID = 0);

  size_t GetNumReflectedValues() const;
  es::string_view GetClassName() const;
  bool UseNames() const;

  std::string GetReflectedValue(size_t id) const;
  std::string GetReflectedValue(JenHashStrong hash) const;
  std::string GetReflectedValue(es::string_view name) const;

  bool IsReflectedSubClass(es::string_view name) const;
  bool IsReflectedSubClass(JenHashStrong hash) const;
  bool IsReflectedSubClass(size_t id) const;

  bool IsArray(es::string_view name) const;
  bool IsArray(JenHashStrong hash) const;
  bool IsArray(size_t id) const; 

  const SubClass GetReflectedSubClass(es::string_view name, size_t subID = 0) const;
  const SubClass GetReflectedSubClass(JenHashStrong hash, size_t subID = 0) const;
  const SubClass GetReflectedSubClass(size_t id, size_t subID = 0) const;

  const SubClass GetReflectedSubClass(es::string_view name, size_t subID = 0);
  const SubClass GetReflectedSubClass(JenHashStrong hash, size_t subID = 0);
  const SubClass GetReflectedSubClass(size_t id, size_t subID = 0);

  KVPair GetReflectedPair(size_t id, const KVPairFormat &settings = {}) const;
  KVPair GetReflectedPair(JenHashStrong hash, const KVPairFormat &settings = {}) const;
  KVPair GetReflectedPair(es::string_view name, const KVPairFormat &settings = {}) const;
  // clang-format on
};

template <class C> class ReflectorWrap : public Reflector {
  const reflectorInstanceConst _rfRetreive() const {
    return static_cast<const C *>(data)->_rfRetreive();
  }
  const reflectorInstance _rfRetreive() { return data->_rfRetreive(); }

public:
  C *data;
  ReflectorWrap(C *_data) : data(_data) {}
  ReflectorWrap(C &_data) : data(&_data) {}
  ReflectorWrap() = delete;
};

template <class C> class ReflectorWrapConst : public Reflector {
  const reflectorInstanceConst _rfRetreive() const {
    return data->_rfRetreive();
  }

public:
  const C *data;
  ReflectorWrapConst(const C *_data) : data(_data) {}
  ReflectorWrapConst(const C &_data) : data(&_data) {}
  ReflectorWrapConst() = delete;
};

class ReflectorSubClass : public Reflector {
  const reflectorInstanceConst _rfRetreive() const { return data.instc; }
  const reflectorInstance _rfRetreive() { return data.inst; }

public:
  SubClass data;
  ReflectorSubClass(const SubClass &_data) : data(_data) {}
};

#include "internal/reflector.inl"
#endif