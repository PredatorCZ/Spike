/*  Python binding definitions for uni::Skeleton
    part of uni module
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

#include "pyenum.hpp"
#include "pylist.hpp"
#include "pyskeleton.hpp"

namespace UniPy {

static const struct {
  int id;
  es::string_view name;
} boneTMTypes[]{
    {uni::Bone::TMTYPE_RTS, "TMTYPE_RTS"},
    {uni::Bone::TMTYPE_MATRIX, "TMTYPE_MATRIX"},
};

struct BoneTMTypeInfo {
  using value_type = decltype(boneTMTypes[0]);
  using iterator_type = std::add_pointer<value_type>::type;

  static constexpr const char *GetName() { return "uniBoneTMType"; }
  static constexpr const char *GetDoc() { return "Uni Bone TM Type Enum"; }
  static size_t Len() { return sizeof(boneTMTypes) / sizeof(value_type); }
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

static PyGetSet boneGetSets[] = {
    {"tm_type", (getter)Bone::GetTMType, nullptr, "Get bone transform type."},
    {"transform", (getter)Bone::GetTM, nullptr, "Get bone transformation."},
    {"name", (getter)Bone::GetName, nullptr, "Get bone name."},
    {"index", (getter)Bone::GetIndex, nullptr, "Get bone identifier."},
    {"parent", (getter)Bone::GetParent, nullptr, "Get bone parent object."},
    {NULL},
};

static PyTypeObject boneType = {
    PyVarObject_HEAD_INIT(NULL, 0)               /* init macro */
    "uni::Bone",                                 /* tp_name */
    sizeof(Bone),                                /* tp_basicsize */
    0,                                           /* tp_itemsize */
    (destructor)Bone::Dealloc,                   /* tp_dealloc */
    0,                                           /* tp_print */
    0,                                           /* tp_getattr */
    0,                                           /* tp_setattr */
    (cmpfunc)Bone::Compare,                      /* tp_compare */
    0,                                           /* tp_repr */
    0,                                           /* tp_as_number */
    0,                                           /* tp_as_sequence */
    0,                                           /* tp_as_mapping */
    0,                                           /* tp_hash */
    0,                                           /* tp_call */
    0,                                           /* tp_str */
    0,                                           /* tp_getattro */
    0,                                           /* tp_setattro */
    0,                                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT, /* tp_flags */
    "Uni Bone interface",                        /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    0,                                           /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    0,                                           /* tp_iter */
    0,                                           /* tp_iternext */
    0,                                           /* tp_methods */
    0,                                           /* tp_members */
    (PyGetSetDef *)boneGetSets,                  /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    0,                                           /* tp_init */
    0,                                           /* tp_alloc */
    0,                                           /* tp_new */
};

PyTypeObject *Bone::GetType() { return &boneType; }

void Bone::Dealloc(Bone *self) { auto t0 = std::move(self->item); }

PyObject *Bone::GetTMType(Bone *self) {
  return PyLong_FromLong(self->item->TMType());
}

PyObject *Bone::GetTM(Bone *self) {
  switch (self->item->TMType()) {
  case uni::Bone::TMTYPE_MATRIX: {
    esMatrix44 mtx;
    self->item->GetTM(mtx);
    return Py_BuildValue("((ffff)(ffff)(ffff)(ffff))", /**********************/
                         mtx.r1.X, mtx.r1.Y, mtx.r1.Z, mtx.r1.W, /************/
                         mtx.r2.X, mtx.r2.Y, mtx.r2.Z, mtx.r2.W, /************/
                         mtx.r3.X, mtx.r3.Y, mtx.r3.Z, mtx.r3.W, /************/
                         mtx.r4.X, mtx.r4.Y, mtx.r4.Z, mtx.r4.W  /************/
    );
  }
  case uni::Bone::TMTYPE_RTS: {
    uni::RTSValue rts;
    self->item->GetTM(rts);
    return Py_BuildValue("((ffff)(ffff)(ffff))", /****************************/
                         rts.translation.X, rts.translation.Y, /**************/
                         rts.translation.Z, rts.translation.W, /**************/
                         rts.rotation.X, rts.rotation.Y,       /**************/
                         rts.rotation.Z, rts.rotation.W,       /**************/
                         rts.scale.X, rts.scale.Y, rts.scale.Z, rts.scale.W ///
    );
  }
  default:
    return Py_None;
  }
}

PyObject *Bone::GetIndex(Bone *self) {
  return PyInt_FromSize_t(self->item->Index());
}

PyObject *Bone::GetName(Bone *self) {
  const auto retName = self->item->Name();
  return PyString_FromStringAndSize(retName.data(), retName.size());
}

PyObject *Bone::GetParent(Bone *self) {
  auto bne = self->item->Parent();

  if (bne) {
    Bone *rtVal = reinterpret_cast<Bone *>(PyType_GenericAlloc(&boneType, 0));
    rtVal->item = {bne, false};
    return reinterpret_cast<PyObject *>(rtVal);
  } else {
    return Py_None;
  }
}

int Bone::Compare(Bone *self, Bone *other) {
  const auto i0d = self->item.get();
  const auto i1d = other->item.get();
  const bool eq = (i0d == i1d) ||
                  (i0d->Index() == i1d->Index() && i0d->Name() == i1d->Name());
  return eq ? 0 : 1;
}

static PyGetSet skeletonGetSets[] = {
    {"name", (getter)Skeleton::Name, nullptr, "Get skeleton's name."},
    {"bones", (getter)Skeleton::Bones, nullptr, "Get skeleton's bones."},
    {NULL} /* Sentinel */
};

static constexpr size_t skeletonTypeFlags =
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT | Py_TPFLAGS_BASETYPE;

static PyTypeObject skeletonType = {
    PyVarObject_HEAD_INIT(NULL, 0)  /* init macro */
    "uni::Skeleton",                /* tp_name */
    sizeof(Skeleton),               /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)Skeleton::Dealloc,  /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    skeletonTypeFlags,              /* tp_flags*/
    "Uni skeleton interface",       /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    0,                              /* tp_methods */
    0,                              /* tp_members */
    (PyGetSetDef *)skeletonGetSets, /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
    0,                              /* tp_new */
};

PyTypeObject *Skeleton::GetType() { return &skeletonType; }

void Skeleton::Dealloc(Skeleton *self) { auto t0 = std::move(self->item); }

PyObject *Skeleton::Name(Skeleton *self) {
  auto skName = self->item->Name();
  return PyString_FromStringAndSize(skName.data(), skName.size());
}

PyObject *Skeleton::Bones(Skeleton *self) {
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
  PyAddType<BoneTMTypeEnum>(module);
  PyAddType<Bone>(module);
  PyAddType<BoneList>(module);
  PyAddType<Skeleton>(module);
  PyAddType<SkeletonList>(module);
}
} // namespace UniPy
