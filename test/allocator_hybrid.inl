#include "../datas/unit_testing.hpp"
#include "../datas/allocator_hybrid.hpp"
#include <vector>
#include <cstring>

int test_alloc_hybrid() {
  char *externalBuffer = static_cast<char *>(malloc(125));
  memset(externalBuffer, 89, 125);

  std::vector<char, es::allocator_hybrid<char>> vectr;
  vectr.resize(85);

  TEST_EQUAL(vectr.get_allocator().buffer, nullptr);

  for (auto &v : vectr)
    TEST_EQUAL(v, 0);

  es::allocator_hybrid_base::LinkStorage(vectr, externalBuffer, 125);

  TEST_EQUAL(vectr.get_allocator().buffer, externalBuffer);
  TEST_EQUAL(
      vectr.get_allocator(),
      es::allocator_hybrid<short>(reinterpret_cast<short *>(externalBuffer)));
  TEST_EQUAL(vectr.size(), 125);

  for (auto &v : vectr)
    TEST_EQUAL(v, 89);

  vectr[12] = 12;

  TEST_EQUAL(vectr[12], 12);
  TEST_EQUAL(externalBuffer[12], 12);

  es::allocator_hybrid_base::DisposeStorage(vectr);

  TEST_EQUAL(vectr.size(), 0);
  TEST_EQUAL(vectr.get_allocator().buffer, nullptr);

  vectr.push_back(75);

  TEST_EQUAL(vectr.size(), 1);
  TEST_EQUAL(vectr.get_allocator().buffer, nullptr);
  TEST_EQUAL(vectr[0], 75);
  TEST_EQUAL(externalBuffer[0], 89);

  free(externalBuffer);

  return 0;
}
