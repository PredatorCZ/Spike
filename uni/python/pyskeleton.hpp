#pragma once
#include "../skeleton.hpp"
#include <Python.h>

namespace UniPy {

struct BoneTMType {
  PyObject_HEAD;
  PyObject *tmVector4;
  PyObject *tmMatrix44;

  static void Dealloc(BoneTMType *self);
  static PyObject* New(PyTypeObject *type, PyObject *args, PyObject *kwds);
  static size_t Len(BoneTMType *self);
  static PyObject *Subscript(BoneTMType *self, PyObject *index);
  PyObject *SubscriptRaw(size_t index);
};

struct Bone {
  PyObject_HEAD;
  uni::Element<const uni::Bone> bone;

  static void Dealloc(Bone *self);
  static PyObject *GetTMType(Bone *self);
  static PyObject *GetTM(Bone *self);
  static PyObject *GetIndex(Bone *self);
  static PyObject *GetName(Bone *self);
  static PyObject *GetParent(Bone *self);
};

struct BoneList {
  PyObject_HEAD;
  uni::SkeletonBonesConst bones;
  size_t iterPos;

  static void Dealloc(BoneList *self);
  static size_t Len(BoneList *self);
  static PyObject *Subscript(BoneList *self, PyObject *index);
  PyObject *SubscriptRaw(size_t index);
  static PyObject *Iter(BoneList *self);
  static PyObject *IterNext(BoneList *self);
};

struct Skeleton {
  PyObject_HEAD;
  uni::Element<const uni::Skeleton> skeleton;

  static void Dealloc(Skeleton *self);
  static PyObject *Name(Skeleton *self);
  static PyObject *Bones(Skeleton *self);

  static void InitType(PyObject *module);
  static PyTypeObject *GetType();
};
} // namespace UniPy