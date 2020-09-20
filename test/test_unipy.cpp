#include "../datas/string_view.hpp"
#include "../uni/list_vector.hpp"
#include "../uni/python/pymotion.hpp"
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
  std::string Name() const override { return name; }
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

  std::string Name() const override { return "Sample name."; }
};

class SkeletonMock2 : public SkeletonMock {
public:
  std::string Name() const override { return "skeleton_1"; }
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

  PyObject *obj = reinterpret_cast<PyObject *>(
      UniPy::Skeleton::Create(uni::Element<const uni::Skeleton>(skel)));

  return obj;
}

class SkeletonList : public uni::List<uni::Skeleton> {
public:
  using Base = uni::List<uni::Skeleton>;
  std::vector<std::unique_ptr<uni::Skeleton>> skels;

  size_t Size() const override { return skels.size(); }
  Base::const_type At(size_t id) const override {
    return {skels.at(id).get(), false};
  }
};

PyObject *GetSkeletonList() {
  SkeletonMock *skel0 = new SkeletonMock;
  SkeletonMock2 *skel1 = new SkeletonMock2;

  SkeletonList *skelList = new SkeletonList;

  skelList->skels.emplace_back(skel0);
  skelList->skels.emplace_back(skel1);

  return UniPy::Skeleton::Create({skelList, true});
}

template <class C> class MotionTrackMock : public uni::MotionTrack {
public:
  uni::MotionTrack::TrackType_e ttype;
  size_t boneIndex;
  std::vector<C> frames;

  TrackType_e TrackType() const override { return ttype; }
  size_t BoneIndex() const override { return boneIndex; }
  void GetValue(C &output, float time) const override { output = frames[time]; }
};

class MotionTracksMock : public uni::List<uni::MotionTrack> {
public:
  std::vector<std::unique_ptr<uni::MotionTrack>> storage;

  size_t Size() const override { return storage.size(); }
  uni::List<uni::MotionTrack>::const_type At(size_t id) const override {
    return {storage[id].get(), false};
  }
};

class MotionMock : public uni::Motion {
public:
  MotionTracksMock tcks;
  mutable uint32 frameRate = 30;

  std::string Name() const override { return "Sample motion."; }
  void FrameRate(uint32 fps) const override { frameRate = fps; }
  uint32 FrameRate() const override { return frameRate; }
  float Duration() const override { return 1.5f; }
  uni::MotionTracksConst Tracks() const override { return {&tcks, false}; }
  uni::Motion::MotionType_e MotionType() const override {
    return uni::Motion::Delta;
  }
};

PyObject *GetMotion() {
  auto tck0 = new MotionTrackMock<uni::RTSValue>;
  tck0->boneIndex = 1;
  tck0->ttype = uni::MotionTrack::PositionRotationScale;
  tck0->frames.emplace_back(Vector4A16(1, 2, 3, 4), Vector4A16(5, 6, 7, 8),
                            Vector4A16(9, 10, 11, 12));
  tck0->frames.emplace_back(Vector4A16(13, 14, 15, 16),
                            Vector4A16(17, 18, 19, 20),
                            Vector4A16(21, 22, 23, 24));

  auto tck1 = new MotionTrackMock<esMatrix44>;
  tck1->boneIndex = 2;
  tck1->ttype = uni::MotionTrack::Matrix;
  tck1->frames.emplace_back(Vector4A16(1, 2, 3, 4), Vector4A16(5, 6, 7, 8),
                            Vector4A16(9, 10, 11, 12),
                            Vector4A16(13, 14, 15, 16));
  tck1->frames.emplace_back(
      Vector4A16(17, 18, 19, 20), Vector4A16(21, 22, 23, 24),
      Vector4A16(25, 26, 27, 28), Vector4A16(29, 30, 31, 32));

  auto tck2 = new MotionTrackMock<Vector4A16>;
  tck2->boneIndex = 3;
  tck2->ttype = uni::MotionTrack::Position;
  tck2->frames.emplace_back(Vector4A16(1, 2, 3, 1));
  tck2->frames.emplace_back(Vector4A16(3, 2, 1, 1));

  auto tck3 = new MotionTrackMock<Vector4A16>;
  tck3->boneIndex = 4;
  tck3->ttype = uni::MotionTrack::Rotation;
  tck3->frames.emplace_back(Vector4A16(0.25f, 0.25f, 0.25f, 0.25f));
  tck3->frames.emplace_back(Vector4A16(0, 0, 1, 0));

  auto tck4 = new MotionTrackMock<Vector4A16>;
  tck4->boneIndex = 5;
  tck4->ttype = uni::MotionTrack::Scale;
  tck4->frames.emplace_back(Vector4A16(0.5f, 0.5f, 0.5f, 0));
  tck4->frames.emplace_back(Vector4A16(1, 1, 1, 0));

  auto tck5 = new MotionTrackMock<float>;
  tck5->boneIndex = 6;
  tck5->ttype = uni::MotionTrack::SingleFloat;
  tck5->frames.emplace_back(20);
  tck5->frames.emplace_back(50);

  auto mot = new MotionMock;
  mot->tcks.storage.emplace_back(tck0);
  mot->tcks.storage.emplace_back(tck1);
  mot->tcks.storage.emplace_back(tck2);
  mot->tcks.storage.emplace_back(tck3);
  mot->tcks.storage.emplace_back(tck4);
  mot->tcks.storage.emplace_back(tck5);

  PyObject *obj =
      reinterpret_cast<PyObject *>(UniPy::Motion::Create({mot, true}));

  return obj;
}

static PyMethodDef methods[]{
    {"get_skeleton", (PyCFunction)GetSkeletonData, METH_NOARGS, nullptr},
    {"get_skeletons", (PyCFunction)GetSkeletonList, METH_NOARGS, nullptr},
    {"get_motion", (PyCFunction)GetMotion, METH_NOARGS, nullptr},
    {NULL},
};

PyMODINIT_FUNC Py_GCC_ATTRIBUTE((visibility("default"))) inittest_unipy() {
  PyObject *m =
      Py_InitModule3("test_unipy", methods, "UNI unit testing module.");

  if (!m)
    return;

  UniPy::Skeleton::InitType(m);
  UniPy::Motion::InitType(m);
}
