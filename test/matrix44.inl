#include "../datas/matrix44.hpp"
#include "../datas/unit_testing.hpp"

int test_matrix44_00() {
  esMatrix44 mtx({1, 2, 3, 0}, {5, 6, 7, 0}, {9, 10, 11, 0}, {13, 14, 15, 16});
  mtx.Transpose();

  TEST_EQUAL(mtx.r1, Vector4A16(1, 5, 9, 0));
  TEST_EQUAL(mtx.r2, Vector4A16(2, 6, 10, 0));
  TEST_EQUAL(mtx.r3, Vector4A16(3, 7, 11, 0));
  TEST_EQUAL(mtx.r4, Vector4A16(13, 14, 15, 16));

  mtx.MakeIdentity();

  TEST_EQUAL(mtx.r1, Vector4A16(1, 0, 0, 0));
  TEST_EQUAL(mtx.r2, Vector4A16(0, 1, 0, 0));
  TEST_EQUAL(mtx.r3, Vector4A16(0, 0, 1, 0));
  TEST_EQUAL(mtx.r4, Vector4A16(0, 0, 0, 1));

  Vector4A16 qt, pos, scale;
  
  mtx.Decompose(pos, qt, scale);

  TEST_EQUAL(qt, Vector4A16(0, 0, 0, 1));
  TEST_EQUAL(pos, Vector4A16(0, 0, 0, 1));
  TEST_EQUAL(scale, Vector4A16(1, 1, 1, 0));

  pos = Vector4A16(10, 20, 30, 1);
  scale = Vector4A16(2, 3, 4, 0);

  mtx.Compose(pos, qt, scale);

  TEST_EQUAL(mtx.r1, Vector4A16(2, 0, 0, 0));
  TEST_EQUAL(mtx.r2, Vector4A16(0, 3, 0, 0));
  TEST_EQUAL(mtx.r3, Vector4A16(0, 0, 4, 0));
  TEST_EQUAL(mtx.r4, Vector4A16(10, 20, 30, 1));

  return 0;
}

int test_matrix44_01() {
  Vector4A16 qt(-0.3062f, 0.1768f, -0.3062f, 0.8839f);
  esMatrix44 mtx(qt);
  mtx.Transpose();
  Vector4A16::SetEpsilon(0.0001f);

  TEST_EQUAL(mtx.r1, Vector4A16(0.75f, -0.64952f, -0.125f, 0));
  TEST_EQUAL(mtx.r2, Vector4A16(0.43301f, 0.625f, -0.64952f, 0));
  TEST_EQUAL(mtx.r3, Vector4A16(0.5f, 0.43301f, 0.75f, 0));
  TEST_EQUAL(mtx.r4, Vector4A16(0, 0, 0, 1));

  return 0;
}

int test_matrix44_02() {
  esMatrix44 mtx({-0.76041f, 0.32728f, -0.56095f, 0},
                 {0.15943f, 0.93138f, 0.32728f, 0},
                 {0.6957f, 0.15943f, -0.76041f, 0}, {0, 0, 0, 1});
  Vector4A16::SetEpsilon(0.0001f);

  Vector4A16 qt = mtx.ToQuat();

  TEST_EQUAL(qt, Vector4A16(-0.1309f, -0.929f, -0.1309f, 0.3381f));

  mtx = esMatrix44({0.57631f, 0.14357f, -0.80452f, 0},
                   {-0.45027f, -0.76576f, -0.45920f, 0},
                   {-0.682f, 0.62689f, -0.37668f, 0}, {0, 0, 0, 1});

  qt = mtx.ToQuat();
  TEST_EQUAL(qt, Vector4A16(0.8244f, -0.093f, -0.4507f, 0.3293f));

  mtx = esMatrix44({0.55507f, -0.23455f, 0.80107f, 0},
                   {0.81644f, 0.35101f, -0.45849f, 0},
                   {-0.17365f, 0.90652f, 0.3848f, 0}, {0, 0, 0, 1});

  qt = mtx.ToQuat();
  TEST_EQUAL(qt, Vector4A16(0.4509f, 0.3219f, 0.3471f, 0.7567f));

  mtx = esMatrix44({-0.42858f, -0.05421f, 0.90187f, 0},
                   {0.74233f, -0.59015f, 0.31729f, 0},
                   {0.51504f, 0.80547f, 0.29317f, 0}, {0, 0, 0, 1});

  qt = mtx.ToQuat();
  TEST_EQUAL(qt, Vector4A16(0.4659f, 0.3692f, 0.7602f, 0.2619f));

  return 0;
}

int test_matrix44_03() {
  esMatrix44 mtx(Vector4A16(-0.5797f, 0.0955f, -0.53f, 0.6115f));

  auto rotated = mtx.RotatePoint({1.f, 1.f, 1.f, 1.f});
  TEST_EQUAL(rotated, Vector4A16(0.1587f, -0.5066f, 1.6486f, 1.f));

  mtx = esMatrix44({0.353041, 0.418703, 0.83669, 0.f},
                   {-0.933951, 0.21091, 0.288535, 0.f},
                   {-0.0556561, -0.883292, 0.465508, 0.f},
                   {-19.9345, 20.1798, -7.52409, 1.f});

  esMatrix44 mtxi(-mtx);

  TEST_EQUAL(mtxi.r1, Vector4A16(0.3530f, -0.9339f, -0.0556f, 0));
  TEST_EQUAL(mtxi.r2, Vector4A16(0.4187f, 0.2109f, -0.8832f, 0));
  TEST_EQUAL(mtxi.r3, Vector4A16(0.8366f, 0.2885f, 0.4655f, 0));
  TEST_EQUAL(mtxi.r4, Vector4A16(4.8836f, -20.7030f, 20.2177f, 1));

  esMatrix44 mtx2(
      {-0.703221, -0.174721, 0.689168, 0}, {0.539334f, -0.762686, 0.356973, 0},
      {0.463248, 0.622723, 0.63057, 0}, {20.8946, 0.80399, 18.8088, 1});

  mtxi = esMatrix44(-mtx2);

  TEST_EQUAL(mtxi.r1, Vector4A16(-0.7032f, 0.5393f, 0.4632f, 0));
  TEST_EQUAL(mtxi.r2, Vector4A16(-0.1747f, -0.7626f, 0.6227f, 0));
  TEST_EQUAL(mtxi.r3, Vector4A16(0.6891f, 0.3569f, 0.6305f, 0));
  TEST_EQUAL(mtxi.r4, Vector4A16(1.8715f, -17.3702f, -22.0403f, 1));

  mtxi = mtx * mtx2;

  TEST_EQUAL(mtxi.r1, Vector4A16(0.3651f, 0.14f, 0.9203f, 0));
  TEST_EQUAL(mtxi.r2, Vector4A16(0.9041f, 0.182f, -0.3864f, 0));
  TEST_EQUAL(mtxi.r3, Vector4A16(-0.2216f, 0.9732f, -0.0601f, 0));
  TEST_EQUAL(mtxi.r4, Vector4A16(42.3110f, -15.7893f, 7.5297f, 1));

  return 0;
}