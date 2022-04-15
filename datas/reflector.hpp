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

#include "internal/reflector_class.hpp"
#include "internal/reflector_enum.hpp"
#include "internal/reflector_type.hpp"

class Reflector {
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

  virtual ~Reflector() = default;

  size_t GetNumReflectedValues() const;
  es::string_view GetClassName() const;

  bool IsReflectedSubClass(JenHash hashName) const;
  bool IsReflectedSubClass(size_t id) const;

  bool IsArray(JenHash hashName) const;
  bool IsArray(size_t id) const;

  // clang-format off
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

  std::string PC_EXTERN GetReflectedValue(size_t id) const;
  std::string GetReflectedValue(JenHash hashName) const;

  std::string PC_EXTERN GetReflectedValue(size_t id, size_t subID) const;
  std::string GetReflectedValue(JenHash hashName, size_t subID) const;

  std::string PC_EXTERN GetReflectedValue(size_t id, size_t subID, size_t element) const;
  std::string GetReflectedValue(JenHash hashName, size_t subID, size_t element) const;

  ReflectedInstance GetReflectedSubClass(JenHash hashName, size_t subID = 0) const;
  ReflectedInstance PC_EXTERN GetReflectedSubClass(size_t id, size_t subID = 0) const;

  ReflectedInstance GetReflectedSubClass(JenHash hashName, size_t subID = 0);
  ReflectedInstance PC_EXTERN GetReflectedSubClass(size_t id, size_t subID = 0);

  KVPair GetReflectedPair(size_t id, const KVPairFormat &settings = {}) const;
  KVPair GetReflectedPair(JenHash hashName, const KVPairFormat &settings = {}) const;

protected:
  const ReflType PC_EXTERN *GetReflectedType(JenHash hashName) const;
  const ReflType *GetReflectedType(size_t ID) const;
  ErrorType PC_EXTERN SetReflectedValue(ReflType type, es::string_view value);
  ErrorType PC_EXTERN SetReflectedValue(ReflType type, es::string_view value, size_t subID);
  ErrorType PC_EXTERN SetReflectedValue(ReflType type, es::string_view value, size_t subID, size_t element);
  ErrorType PC_EXTERN SetReflectedValueUInt(ReflType type, uint64 value, size_t subID = 0);
  ErrorType PC_EXTERN SetReflectedValueInt(ReflType type, int64 value, size_t subID = 0);
  ErrorType PC_EXTERN SetReflectedValueFloat(ReflType type, double value, size_t subID = 0);
  // clang-format on
private:
  virtual ReflectedInstance GetReflectedInstance() const = 0;
  ReflectedInstance GetReflectedInstance();
};

template <class C> class ReflectorWrap : public Reflector {
  ReflectedInstance GetReflectedInstance() const override {
    return {GetReflectedClass<std::decay_t<C>>(), &data};
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
