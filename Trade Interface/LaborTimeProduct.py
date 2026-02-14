class LaborTimeProduct:
    def __init__(self, name: str, labor_time: float, order_size: float):
        self.name = name
        self._labor_time = labor_time
        self._order_size = order_size
    
    # Labor-time values and order sizes are static (read-only)
    @property
    def labor_time(self):
        return self._labor_time
    
    @property
    def order_size(self):
        return self._order_size
    
    def __repr__(self):
        return (
            f"Product(name={self.name!r}, "
            f"labor_time={self.labor_time}, "
            f"order_size={self.order_size})"
        )

