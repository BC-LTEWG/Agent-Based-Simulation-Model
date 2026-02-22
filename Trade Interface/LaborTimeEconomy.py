class LaborTimeEconomy:
    def __init__(
        self,
        name: str,
        total_currency: float,
    ):
        self.name = name
        self.total_currency = total_currency

    def __repr__(self):
        return (
            f"LaborTimeEconomy("
            f"name={self.name!r}, "
            f"total_currency={self.total_currency:.4f}"
            f")"
        )