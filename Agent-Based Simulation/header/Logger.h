#pragma once

#include <cstdarg>
#include <fstream>
#include <map>
#include <tuple>
#include <variant>
#include <unordered_map>

using TupleNone = std::tuple<>;
using TupleInt = std::tuple<int>;
using TupleDouble = std::tuple<double>;
using TupleIntDoubleInt = std::tuple<int, double, int>;
using TupleString = std::tuple<std::string>;
using TupleStringStringInt = std::tuple<std::string, std::string, int>;
using Tuple = std::variant<TupleNone,
      TupleInt,
      TupleDouble,
      TupleString,
      TupleIntDoubleInt
      >;

struct Product;

class Logger {
    public:
        enum Client {
            FIRM,
            DISTRIBUTOR,
            PERSON,
            PRODUCER,
            PRODUCT,
            SOCIETY,
            ERROR
        };
        static Logger * get_instance();
        void log(
                const Client client,
                const std::string label,
                const unsigned int id
                );
        void log(
                const Client client,
                const std::string label,
                const unsigned int id,
                const int value
                );
        void log(
                const Client client,
                const std::string label,
                const unsigned int id,
                const double measure
                );
        void log(
                const Client client,
                const std::string label,
                const unsigned int id,
                const std::string value
                );
        void log(
                const Client client,
                const std::string label,
                const unsigned int id,
                const std::string name,
                const int quantity
                );
        void log(
                const Client client,
                const std::string label,
                const unsigned int id,
                const std::string name,
                const double measure
                );
        void log(
                const Client client,
                const std::string label,
                const unsigned int id,
                const std::pair<int, int> coords,
                const double value
                );
    private:
        void log_impl(
                const Client client,
                const std::string label,
                const unsigned int id,
                const Tuple& values
                );
        template <typename T>
        void log_impl(
                const Client client,
                const std::string label,
                const unsigned int id,
                const std::string& key,
                const T value
                );
        static void json(
                const int time_step,
                const Client client,
                std::string label,
                unsigned int id,
                const Tuple& values
                );
        template <typename T>
        static void json(
                const int time_step,
                const Client client,
                std::string label,
                unsigned int id,
                const std::string key,
                const T value
                );
        template<typename TupleT>
            static void trace_tuple(const TupleT& values);
        static const char * clients[];
};


