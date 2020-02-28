
#pragma once
#include "reflector.hpp"
#include "pugixml.hpp"


class ReflectorXMLUtil {
  friend class ReflectorXMLUtilFriend;
public:
  static pugi::xml_node Save(const Reflector &ri, pugi::xml_node iNode, bool asNewNode = false);
  static pugi::xml_node Load(Reflector &ri, pugi::xml_node iNode, bool lookupClassNode = false);
};