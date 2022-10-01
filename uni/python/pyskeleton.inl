/*  Python binding definitions for uni::Skeleton
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

#include "datas/python/matrix44.hpp"
#include "pyenum.hpp"
#include "pylist.hpp"
#include "pyrts.hpp"
#include "pyskeleton.hpp"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

namespace UniPy {

static const struct {
  int id;
  std::string_view name;
} boneTMTypes[]{
    {uni::TransformType::TMTYPE_RTS, "TMTYPE_RTS"},
    {uni::TransformType::TMTYPE_MATRIX, "TMTYPE_MATRIX"},
};

struct BoneTMTypeInfo {
  using value_type = decltype(boneTMTypes[0]);
  using iterator_type = std::add_pointer<value_type>::type;

  static constexpr const char *GetName() { return "uniBoneTMType"; }
  static constexpr const char *GetDoc() { return "Uni Bone TM Type Enum"; }
  static size_t Len(PyObject *) {
    return sizeof(boneTMTypes) / sizeof(value_type);
  }
  static iterator_type begin() { return std::begin(boneTMTypes); }
  static iterator_type end() { return std::end(boneTMTypes); }
};

using BoneTMTypeEnum = Enum<BoneTMTypeInfo>;

struct BoneListInfo {
  using item_type = uni::Bone;
  using wrap_type = Bone;
  static constexpr const char *GetName() { return "uni::BoneList"; }
  static constexpr const char *GetDoc() { return "Uni Bone iterator/list"; }
};

using BoneList = List<BoneListInfo>;

struct SkeletonListInfo {
  using item_type = uni::Skeleton;
  using wrap_type = Skeleton;
  static constexpr const char *GetName() { return "uni::SkeletonList"; }
  static constexpr const char *GetDoc() { return "Uni Skeleton iterator/list"; }
};

using SkeletonList = List<SkeletonListInfo>;

PyTypeObject *Bone::GetType() {
  static PyGetSetDef boneGetSets[] = {
      {"tm_type", (getter)Bone::GetTMType, nullptr, "Get bone transform type."},
      {"transform", (getter)Bone::GetTM, nullptr, "Get bone transformation."},
      {"name", (getter)Bone::GetName, nullptr, "Get bone name."},
      {"index", (getter)Bone::GetIndex, nullptr, "Get bone identifier."},
      {"parent", (getter)Bone::GetParent, nullptr, "Get bone parent object."},
      {NULL},
  };

  static PyTypeObject boneType{
      .tp_name = "uni::Bone",
      .tp_basicsize = sizeof(Bone),
      .tp_dealloc = (destructor)Bone::Dealloc,
      .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT,
      .tp_doc = "Uni Bone interface",
      .tp_richcompare = (richcmpfunc)Bone::Compare,
      .tp_getset = (PyGetSetDef *)boneGetSets,
  };

  return &boneType;
}

void Bone::Dealloc(Bone *self) {
  auto t0 = std::move(self->item);
  Py_TYPE(self)->tp_free(self);
}

PyObject *Bone::GetTMType(Bone *self, void *) {
  return PyLong_FromLong(self->item->TMType());
}

PyObject *Bone::GetTM(Bone *self, void *) {
  switch (self->item->TMType()) {
  case uni::TransformType::TMTYPE_MATRIX: {
    es::Matrix44 mtx;
    self->item->GetTM(mtx);
    return Py_BuildValue(mtx);
  }
  case uni::TransformType::TMTYPE_RTS: {
    uni::RTSValue rts;
    self->item->GetTM(rts);
    return Py_BuildValue(rts);
  }
  default:
    Py_RETURN_NONE;
  }
}

PyObject *Bone::GetIndex(Bone *self, void *) {
  return PyLong_FromSize_t(self->item->Index());
}

PyObject *Bone::GetName(Bone *self, void *) {
  const auto retName = self->item->Name();
  return PyUnicode_FromStringAndSize(retName.data(), retName.size());
}

PyObject *Bone::GetParent(Bone *self, void *) {
  auto bne = self->item->Parent();

  if (bne) {
    Bone *rtVal =
        reinterpret_cast<Bone *>(PyType_GenericAlloc(Bone::GetType(), 0));
    rtVal->item = {bne, false};
    return reinterpret_cast<PyObject *>(rtVal);
  } else {
    Py_RETURN_NONE;
  }
}

PyObject *Bone::Compare(Bone *self, Bone *other, int op) {
  if (!(Py_TYPE(self) == GetType() && Py_TYPE(other) == GetType())) {
    Py_RETURN_NOTIMPLEMENTED;
  }

  if (!(op == Py_EQ || op == Py_NE)) {
    Py_RETURN_NOTIMPLEMENTED;
  }

  const auto i0d = self->item.get();
  const auto i1d = other->item.get();
  bool eq = (i0d == i1d) ||
            (i0d->Index() == i1d->Index() && i0d->Name() == i1d->Name());
  if (op == Py_NE) {
    eq = !eq;
  }

  auto retVal = eq ? Py_True : Py_False;
  Py_INCREF(retVal);

  return retVal;
}

PyTypeObject *Skeleton::GetType() {
  static PyGetSetDef skeletonGetSets[] = {
      {"name", (getter)Skeleton::Name, nullptr, "Get skeleton's name."},
      {"bones", (getter)Skeleton::Bones, nullptr, "Get skeleton's bones."},
      {NULL} /* Sentinel */
  };

  static constexpr size_t skeletonTypeFlags =
      Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT | Py_TPFLAGS_BASETYPE;

  static PyTypeObject skeletonType{
      .tp_name = "uni::Skeleton",
      .tp_basicsize = sizeof(Skeleton),
      .tp_dealloc = (destructor)Skeleton::Dealloc,
      .tp_flags = skeletonTypeFlags,
      .tp_doc = "Uni skeleton interface",
      .tp_getset = (PyGetSetDef *)skeletonGetSets,
  };

  return &skeletonType;
}

void Skeleton::Dealloc(Skeleton *self) {
  auto t0 = std::move(self->item);
  Py_TYPE(self)->tp_free(self);
}

PyObject *Skeleton::Name(Skeleton *self, void *) {
  auto skName = self->item->Name();
  return PyUnicode_FromStringAndSize(skName.data(), skName.size());
}

PyObject *Skeleton::Bones(Skeleton *self, void *) {
  return BoneList::Create(self->item->Bones());
}

Skeleton *Skeleton::Create(uni::Element<const uni::Skeleton> &&tp) {
  auto obj = reinterpret_cast<Skeleton *>(
      PyType_GenericNew(GetType(), nullptr, nullptr));
  obj->item = std::move(tp);
  return obj;
}

PyObject *Skeleton::Create(uni::SkeletonsConst &&tp) {
  return SkeletonList::Create(std::move(tp));
}

void Skeleton::InitType(PyObject *module) {
  PyAddTypes<BoneTMTypeEnum, Bone, BoneList, Skeleton, SkeletonList>(module);
}
} // namespace UniPy
#pragma GCC diagnostic pop
