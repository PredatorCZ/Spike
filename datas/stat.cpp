#include "stat.hpp"

#ifdef _MSC_VER
#include "internal/stat_win.inl"
#else
#include "internal/stat_ix.inl"
#endif
