#include "../uni/list_vector.hpp"
#include "../uni/python/pyskeleton.hpp"
#include <Python.h>

class BoneMock : public uni::Bone {
public:
  es::string_view name;
  size_t ID;
  TransformType tmType;
  BoneMock *parent;
  uni::RTSValue rval;
  esMatrix44 mval;

  TransformType TMType() const override { return tmType; }
  void GetTM(uni::RTSValue &out) const override { out = rval; }
  void GetTM(esMatrix44 &out) const override { out = mval; }
  const Bone *Parent() const override { return parent; }
  size_t Index() const override { return ID; }
  es::string_view Name() const override { return name; }
  operator uni::Element<const uni::Bone>() const {
    return {static_cast<const uni::Bone *>(this), false};
  }
};

class SkeletonMock : public uni::Skeleton {
public:
  uni::VectorList<uni::Bone, BoneMock> bones;

  uni::SkeletonBonesConst Bones() const override {
    return uni::SkeletonBonesConst(&bones, false);
  }

  es::string_view Name() const override { return "Sample name."; }
};

PyObject *GetSkeletonData() {
  SkeletonMock *skel = new SkeletonMock;
  skel->bones.storage.emplace_back();
  skel->bones.storage.emplace_back();

  BoneMock &bone0 = skel->bones.storage.at(0);
  bone0.name = "bone0";
  bone0.ID = 0;
  bone0.parent = nullptr;
  bone0.tmType = bone0.TMTYPE_RTS;
  bone0.rval.rotation = Vector4A16(1, 2, 3, 4);
  bone0.rval.translation = Vector4A16(5, 6, 7, 8);
  bone0.rval.scale = Vector4A16(9, 10, 11, 12);

  BoneMock &bone1 = skel->bones.storage.at(1);
  bone1.name = "bone1";
  bone1.ID = 1;
  bone1.parent = &bone0;
  bone1.tmType = bone0.TMTYPE_MATRIX;
  bone1.mval.r1 = Vector4A16(1, 2, 3, 4);
  bone1.mval.r2 = Vector4A16(5, 6, 7, 8);
  bone1.mval.r3 = Vector4A16(9, 10, 11, 12);
  bone1.mval.r4 = Vector4A16(13, 14, 15, 16);

  PyObject *retVal = PyType_GenericAlloc(UniPy::Skeleton::GetType(), 0);
  auto rtVal = reinterpret_cast<UniPy::Skeleton *>(retVal);
  rtVal->item = decltype(rtVal->item){skel};

  return retVal;
}

static PyMethodDef methods[]{
    {"get_skeleton", (PyCFunction)GetSkeletonData, METH_NOARGS, nullptr},
    {NULL},
};

PyMODINIT_FUNC Py_GCC_ATTRIBUTE((visibility("default"))) inittest_unipy() {
  PyObject *m =
      Py_InitModule3("test_unipy", methods, "UNI unit testing module.");

  if (!m)
    return;

  UniPy::Skeleton::InitType(m);
}