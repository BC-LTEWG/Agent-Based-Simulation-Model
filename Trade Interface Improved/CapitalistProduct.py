class CapitalistProduct:
    def __init__(
        self,
        name: str,
        price: float,
        supply: float,
        current_total_demand: float,
        eq_price: float,
        eq_supply: float,
        eq_total_demand: float
    ):
        self.name = name
        self.price = price
        self.supply = supply
        self.current_total_demand = current_total_demand
        
        self.physical_net_excess = 0
        self.value_of_goods_for_trade = 0

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
    
    # Net excess supply autocomputed every time it is called    
    @property
    def net_excess_supply(self):
        return self.supply - self.current_total_demand

    def __repr__(self):
        return (
            f"CapitalistProduct("
            f"name={self.name!r}, "
            f"price={self.price:.4f}, "
            f"supply={self.supply:.4f}, "
            f"current_total_demand={self.current_total_demand:.4f}, "
            f"net_excess_supply={self.net_excess_supply:.4f}, "
            f"physical_net_excess={self.physical_net_excess:.4f}, "
            f"value_of_goods_for_trade={self.value_of_goods_for_trade:.4f}, "
            f"eq_price={self.eq_price:.4f}, "
            f"eq_supply={self.eq_supply:.4f}, "
            f"eq_total_demand={self.eq_total_demand:.4f}"
            f")"
        )