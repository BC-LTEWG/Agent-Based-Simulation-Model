#pragma once

template <typename T> 
struct Determinism { //Fallback execution (resolves errors in Person.cpp)
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

struct Product;
struct Order;
struct Plan;
class Person;
template <> struct Determinism<Product*> {
    bool operator()(Product* a, Product* b) const;
};
template <> struct Determinism<Order*> {
    bool operator()(Order* a, Order* b) const;
};
template <> struct Determinism<Plan*> {
    bool operator()(Plan* a, Plan* b) const;
};
template <> struct Determinism<Person*> {
    bool operator()(Person* a, Person* b) const;
};

#ifdef DEBUG
#include <map>
#include <set>
template <typename T, typename U>
using MAP = std::map<T, U, Determinism<T>>;
template <typename T>
using SET = std::set<T, Determinism<T>>;
#define DEBUG_SEED(args) { \
    args.seed = 1; \
    args.fixed_seed = true; \
}

#else
#include <unordered_map>
#include <unordered_set>
template <typename T, typename U>
using MAP = std::unordered_map<T, U>;
template <typename T>
using SET = std::unordered_set<T>;
#define DEBUG_SEED(args) {}
#endif
