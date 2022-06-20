/*  Python binding classes for uni::Skeleton
    part of uni module
    Copyright 2020-2022 Lukas Cone

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
#include "uni/skeleton.hpp"
#include "datas/pythonex.hpp"

namespace UniPy {

struct Bone {
  PyObject_HEAD;
  uni::Element<const uni::Bone> item;

  static void Dealloc(Bone *self);
  static PyObject *GetTMType(Bone *self, void * = nullptr);
  static PyObject *GetTM(Bone *self, void * = nullptr);
  static PyObject *GetIndex(Bone *self, void * = nullptr);
  static PyObject *GetName(Bone *self, void * = nullptr);
  static PyObject *GetParent(Bone *self, void * = nullptr);
  static PyObject *Compare(Bone *self, Bone *other, int op);

  static PyTypeObject *GetType();
};

struct Skeleton {
  PyObject_HEAD;
  uni::Element<const uni::Skeleton> item;

  static void Dealloc(Skeleton *self);
  static PyObject *Name(Skeleton *self, void * = nullptr);
  static PyObject *Bones(Skeleton *self, void * = nullptr);

  static void InitType(PyObject *module);
  static PyTypeObject *GetType();
  static PyTypeObject *GetListType();

  static PyObject *Create(uni::SkeletonsConst &&tp);
  static Skeleton *Create(uni::Element<const uni::Skeleton> &&tp);
};
} // namespace UniPy
