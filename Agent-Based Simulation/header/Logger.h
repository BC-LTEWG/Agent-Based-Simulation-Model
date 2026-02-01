#include <cstdarg>
#include <tuple>
#include <variant>
#include <unordered_map>

using TupleNone = std::tuple<>;
using TupleInt = std::tuple<int>;
using TupleDouble = std::tuple<double>;
using TupleStringInt = std::tuple<std::string, int>;
using TupleStringDouble = std::tuple<std::string, double>;
using TupleIntDoubleInt = std::tuple<int, double, int>;
using Tuple = std::variant<TupleNone,
      TupleDouble,
      TupleStringInt,
      TupleStringDouble,
      TupleIntDoubleInt>;

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
        void log(const Client client, const std::string label);
        void log(
                const Client client,
                const std::string label,
                const int value
                );
        void log(
                const Client client,
                const std::string label,
                const double measure
                );
        void log(
                const Client client,
                const std::string label,
                const std::string name,
                const int quantity
                );
        void log(
                const Client client,
                const std::string label,
                const std::string name,
                const double measure
                );
    private:
        Logger();
        static void trace(
                const int time_step,
                const Client client,
                std::string label,
                const Tuple& values
                );
        template<typename TupleT>
            static void trace_tuple(const TupleT& values);
        static const char * clients[];
        std::unordered_map<Client,
            std::unordered_map<std::string,
            std::unordered_map<int,
            Tuple>>> data;
};
