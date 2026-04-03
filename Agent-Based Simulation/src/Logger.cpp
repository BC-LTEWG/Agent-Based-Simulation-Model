#include <iostream>
#include <stdexcept>

#include "Constants.h"
#include "Firm.h"
#include "Logger.h"
#include "Person.h"
#include "Product.h"
#include "Sim.h"
#include "sqlite3.h"

Logger * Logger::get_instance() {
    static Logger * instance = new Logger;
    return instance;
}


void Logger::log(
        const Client client,
        const std::string label,
        const unsigned int id
        ) {
    TupleNone tuple;
    log_impl(client, label, id, tuple);
}

void Logger::log(const Client client,
        const std::string label,
        const unsigned int id,
        const int value
        ) {
    TupleInt tuple = std::make_tuple(value);
    log_impl(client, label, id, tuple);
}

void Logger::log(const Client client,
        const std::string label,
        const unsigned int id,
        const double measure
        ) {
    TupleDouble tuple = std::make_tuple(measure);
    log_impl(client, label, id, tuple);
}

void Logger::log(
        const Client client,
        const std::string label,
        const unsigned int id,
        const std::string name,
        const int quantity
        ) {
    log_impl<int>(client, label, id, name, quantity);
}

void Logger::log(
        const Client client,
        const std::string label,
        const unsigned int id,
        const std::string name,
        const double measure
        ) {
    log_impl<double>(client, label, id, name, measure);
}

const char * Logger::clients[] = {"Firm", "Distributor", "Person", "Producer", "Product", "Society"};

void Logger::log_impl(
        const Client client,
        const std::string label,
        const unsigned int id,
        const Tuple& values
        ) {
    if (!Sim::does_json()) {
        return;
    }
    int time_step = Sim::get_current_time_step();
    Logger::json(time_step, client, label, id, values);
}

template <typename T>
void Logger::log_impl(
        const Client client,
        const std::string label,
        const unsigned int id,
        const std::string& key,
        const T value
        ) {
    if (!Sim::does_json()) {
        return;
    }
    int time_step = Sim::get_current_time_step();
    Logger::json<T>(time_step, client, label, id, key, value);
}

void Logger::json(
        const int time_step,
        const Client client,
        std::string label,
        unsigned int id,
        const Tuple& values
        ) {
    if (client >= ERROR) {
        throw std::invalid_argument("Logging client does not exist");
    }
    std::cout << "{\"t\":" << time_step << "," <<
        "\"client\":\"" << clients[client] << "\"," <<
        "\"id\":" << id << "," <<
        "\"label\":\"" << label << "\"," <<
        "\"values\":[";
    auto visitor = [](auto&& arg) {
        Logger::trace_tuple(arg);
    };
    std::visit(visitor, values);
    std::cout << "]}" << std::endl;
}

template <typename T>
void Logger::json(
        const int time_step,
        const Client client,
        std::string label,
        unsigned int id,
        const std::string key,
        const T value
        ) {
    if (client >= ERROR) {
        throw std::invalid_argument("Logging client does not exist");
    }
    std::cout << "{\"t\":" << time_step << "," <<
        "\"client\":\"" << clients[client] << "\"," <<
        "\"id\":" << id << "," <<
        "\"label\":\"" << label << "\"," <<
        "\"" << key << "\":" << value << "}" << std::endl;
}

template<typename TupleT>
void Logger::trace_tuple(const TupleT& values) {
    static int count = 0;
    std::apply([](auto&& ... arg) {
            ((std::cout << (count++ ? "," : "") << arg), ...); count++;
            }, values);
    count = 0;
}

