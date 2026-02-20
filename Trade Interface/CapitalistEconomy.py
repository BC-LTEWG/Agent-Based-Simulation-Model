import random

class CapitalistEconomy:
    def __init__(
        self,
        name: str,
        total_currency: float,
        melt: float,
        gdp_ppp_per_capita: float,
        avg_hours_worked_by_working_population: float,
        avg_hours_worked_by_employees_only: float,
    ):
        self.name = name
        self.total_currency = total_currency
        self.melt = melt

        # Baseline international data (read-only reference) 
        self._gdp_ppp_per_capita = gdp_ppp_per_capita
        self._avg_hours_worked_by_working_population = (
            avg_hours_worked_by_working_population
        )
        self._avg_hours_worked_by_employees_only = (
            avg_hours_worked_by_employees_only
        )

        # Dynamic simulation variables 
        self.current_gdp_ppp_per_capita = gdp_ppp_per_capita
        self.current_avg_hours_worked = (
            avg_hours_worked_by_working_population
        )
    
    @property
    def gdp_ppp_per_capita(self):
        return self._gdp_ppp_per_capita

    @property
    def avg_hours_worked_by_working_population(self):
        return self._avg_hours_worked_by_working_population

    @property
    def avg_hours_worked_by_employees_only(self):
        return self._avg_hours_worked_by_employees_only
    
    def new_gdp_ppp_per_capita(self):
        self.current_gdp_ppp_per_capita *= random.uniform(0.9, 1.1)

    def new_average_work_hours(self):
        hours = self.current_avg_hours_worked

        if 1000 <= hours <= 2000:
            hours *= random.uniform(0.8, 1.2)
        elif 800 <= hours < 1000:
            hours *= random.uniform(1.0, 1.2)
        elif 2000 < hours <= 2400:
            hours *= random.uniform(0.8, 1.0)

        # hard bounds
        hours = max(800, min(2400, hours))
        self.current_avg_hours_worked = hours

    def __repr__(self):
        return (
            f"CapitalistEconomy("
            f"name={self.name!r}, "
            f"total_currency={self.total_currency}, "
            f"melt={self.melt:.4f}, "
            f"current_gdp_ppp_per_capita="
            f"{self.current_gdp_ppp_per_capita:.2f}, "
            f"current_avg_hours_worked="
            f"{self.current_avg_hours_worked:.2f}"
            f")"
        )
