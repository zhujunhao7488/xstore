#pragma once

#include <vector>

#include "../../deps/r2/src/common.hh"

namespace xstore {

namespace xcache {

using namespace r2;

template <typename EntryType> struct TT {
  std::vector<EntryType> entries;

  EntryType &operator[](int i) { return entries[i]; }

  auto add(const EntryType &e) { entries.push_back(e); }

  auto size() -> usize { return entries.size(); }

  auto clear() { entries.clear(); }
};
} // namespace xcache
} // namespace xstore