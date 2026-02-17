#include <cstdarg>
#include <fstream>
#include <map>
#include <tuple>
#include <variant>
#include <unordered_map>

#include "sqlite3.h"

using TupleNone = std::tuple<>;
using TupleInt = std::tuple<int>;
using TupleDouble = std::tuple<double>;
using TupleStringInt = std::tuple<std::string, int>;
using TupleStringDouble = std::tuple<std::string, double>;
using TupleIntDoubleInt = std::tuple<int, double, int>;
using TupleStringStringInt = std::tuple<std::string, std::string, int>;
using Tuple = std::variant<TupleNone,
      TupleDouble,
      TupleStringInt,
      TupleStringDouble,
      TupleIntDoubleInt
      >;

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
        void write_data();
    private:
        Logger();
        ~Logger();
        void log_impl(
                const Client client,
                const std::string label,
                const unsigned int id,
                const Tuple& values
                );
        static void trace(
                const int time_step,
                const Client client,
                std::string label,
                unsigned int id,
                const Tuple& values
                );
        static void log_to_db(
                const int time_step,
                const Client client,
                std::string label,
                unsigned int id,
                const Tuple& values
                );
        template<typename TupleT>
            static void trace_tuple(const TupleT& values);
        template<typename TupleT>
            static void write_tuple(std::ofstream& file, const TupleT& values);
        static const char * clients[];
        sqlite3 * db;
        // client -> label -> id -> time step -> data
        std::unordered_map<Client,
            std::unordered_map<std::string,
            std::map<unsigned int,
            std::map<int, Tuple>>>> data;
};
