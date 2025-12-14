#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class DataLoader
{
public:
    // Loads data.csv into memory
    static void loadData(const std::string &filename);

    // Retrieves a value (returns INVALID_VALUE if not found)
    static double getValue(const std::string &country, const std::string &key);

    // Retrieves all data (read-only reference)
    static const std::unordered_map<std::string, std::unordered_map<std::string, double>> &getAllData();

    // Retrieves all countries in the order they appeared in the CSV
    static const std::vector<std::string> &getCountryOrder();

private:
    static std::unordered_map<std::string, std::unordered_map<std::string, double>> data;
    static std::vector<std::string> country_in_order;
};
