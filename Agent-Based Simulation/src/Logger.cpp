#include <iostream>
#include <stdexcept>

#include "Firm.h"
#include "Logger.h"
#include "Person.h"
#include "Product.h"
#include "Sim.h"

Logger::Logger() {}

Logger * Logger::get_instance() {
    static Logger * instance = new Logger;
    return instance;
}


void Logger::log(const Client client, const std::string label) {
    int time_step = Sim::get_current_time_step();
    TupleNone tuple;
    if (Sim::is_trace_logging()) {
        Logger::trace(time_step, client, label, tuple);
    }
    data[client][label][time_step] = tuple;
}

void Logger::log(const Client client, const std::string label, const int value) {
    int time_step = Sim::get_current_time_step();
    TupleInt tuple = std::make_tuple(value);
    if (Sim::is_trace_logging()) {
        Logger::trace(time_step, client, label, tuple);
    }
    data[client][label][time_step] = tuple;

}

void Logger::log(const Client client, const std::string label, const double measure) {
    int time_step = Sim::get_current_time_step();
    TupleDouble tuple = std::make_tuple(measure);
    if (Sim::is_trace_logging()) {
        Logger::trace(time_step, client, label, tuple);
    }
    data[client][label][time_step] = tuple;

}

void Logger::log(
        const Client client,
        const std::string label,
        const std::string name,
        const int quantity
        ) {
    int time_step = Sim::get_current_time_step();
    TupleStringInt tuple = std::make_tuple(name, quantity);
    if (Sim::is_trace_logging()) {
        Logger::trace(time_step, client, label, tuple);
    }
    data[client][label][Sim::get_current_time_step()] = tuple;
}

void Logger::log(
        const Client client,
        const std::string label,
        const std::string name,
        const double measure
        ) {
    int time_step = Sim::get_current_time_step();
    TupleStringDouble tuple = std::make_tuple(name, measure);
    if (Sim::is_trace_logging()) {
        Logger::trace(time_step, client, label, tuple);
    }
    data[client][label][Sim::get_current_time_step()] = std::make_tuple(name, measure);
}

const char * Logger::clients[] = {"Firm", "Distributor", "Person", "Producer", "Product", "Society"};

void Logger::trace(
        const int time_step,
        const Client client,
        std::string label,
        const Tuple& values
        ) {
    if (client >= ERROR) {
        throw std::invalid_argument("Logging client does not exist");
    }
    std::cout << "[" << time_step << "] ";
    std::cout << clients[client] << ": ";
    std::cout << label << " - ";
    auto visitor = [](auto&& arg) {
        Logger::trace_tuple(arg);
};

    std::visit(visitor, values);
    std::cout << std::endl;
}

template<typename TupleT>
void Logger::trace_tuple(const TupleT& values) {
    std::apply([](auto&& ... arg) {
            ((std::cout << arg << " "), ...);
            }, values);
}
