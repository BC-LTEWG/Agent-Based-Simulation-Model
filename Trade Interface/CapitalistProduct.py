class CapitalistProduct:
    def __init__(self, name: str, price: float, output: float, eq_price: float, eq_output: float):
        self.name = name
        self.price = price
        self.output = output            
        self._eq_price = eq_price
        self._eq_output = eq_output

    # Equilibrium is static (read-only)
    @property
    def eq_price(self):
        return self._eq_price

    @property
    def eq_output(self):
        return self._eq_output

    def __repr__(self):
        return (
            f"Product(name={self.name!r}, price={self.price}, output={self.output}, "
            f"eq_price={self.eq_price}, eq_output={self.eq_output})"
        )
