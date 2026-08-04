#pragma once

#ifdef DEBUG
#include <map>
#include <set>
template <typename T, typename U>
using MAP = std::map<T, U>;
template <typename T>
using SET = std::set<T>;
#else
#include <unordered_map>
#include <unordered_set>
template <typename T, typename U>
using MAP = MAP<T, U>;
template <typename T>
using SET = SET<T>;
#endif
