#include "data_loader.h"
#include "values.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::unordered_map<std::string, std::unordered_map<std::string, double> > DataLoader::data;

void DataLoader::loadData(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string country, gdp_nominal, gdp_ppp;
        std::getline(ss, country, ',');
        std::getline(ss, gdp_nominal, ',');
        std::getline(ss, gdp_ppp, ',');

        data[country]["GDP_NOMINAL"] = (gdp_nominal == "Missing") ? INVALID_VALUE : std::stod(gdp_nominal);
        data[country]["GDP_PPP"] = (gdp_ppp == "Missing") ? INVALID_VALUE : std::stod(gdp_ppp);
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

const std::unordered_map<std::string, std::unordered_map<std::string, double> > &DataLoader::getAllData()
{
    return data;
}
