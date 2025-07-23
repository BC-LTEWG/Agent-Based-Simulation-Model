#pragma once
#include <string>

class CapitalistEconomy
{
public:
    int ce_id;
    std::string economy_name;

    double gdp_nominal_per_capita;
    double gdp_ppp_per_capita;
    double median_income;
    double mean_income;

    double avg_work_hours_per_year;

    // Named static constructors
    static CapitalistEconomy from_gdp_nominal(int id, const std::string &name, double gdp, double hours);
    static CapitalistEconomy from_gdp_ppp(int id, const std::string &name, double ppp, double hours);
    static CapitalistEconomy from_median_income(int id, const std::string &name, double median, double hours);
    static CapitalistEconomy from_mean_income(int id, const std::string &name, double mean, double hours);

    // Utility methods
    bool uses_gdp_nominal() const;
    bool uses_gdp_ppp() const;
    bool uses_median() const;
    bool uses_mean() const;

private:
    CapitalistEconomy(int id, const std::string &name,
                      double gdp_nominal, double gdp_ppp,
                      double median, double mean,
                      double hours);
};
