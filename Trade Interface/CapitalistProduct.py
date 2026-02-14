class CapitalistProduct:
    def __init__(
        self,
        name: str,
        price: float,
        supply: float,
        eq_price: float,
        eq_supply: float,
        eq_total_demand: float
    ):
        self.name = name
        self.price = price
        self.supply = supply
        self.export_capacity_ratio = supply / eq_total_demand

        # Equilibrium values (static)
        self._eq_price = eq_price
        self._eq_supply = eq_supply
        self._eq_total_demand = eq_total_demand

    # Equilibrium values are static (read-only)
    @property
    def eq_price(self):
        return self._eq_price

    @property
    def eq_supply(self):
        return self._eq_supply

    @property
    def eq_total_demand(self):
        return self._eq_total_demand

    def __repr__(self):
        return (
            f"Product(name={self.name!r}, price={self.price}, supply={self.supply}, export_capacity_ratio={self.export_capacity_ratio}, "
            f"eq_price={self.eq_price}, eq_supply={self.eq_supply}, "
            f"eq_total_demand={self.eq_total_demand})"
        )
