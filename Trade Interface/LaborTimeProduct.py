class LaborTimeProduct:
    def __init__(self, name: str, labor_time: float):
        self.name = name
        self.labor_time = labor_time

    def __repr__(self):
        return f"Product(name={self.name!r}, price={self.labor_time})"
