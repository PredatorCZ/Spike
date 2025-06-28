/*  Contains macros/classes for reflection of classes/enums

    Copyright 2018-2024 Lukas Cone

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
#include "spike/util/macroLoop.hpp"
#include "spike/util/supercore.hpp"

#include "detail/reflector_class.hpp"
#include "detail/reflector_enum.hpp"
#include "detail/reflector_type.hpp"

#include <concepts>
#include <stdexcept>
#include <string>
#include <variant>

class ReflectorPureWrap;

using ReflectorInputValue =
    std::variant<uint64, int64, float, double, std::string_view>;

class ReflectorMember {
public:
  ReflectorMember(ReflectedInstance data_, size_t id_) : data(data_), id(id_) {}

  struct KVPairFormat {
    bool aliasName = false;
    bool formatValue = false;

    constexpr KVPairFormat() {}
  };

  struct KVPair {
    std::string_view name;
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

  bool IsReflectedSubClass() const;
  bool IsArray() const;
  size_t Size() const;
  operator bool() const { return data; }

  template <std::unsigned_integral C>
  ErrorType ReflectValue(C value, size_t index = -1) {
    return ReflectValue(ReflectorInputValue(uint64(value)), index);
  }

  template <std::signed_integral C>
  ErrorType ReflectValue(C value, size_t index = -1) {
    return ReflectValue(ReflectorInputValue(int64(value)), index);
  }

  ErrorType PC_EXTERN ReflectValue(ReflectorInputValue value,
                                   size_t index = -1);
  std::string PC_EXTERN ReflectedValue(size_t index = -1) const;

  ReflectorMember &operator=(ReflectorInputValue value) {
    lastError = ReflectValue(value);
    return *this;
  }

  operator std::string() const { return ReflectedValue(); }

  ReflectorPureWrap PC_EXTERN ReflectedSubClass(size_t index = 0) const;

  KVPair ReflectedPair(KVPairFormat settings = {}) const;

  ErrorType lastError = ErrorType::None;

protected:
  ReflectedInstance data;
  size_t id;
};

class ReflectorMemberIterator {
public:
  explicit ReflectorMemberIterator(ReflectedInstance data_, size_t num_,
                                   size_t id_)
      : data(data_), id(id_), num(num_) {}
  ReflectorMemberIterator &operator++() {
    id++;
    return *this;
  }
  ReflectorMemberIterator operator++(int) {
    ReflectorMemberIterator retval = *this;
    ++(*this);
    return retval;
  }
  ReflectorMemberIterator &operator--() {
    id++;
    return *this;
  }
  ReflectorMemberIterator operator--(int) {
    ReflectorMemberIterator retval = *this;
    --(*this);
    return retval;
  }
  bool operator==(ReflectorMemberIterator other) const {
    return id == other.id;
  }
  bool operator!=(ReflectorMemberIterator other) const {
    return !(*this == other);
  }
  ReflectorMember operator*() const { return ReflectorMember{data, id}; }

private:
  ReflectedInstance data;
  size_t id = 0;
  size_t num;
};

class Reflector {
  friend class ReflectorFriend;

public:
  virtual ~Reflector() = default;

  size_t NumReflectedValues() const;
  std::string_view ClassName() const;
  ReflectorMember PC_EXTERN operator[](JenHash memberName) const;
  ReflectorMember operator[](size_t id) const;

  ReflectorMemberIterator begin() const {
    return ReflectorMemberIterator{GetReflectedInstance(), NumReflectedValues(),
                                   0};
  }
  ReflectorMemberIterator end() const {
    return ReflectorMemberIterator{GetReflectedInstance(), NumReflectedValues(),
                                   NumReflectedValues()};
  }
  ReflectorMemberIterator begin() {
    return ReflectorMemberIterator{GetReflectedInstance(), NumReflectedValues(),
                                   0};
  }
  ReflectorMemberIterator end() {
    return ReflectorMemberIterator{GetReflectedInstance(), NumReflectedValues(),
                                   NumReflectedValues()};
  }

private:
  virtual ReflectedInstance GetReflectedInstance() const = 0;
  ReflectedInstance GetReflectedInstance();
};

// Wrap around concrete class instance
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

// Wrap around raw data
class ReflectorPureWrap : public Reflector {
  ReflectedInstance GetReflectedInstance() const override { return data; }

public:
  ReflectedInstance data;
  ReflectorPureWrap(ReflectedInstance _data) : data(_data) {}
};

// Used in inheritance
template <class C> class ReflectorBase : public Reflector {
public:
  ReflectedInstance GetReflectedInstance() const override {
    return {GetReflectedClass<C>(), {static_cast<const C *>(this)}};
  }
};

#include "detail/reflector.inl"
