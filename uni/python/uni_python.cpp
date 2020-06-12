#include "pyskeleton.hpp"
#include <structmember.h>

namespace UniPy {

static PyMappingMethods boneTMTypeMappingMethods[] = {
    (lenfunc)BoneTMType::Len,
    (binaryfunc)BoneTMType::Subscript,
    0,
};

static PyMemberDef boneTMTypeMembers[] = {
    {"TMTYPE_RTS", T_INT, offsetof(BoneTMType, tmVector4), READONLY, ""},
    {"TMTYPE_MATRIX", T_INT, offsetof(BoneTMType, tmMatrix44), READONLY, ""},
    {NULL},
};

static PyTypeObject boneTMTypeType = {
    PyVarObject_HEAD_INIT(NULL, 0) /* init macro */
    "uniBoneTMType",               /* tp_name */
    sizeof(BoneTMType),            /* tp_basicsize */
    0,                             /* tp_itemsize */
    0,                             /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    boneTMTypeMappingMethods,      /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    "Uni Bone TM Type Enum",       /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    0,                             /* tp_methods */
    boneTMTypeMembers,             /* tp_members */
    0,                             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    0,                             /* tp_init */
    0,                             /* tp_alloc */
    BoneTMType::New,               /* tp_new */
};

void BoneTMType::Dealloc(BoneTMType *self) {
  Py_TYPE(self)->tp_free(reinterpret_cast<PyObject *>(self));
}

PyObject *BoneTMType::New(PyTypeObject *type, PyObject *, PyObject *) {
  BoneTMType *self = reinterpret_cast<BoneTMType *>(type->tp_alloc(type, 0));

  if (self) {
    self->tmMatrix44 = uni::Bone::TMTYPE_MATRIX;
    self->tmVector4 = uni::Bone::TMTYPE_RTS;
  }

  return reinterpret_cast<PyObject *>(self);
}

size_t BoneTMType::Len(BoneTMType *) { return 2; }

PyObject *BoneTMType::Subscript(BoneTMType *self, PyObject *index) {
  return self->SubscriptRaw(PyInt_AsSsize_t(index));
}

PyObject *BoneTMType::SubscriptRaw(size_t index) {
  switch (index) {
  case 0:
    return PyString_FromString("TMTYPE_RTS");
  case 1:
    return PyString_FromString("TMTYPE_MATRIX");
  default:
    PyErr_SetString(PyExc_IndexError, "index out of range");
    return nullptr;
  }
}

static PyMethodDef boneMethods[] = {
    {"tm_type", (PyCFunction)Bone::GetTMType, METH_NOARGS,
     "Get bone transform type."},
    {"transform", (PyCFunction)Bone::GetTM, METH_NOARGS,
     "Get bone transformation."},
    {"name", (PyCFunction)Bone::GetName, METH_NOARGS, "Get bone name."},
    {"index", (PyCFunction)Bone::GetIndex, METH_NOARGS, "Get bone identifier."},
    {"parent", (PyCFunction)Bone::GetParent, METH_NOARGS,
     "Get bone parent object."},
    {NULL},
};

static PyTypeObject boneType = {
    PyVarObject_HEAD_INIT(NULL, 0)               /* init macro */
    "uni::Bone",                                 /* tp_name */
    sizeof(Bone),                                /* tp_basicsize */
    0,                                           /* tp_itemsize */
    0,                                           /* tp_dealloc */
    0,                                           /* tp_print */
    0,                                           /* tp_getattr */
    0,                                           /* tp_setattr */
    0,                                           /* tp_compare */
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
    boneMethods,                                 /* tp_methods */
    0,                                           /* tp_members */
    0,                                           /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    0,                                           /* tp_init */
    0,                                           /* tp_alloc */
    0,                                           /* tp_new */
};

void Bone::Dealloc(Bone *self) { std::move(self->bone); }

PyObject *Bone::GetTMType(Bone *self) {
  BoneTMType *rtVal = reinterpret_cast<BoneTMType *>(
      PyType_GenericNew(&boneTMTypeType, nullptr, nullptr));

  return rtVal->SubscriptRaw(self->bone->TMType());
}

PyObject *Bone::GetTM(Bone *self) {
  switch (self->bone->TMType()) {
  case uni::Bone::TMTYPE_MATRIX: {
    esMatrix44 mtx;
    self->bone->GetTM(mtx);
    return Py_BuildValue("((ffff)(ffff)(ffff)(ffff))", /**********************/
                         mtx.r1.X, mtx.r1.Y, mtx.r1.Z, mtx.r1.W, /************/
                         mtx.r2.X, mtx.r2.Y, mtx.r2.Z, mtx.r2.W, /************/
                         mtx.r3.X, mtx.r3.Y, mtx.r3.Z, mtx.r3.W, /************/
                         mtx.r4.X, mtx.r4.Y, mtx.r4.Z, mtx.r4.W  /************/
    );
  }
  case uni::Bone::TMTYPE_RTS: {
    uni::RTSValue rts;
    self->bone->GetTM(rts);
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
  return PyInt_FromSize_t(self->bone->Index());
}

PyObject *Bone::GetName(Bone *self) {
  const auto retName = self->bone->Name();
  return PyString_FromStringAndSize(retName.data(), retName.size());
}

PyObject *Bone::GetParent(Bone *self) {
  auto bne = self->bone->Parent();

  if (bne) {
    Bone *rtVal = reinterpret_cast<Bone *>(PyType_GenericAlloc(&boneType, 0));
    rtVal->bone = {bne, false};
    return reinterpret_cast<PyObject *>(rtVal);
  } else {
    return Py_None;
  }
}

static PyMappingMethods boneListMappingMethods[] = {
    (lenfunc)BoneList::Len,
    (binaryfunc)BoneList::Subscript,
    0,
};

static PyTypeObject boneListType = {
    PyVarObject_HEAD_INIT(NULL, 0)               /* init macro */
    "uni::BoneList",                             /* tp_name */
    sizeof(BoneList),                            /* tp_basicsize */
    0,                                           /* tp_itemsize */
    0,                                           /* tp_dealloc */
    0,                                           /* tp_print */
    0,                                           /* tp_getattr */
    0,                                           /* tp_setattr */
    0,                                           /* tp_compare */
    0,                                           /* tp_repr */
    0,                                           /* tp_as_number */
    0,                                           /* tp_as_sequence */
    boneListMappingMethods,                      /* tp_as_mapping */
    0,                                           /* tp_hash */
    0,                                           /* tp_call */
    0,                                           /* tp_str */
    0,                                           /* tp_getattro */
    0,                                           /* tp_setattro */
    0,                                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT, /* tp_flags */
    "Uni Bones iterator/list",                   /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    0,                                           /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    (getiterfunc)BoneList::Iter,                 /* tp_iter */
    (iternextfunc)BoneList::IterNext,            /* tp_iternext */
    0,                                           /* tp_methods */
    0,                                           /* tp_members */
    0,                                           /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    0,                                           /* tp_init */
    0,                                           /* tp_alloc */
    0,                                           /* tp_new */
};

void BoneList::Dealloc(BoneList *self) { std::move(self->bones); }

size_t BoneList::Len(BoneList *self) { return self->bones->Size(); }

PyObject *BoneList::Subscript(BoneList *self, PyObject *index) {
  const auto id = PyInt_AsSsize_t(index);
  return self->SubscriptRaw(id);
}

PyObject *BoneList::SubscriptRaw(size_t index) {
  Bone *rtVal = reinterpret_cast<Bone *>(PyType_GenericAlloc(&boneType, 0));
  rtVal->bone = bones->At(index);

  return reinterpret_cast<PyObject *>(rtVal);
}

PyObject *BoneList::Iter(BoneList *self) {
  self->iterPos = 0;
  return reinterpret_cast<PyObject *>(self);
}

PyObject *BoneList::IterNext(BoneList *self) {
  if (self->iterPos < self->Len(self)) {
    return self->SubscriptRaw(self->iterPos++);
  } else {
    PyErr_SetNone(PyExc_StopIteration);
    return nullptr;
  }
}

static PyMethodDef skeletonMethods[] = {
    {"name", (PyCFunction)Skeleton::Name, METH_NOARGS, "Get skeleton's name."},
    {"bones", (PyCFunction)Skeleton::Bones, METH_NOARGS,
     "Get skeleton's bones."},
    {NULL} /* Sentinel */
};

static constexpr size_t skeletonTypeFlags =
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT | Py_TPFLAGS_BASETYPE;

static PyTypeObject skeletonType = {
    PyVarObject_HEAD_INIT(NULL, 0) /* init macro */
    "uni::Skeleton",               /* tp_name */
    sizeof(Skeleton),              /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)Skeleton::Dealloc, /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    skeletonTypeFlags,             /* tp_flags*/
    "Uni skeleton interface",      /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    skeletonMethods,               /* tp_methods */
    0,                             /* tp_members */
    0,                             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    0,                             /* tp_init */
    0,                             /* tp_alloc */
    0,                             /* tp_new */
};

void Skeleton::InitType(PyObject *module) {
  if (PyType_Ready(&boneTMTypeType) < 0)
    return;
  Py_INCREF(&boneTMTypeType);
  PyModule_AddObject(module, boneTMTypeType.tp_name,
                     (PyObject *)&boneTMTypeType);

  if (PyType_Ready(&boneType) < 0)
    return;
  Py_INCREF(&boneType);
  PyModule_AddObject(module, boneType.tp_name, (PyObject *)&boneType);

  if (PyType_Ready(&boneListType) < 0)
    return;
  Py_INCREF(&boneListType);
  PyModule_AddObject(module, boneListType.tp_name, (PyObject *)&boneListType);

  if (PyType_Ready(&skeletonType) < 0)
    return;
  Py_INCREF(&skeletonType);
  PyModule_AddObject(module, skeletonType.tp_name, (PyObject *)&skeletonType);
}

PyTypeObject *Skeleton::GetType() { return &skeletonType; }

void Skeleton::Dealloc(Skeleton *self) { auto t0 = std::move(self->skeleton); }

PyObject *Skeleton::Name(Skeleton *self) {
  auto skName = self->skeleton->Name();
  return PyString_FromStringAndSize(skName.data(), skName.size());
}

PyObject *Skeleton::Bones(Skeleton *self) {
  BoneList *rtVal =
      reinterpret_cast<BoneList *>(PyType_GenericAlloc(&boneListType, 0));
  rtVal->bones = self->skeleton->Bones();

  return reinterpret_cast<PyObject *>(rtVal);
}

} // namespace UniPy