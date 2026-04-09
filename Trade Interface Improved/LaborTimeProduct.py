class LaborTimeProduct:
    def __init__(self, name: str, labor_time: float, order_size: float):
        self.name = name
        self._labor_time = labor_time
        self._order_size = order_size
        
        self.value_of_goods_for_trade = 0
    
    # Labor-time values and order sizes are static (read-only)
    @property
    def labor_time(self):
        return self._labor_time
    
    @property
    def order_size(self):
        return self._order_size
    
    def __repr__(self):
        return (
            f"LaborTimeProduct("
            f"name={self.name!r}, "
            f"labor_time={self.labor_time:.4f}, "
            f"value_of_goods_for_trade={self.value_of_goods_for_trade:.4f}, "
            f"order_size={self.order_size:.4f}"
            f")"
        )

