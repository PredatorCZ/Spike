/*  a XML I/O source for Reflector class
    more info in README for PreCore Project

    Copyright 2019-2021 Lukas Cone

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

#include "reflector_xml.hpp"
#include "master_printer.hpp"
#include "pugiex.hpp"

struct ReflectedInstanceFriend : ReflectedInstance {
  void *Instance() { return instance; }
  const void *Instance() const { return constInstance; }
  const reflectorStatic *Refl() const { return rfStatic; }
};

pugi::xml_node ReflectorXMLUtil::Save(const Reflector &ri, pugi::xml_node node,
                                      bool asNewNode) {
  const reflectorStatic *stat =
      static_cast<ReflectedInstanceFriend &&>(ri.GetReflectedInstance()).Refl();
  pugi::xml_node thisNode = node;

  if (asNewNode) {
    std::string className;

    if (ri.UseNames())
      className = stat->className;
    else {
      className.resize(15);
      const auto cHash = stat->classHash.raw();
      snprintf(&className[0], 15, "h:%X", cHash);
    }

    thisNode = node.append_child(className.c_str());
  }

  for (size_t t = 0; t < stat->nTypes; t++) {
    std::string varName;

    if (ri.UseNames()) {
      varName = stat->typeNames[t];
      size_t fndBrace = varName.find('[');
      size_t fndBraceEnd = varName.find(']');

      if (fndBraceEnd != varName.npos)
        varName.pop_back();

      if (fndBrace != varName.npos)
        varName.replace(fndBrace, 1, 1, ':');
    } else {
      varName.resize(15);
      const auto cHash = stat->types[t].valueNameHash.raw();
      snprintf(&varName[0], 15, "h:%X", cHash);
    }

    pugi::xml_node cNode = thisNode.append_child(varName.c_str());

    if (ri.IsReflectedSubClass(t)) {
      if (ri.IsArray(t)) {
        const int numItems = stat->types[t].numItems;

        for (int s = 0; s < numItems; s++) {
          auto subRef = ri.GetReflectedSubClass(t, s);
          ReflectorPureWrap subCl(subRef);
          ReflectorXMLUtil::Save(
              subCl, cNode.append_child(("i:" + std::to_string(s)).c_str()),
              false);
        }

      } else {
        auto subRef = ri.GetReflectedSubClass(t);
        ReflectorPureWrap subCl(subRef);
        ReflectorXMLUtil::Save(subCl, cNode, false);
      }
    } else {
      std::string str = ri.GetReflectedValue(t);
      cNode.append_buffer(str.c_str(), str.size());
    }
  }
  return thisNode;
}

pugi::xml_node ReflectorXMLUtil::Load(Reflector &ri, pugi::xml_node node,
                                      bool lookupClassNode) {
  const reflectorStatic *stat =
      static_cast<ReflectedInstanceFriend &&>(ri.GetReflectedInstance()).Refl();
  pugi::xml_node thisNode;

  auto MakeHash = [](auto &a) -> JenHash {
    if (*a.name() == 'h' && *(a.name() + 1) == ':') {
      return JenHash(strtoul(a.name() + 2, nullptr, 16));
    } else {
      return {a.name()};
    }
  };

  if (lookupClassNode) {
    for (auto &a : node.children()) {
      if (MakeHash(a) == stat->classHash) {
        thisNode = a;
        break;
      }
    }
  } else
    thisNode = node;

  if (!thisNode.empty())
    for (auto &a : thisNode.children()) {
      ri.SetReflectedValue(MakeHash(a), a.text().as_string());
    }

  return thisNode;
}
