/*  De/Serializing Reflector data XML format

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
#include "reflector.hpp"
#include "pugi_fwd.hpp"
#include "flags.hpp"


class ReflectorXMLUtil {
  friend class ReflectorXMLUtilFriend;
public:
  enum Flags {
    Flags_ClassNode,
    Flags_StringAsAttribute,
  };
  using flag_type = es::Flags<Flags>;
  static pugi::xml_node PC_EXTERN Save(const Reflector &ri, pugi::xml_node iNode, bool asNewNode = false);
  static pugi::xml_node PC_EXTERN SaveV2(const Reflector &ri, pugi::xml_node iNode, bool asNewNode = false);
  static pugi::xml_node PC_EXTERN SaveV2a(const Reflector &ri, pugi::xml_node iNode, flag_type options = {});
  static pugi::xml_node PC_EXTERN Load(Reflector &ri, pugi::xml_node iNode, bool lookupClassNode = false);
  static pugi::xml_node PC_EXTERN LoadV2(Reflector &ri, pugi::xml_node iNode, bool lookupClassNode = false);
};
