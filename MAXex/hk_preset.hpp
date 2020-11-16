/*  Saving external HavokPreset into xml conf

    Copyright 2020 Lukas Cone

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
#include <list>

static auto GetCorrectionMatrix(const Matrix3 &value) {
  static const char cormatElements[] = "XYZ";
  std::string retVal;

  for (uint32 r = 0; r < 3; r++) {
    for (uint32 e = 0; e < 3; e++) {
      if (value[r][e] != 0.0f) {
        if (value[r][e] < 0.0f) {
          retVal.push_back('-');
        }

        retVal.push_back(cormatElements[e]);
      }
    }
  }

  return retVal;
}

template <class C>
static void SaveHKPreset(const C &cls, const pugi::char_t *presetName,
                         const std::list<std::string> &extensions,
                         pugi::xml_node node) {
  auto presNode = node.append_child("HavokPreset");
  presNode.append_attribute("name").set_value(presetName);
  char tmpBuffer_[0x10] = {};
  snprintf(tmpBuffer_, sizeof(tmpBuffer_), "%.6g", cls.objectScale);
  presNode.append_child("scale").append_buffer(tmpBuffer_, strlen(tmpBuffer_));
  auto cMat = GetCorrectionMatrix(cls.corMat);
  presNode.append_child("matrix").append_buffer(cMat.data(),cMat.size());
  auto exts = presNode.append_child("extensions");

  for (auto &e : extensions) {
    exts.append_child(e.data());
  }
}
