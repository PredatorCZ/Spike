#include "stat.hpp"

#ifdef USEWIN
#include "internal/stat_win.inl"
#else
#include "internal/stat_ix.inl"
#endif
