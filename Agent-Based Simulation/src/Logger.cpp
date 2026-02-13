#include <iostream>
#include <stdexcept>

#include "Constants.h"
#include "Firm.h"
#include "Logger.h"
#include "Person.h"
#include "Product.h"
#include "Sim.h"
#include "sqlite3.h"

Logger::Logger() {
    if (Sim::is_using_db()) {
        int return_code = sqlite3_open(LOG_FILE, &db);
        if (return_code) {
            throw std::runtime_error("Dztabase connection failure");
        }
        std::cout << "Opened DB" << std::endl;
    } else {
        db = NULL;
    }
}

Logger::~Logger() {
    if (db) {
        sqlite3_close(db);
    }
}

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
    TupleStringInt tuple = std::make_tuple(name, quantity);
    log_impl(client, label, id, tuple);
}

void Logger::log(
        const Client client,
        const std::string label,
        const unsigned int id,
        const std::string name,
        const double measure
        ) {
    TupleStringDouble tuple = std::make_tuple(name, measure);
    log_impl(client, label, id, tuple);
}

const char * Logger::clients[] = {"Firm", "Distributor", "Person", "Producer", "Product", "Society"};

const char * Logger::logging_dir = "data";

void Logger::log_impl(
        const Client client,
        const std::string label,
        const unsigned int id,
        const Tuple& values
        ) {
    int time_step = Sim::get_current_time_step();
    if (Sim::is_trace_logging()) {
        Logger::trace(time_step, client, label, id, values);
    }
    if (Sim::is_using_db()) {
        log_to_db(time_step, client, label, id, values);
    }
    data[client][label][id][time_step] = values;
}

void Logger::trace(
        const int time_step,
        const Client client,
        std::string label,
        unsigned int id,
        const Tuple& values
        ) {
    if (client >= ERROR) {
        throw std::invalid_argument("Logging client does not exist");
    }
    std::cout << "[" << time_step << "] ";
    std::cout << clients[client] << "_" << id << ": ";
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

template<typename TupleT>
void Logger::write_tuple(std::ofstream& out_file, const TupleT& values) {
    std::apply([&out_file](auto&& ... arg) {
            ((out_file << "," << arg), ...);
            }, values);
}

void Logger::write_data() {
    std::string s("price");
    for (auto& client_map : data) {
        std::string client_prefix = clients[client_map.first];
        for (auto& label_map : client_map.second) {
            std::string file_name = std::string(LOGGING_DIR) + "/" +
                client_prefix + "_" + label_map.first + ".csv";
            std::ofstream out_file(file_name);
            auto visitor = [&out_file](auto&& arg) {
                Logger::write_tuple(out_file, arg);
            };
            for (auto& id_map : label_map.second) {
                for (auto& time_step_map : id_map.second) {
                    out_file << id_map.first;
                    out_file << "," << time_step_map.first;
                    std::visit(visitor, time_step_map.second);
                    out_file << std::endl;
                }
            }
            out_file.close();
        }
    }
}

void Logger::log_to_db(
                const int time_step,
                const Client client,
                std::string label,
                unsigned int id,
                const Tuple& values
                ) {
}
