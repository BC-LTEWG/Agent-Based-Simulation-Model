#include "capitalist_economy.h"
#include "values.h"

CapitalistEconomy::CapitalistEconomy(int id, const std::string &name,
                                     double gdp_nominal, double gdp_ppp,
                                     double median, double mean,
                                     double hours)
    : ce_id(id), economy_name(name),
      gdp_nominal_per_capita(gdp_nominal),
      gdp_ppp_per_capita(gdp_ppp),
      median_income(median),
      mean_income(mean),
      avg_work_hours_per_year(hours) {}

CapitalistEconomy CapitalistEconomy::from_gdp_nominal(int id, const std::string &name,
                                                      double gdp, double hours)
{
    return CapitalistEconomy(id, name, gdp, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, hours);
}

CapitalistEconomy CapitalistEconomy::from_gdp_ppp(int id, const std::string &name,
                                                  double ppp, double hours)
{
    return CapitalistEconomy(id, name, INVALID_VALUE, ppp, INVALID_VALUE, INVALID_VALUE, hours);
}

CapitalistEconomy CapitalistEconomy::from_median_income(int id, const std::string &name,
                                                        double median, double hours)
{
    return CapitalistEconomy(id, name, INVALID_VALUE, INVALID_VALUE, median, INVALID_VALUE, hours);
}

CapitalistEconomy CapitalistEconomy::from_mean_income(int id, const std::string &name,
                                                      double mean, double hours)
{
    return CapitalistEconomy(id, name, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, mean, hours);
}

bool CapitalistEconomy::uses_gdp_nominal() const
{
    return gdp_nominal_per_capita != INVALID_VALUE;
}

bool CapitalistEconomy::uses_gdp_ppp() const
{
    return gdp_ppp_per_capita != INVALID_VALUE;
}

bool CapitalistEconomy::uses_median() const
{
    return median_income != INVALID_VALUE;
}

bool CapitalistEconomy::uses_mean() const
{
    return mean_income != INVALID_VALUE;
}
