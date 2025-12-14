#include "data_loader.h"
#include "values.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Static member definitions
std::unordered_map<std::string, std::unordered_map<std::string, double>> DataLoader::data;
std::vector<std::string> DataLoader::country_in_order;

void DataLoader::loadData(const std::string &filename)
{
    data.clear();
    country_in_order.clear();

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Read header row

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string country, gdp_nominal, gdp_ppp, mean_income, work_hours;

        std::getline(ss, country, ',');
        std::getline(ss, gdp_nominal, ',');
        std::getline(ss, gdp_ppp, ',');
        std::getline(ss, mean_income, ',');
        std::getline(ss, work_hours, ',');

        // Save fields, converting to double or INVALID_VALUE if missing
        auto &entry = data[country];
        entry["GDP_Nominal_Per_Capita"] = (gdp_nominal == "Missing") ? INVALID_VALUE : std::stod(gdp_nominal);
        entry["GDP_PPP_Per_Capita"] = (gdp_ppp == "Missing") ? INVALID_VALUE : std::stod(gdp_ppp);
        entry["Avg_Annual_Wage_USD_PPP_2023"] = (mean_income == "Missing") ? INVALID_VALUE : std::stod(mean_income);
        entry["Average_Hours_Worked_2023"] = (work_hours == "Missing") ? INVALID_VALUE : std::stod(work_hours);

        country_in_order.push_back(country);
    }

    file.close();
}

double DataLoader::getValue(const std::string &country, const std::string &key)
{
    if (data.find(country) == data.end())
        return INVALID_VALUE;
    if (data[country].find(key) == data[country].end())
        return INVALID_VALUE;
    return data[country][key];
}

const std::unordered_map<std::string, std::unordered_map<std::string, double>> &DataLoader::getAllData()
{
    return data;
}

const std::vector<std::string> &DataLoader::getCountryOrder()
{
    return country_in_order;
}
