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

        # Raw international data (static / read-only)
        self._gdp_ppp_per_capita = gdp_ppp_per_capita
        self._avg_hours_worked_by_working_population = (
            avg_hours_worked_by_working_population
        )
        self._avg_hours_worked_by_employees_only = avg_hours_worked_by_employees_only

    # Read-only properties 
    @property
    def gdp_ppp_per_capita(self):
        return self._gdp_ppp_per_capita

    @property
    def avg_hours_worked_by_working_population(self):
        return self._avg_hours_worked_by_working_population

    @property
    def avg_hours_worked_by_employees_only(self):
        return self._avg_hours_worked_by_employees_only

    def __repr__(self):
        return (
            f"CapitalistEconomy("
            f"name={self.name!r}, "
            f"total_currency={self.total_currency}, "
            f"melt={self.melt}, "
            f"gdp_ppp_per_capita={self.gdp_ppp_per_capita}, "
            f"avg_hours_worked_by_working_population="
            f"{self.avg_hours_worked_by_working_population}, "
            f"avg_hours_worked_by_employee="
            f"{self.avg_hours_worked_by_employees_only}"
            f")"
        )
