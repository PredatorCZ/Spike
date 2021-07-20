/*  De/Serializing Reflector data from stream

    Copyright 2020-2021 Lukas Cone

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
#include "bincore_fwd.hpp"
#include "reflector.hpp"

class ReflectorIO {
  typedef std::vector<const reflectorStatic *> classes_type;
  typedef std::vector<ReflectedEnum> enums_type;
  std::string data;
  classes_type classes;
  enums_type enums;

public:
  int PC_EXTERN Load(BinReaderRef rd);
  int PC_EXTERN Save(BinWritterRef wr);

  void AddClass(const reflectorStatic *ref) { classes.push_back(ref); }
  template <class C> void AddClass() {
    classes.push_back(GetReflectedClass<C>());
  }
  void AddEnum(const ReflectedEnum &enumRef) { enums.push_back(enumRef); }
  template <class C> void AddEnum() { enums.push_back(GetReflectedEnum<C>()); }

  const classes_type &Classes() const { return classes; }
  const enums_type &Enums() const { return enums; }
};

class ReflectorBinUtil {
  friend class ReflectorBinUtilFriend;
  static const reflType *Find(Reflector &ri, JenHash hash);

public:
  static int PC_EXTERN Save(const Reflector &ri, BinWritterRef wr);
  static int PC_EXTERN Load(Reflector &ri, BinReaderRef rd);
};
