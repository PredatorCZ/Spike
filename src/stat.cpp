#include "spike/io/stat.hpp"

#ifdef USEWIN
#include "detail/stat_win.inl"
#else
#include "detail/stat_ix.inl"
#endif
