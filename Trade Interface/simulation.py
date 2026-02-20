import csv
import random 
import os

# Product classes for each economy
from CapitalistEconomy import CapitalistEconomy
from CapitalistProduct import CapitalistProduct
from LaborTimeProduct import LaborTimeProduct 

# Use labor time value + order size for LTE products 
# use MELT for the US, perhaps the mechanics from the summer project 
# Prices and supply needs to be recomputed as well 
# Order size does not vary 

CE_NUMBER_OF_PRODUCT_TYPES = 26
LTE_NUMBER_OF_PRODUCT_TYPES = 26
NUMBER_OF_TIME_STEPS = 40
CE_PRICE_SCALE_FACTOR = 938.07
# CE quantities are proportional (structurally real) units,
# not physical units. The input-output (Leontief) system
# determines production ratios but not absolute magnitude.
#
# Because the model does not fix the economy’s physical size,
# I choose an arbitrary scaling factor to anchor abstract
# production units to a physical magnitude.
#
# Scaling changes size, not structure.
CE_PHYSICAL_SCALE = 1000

# Note: the prices are in ratios, not in any currency. 
def build_ce_map(ce_csv_path):
    """
    Build the CE commodity → CapitalistProduct map using the equilibrium row
    from trajectories_with_equilibrium.csv.
    Includes equilibrium price, supply, and total internal demand.
    """
    # Ensure CSV file exists
    if not os.path.exists(ce_csv_path):
        raise FileNotFoundError(f"CSV not found at: {ce_csv_path}")

    eq_row = None  # Will store the equilibrium row

    # Read CSV as a dictionary per row
    with open(ce_csv_path, "r", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            # Identify the unique equilibrium row
            if row.get("is_equilibrium_step") == "1":
                eq_row = row
                break

    # Equilibrium must exist
    if eq_row is None:
        raise ValueError("No equilibrium row found (is_equilibrium_step == 1).")

    ce_map = {}

    # Build CE product objects for A–Z
    for i in range(CE_NUMBER_OF_PRODUCT_TYPES):
        name = chr(ord("A") + i)

        price_key = f"equilibrium_price_{name}"
        supply_key = f"equilibrium_supply_{name}"
        demand_key = f"equilibrium_total_demand_{name}"

        price_val = eq_row.get(price_key)
        supply_val = eq_row.get(supply_key)
        demand_val = eq_row.get(demand_key)

        if price_val in (None, ""):
            raise ValueError(f"Missing {price_key} in equilibrium row.")
        if supply_val in (None, ""):
            raise ValueError(f"Missing {supply_key} in equilibrium row.")
        if demand_val in (None, ""):
            raise ValueError(f"Missing {demand_key} in equilibrium row.")

        eq_price = float(price_val) * CE_PRICE_SCALE_FACTOR
        eq_supply = float(supply_val)
        eq_total_demand = float(demand_val)

        # Current values initialized at equilibrium
        current_price = eq_price
        current_supply = eq_supply

        ce_map[name] = CapitalistProduct(
            name=name,
            price=current_price,
            supply=current_supply,
            current_total_demand=eq_total_demand,
            eq_price=eq_price,
            eq_supply=eq_supply,
            eq_total_demand=eq_total_demand
        )

    return ce_map


def build_lte_map(lte_csv_path):
    """
    Build LTE commodity → LaborTimeProduct map using
    the first 26 entries from Society_price.csv and Society_order_size.csv.
    """

    lte_map = {}

    price_path = os.path.join(lte_csv_path, "Society_price.csv")
    order_size_path = os.path.join(lte_csv_path, "Society_order_size.csv")


    # Read labor times (price)
    labor_times = []
    with open(price_path, newline="") as f:
        reader = csv.reader(f)
        next(reader)  # skip header if present
        for row in reader:
            labor_times.append(float(row[2]))  # third column

    # Read order sizes
    order_sizes = []
    with open(order_size_path, newline="") as f:
        reader = csv.reader(f)
        next(reader)  # skip header if present
        for row in reader:
            order_sizes.append(float(row[2]))  # third column

    # Safety check
    if len(labor_times) < LTE_NUMBER_OF_PRODUCT_TYPES:
        raise ValueError("Not enough labor-time entries in Society_price.csv")

    if len(order_sizes) < LTE_NUMBER_OF_PRODUCT_TYPES:
        raise ValueError("Not enough order-size entries in Society_order_size.csv")

    # Build products A–Z
    for i in range(LTE_NUMBER_OF_PRODUCT_TYPES):
        name = chr(ord("A") + i)
        labor_time = labor_times[i]
        order_size = order_sizes[i]

        lte_map[name] = LaborTimeProduct(
            name=name,
            labor_time=labor_time,
            order_size=order_size,
        )

    return lte_map


def print_maps(ce_map, lte_map):
    """
    Print both CE and LTE maps in a readable format.
    """
    print("\n--- CE map (commodity -> CapitalistProduct) ---")
    for name in sorted(ce_map.keys()):
        print(name, "->", ce_map[name])

    print("\n--- LTE map (commodity -> LaborTimeProduct) ---")
    for name in sorted(lte_map.keys()):
        print(name, "->", lte_map[name])

def load_ce_post_equilibrium_series(csv_path, window_size=40):
    """
    Load CE price and supply data starting at equilibrium
    and continuing for `window_size` time steps after equilibrium.

    Indexing convention:
        t = 0  -> equilibrium
        t = 1  -> equilibrium + 1
        ...
        t = window_size → equilibrium + window_size
    """

    with open(csv_path, "r", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)

    # Locate equilibrium index
    eq_index = None
    for i, row in enumerate(rows):
        if row.get("is_equilibrium_step") == "1":
            eq_index = i
            break

    if eq_index is None:
        raise ValueError("Equilibrium row not found in CSV.")

    # Slice desired window
    end_index = eq_index + window_size + 1
    window_rows = rows[eq_index:end_index]

    ce_time_series = []

    for row in window_rows:
        supply = {}
        price = {}
        demand = {}

        for i in range(CE_NUMBER_OF_PRODUCT_TYPES):
            name = chr(ord("A") + i)
            supply[name] = float(row[f"supply_{name}"])
            price[name] = float(row[f"price_{name}"]) * CE_PRICE_SCALE_FACTOR
            demand[name] = float(row[f"current_total_demand_{name}"])

        ce_time_series.append({
            "supply": supply,
            "price": price, 
            "demand": demand
        })

    return ce_time_series

# For debugging
def print_equilibrium_total_demand(ce_map):
    print("\n--- Equilibrium Total Demand (REAL UNITS) ---")
    for name in sorted(ce_map.keys()):
        product = ce_map[name]
        print(f"{name}: {product.eq_total_demand:.4f} (real units)")

# Lists for import/export         
CE_Import = []
LTE_Import = []
CE_Export = []
LTE_Export = []

def update_ce_map(ce_map, ce_time_series, time_step):
    """
    Update CE product state at a given time step.

    For each CE product i:
        - Update current supply and price from simulation data.
        - Compute net_excess_supply_i(t):

            net_excess_supply_i(t)
                = current_supply_i(t) - current_total_demand_i

    Interpretation:
        If net_excess_supply_i(t) > 0:
            The CE has excess stock available for export.
        If net_excess_supply_i(t) < 0:
            The CE has a deficit and requires imports.
        If net_excess_supply_i(t) = 0:
            The CE is exactly reproducing its internal requirements.
    """

    for name, product in ce_map.items():
        # Update current supply from the simulation time step
        product.supply = ce_time_series[time_step]["supply"][name]
        
        # Update current price from the time series 
        product.price = ce_time_series[time_step]["price"][name]
        
        # Update current total demand 
        product.current_total_demand = ce_time_series[time_step]["demand"][name]
        
        product.physical_net_excess = (
            product.net_excess_supply * CE_PHYSICAL_SCALE
        )
        
def compute_melt(ce):
    # update the two fields here 
    ce.new_gdp_ppp_per_capita()
    ce.new_average_work_hours()
    # Compute MELT (money per labor-hour) using working population hours (incl. self-employed)
    ce.melt = ce.current_gdp_ppp_per_capita / ce.current_avg_hours_worked
    
def generate_lte_shortages(lte_map):
    """
    Randomly select 0-5 LTE products to be missing this time step.
    Add them to LTE_Import.
    """

    global LTE_Import

    # List out all LTE product names
    product_names = list(lte_map.keys())

    # Random number of missing goods (0 to 5)
    num_missing = random.randint(0, 5)

    # Randomly select without replacement
    missing_products = random.sample(product_names, k=num_missing)

    for name in missing_products:
        if name not in LTE_Import:
            LTE_Import.append(name)
    
    return missing_products 
    
# Note: This trade function only handles trade on a perticular time step. 
# If an item on the trading list can not be traded, then the shortage/surplus persists 
# On the next time step, the trade function will be called again and attempt to resolve the 
# previous shortage/surplus issue while the economy adjusts itself to try to satisfy its own needs 
def trade(ce, lte, ce_map, ce_time_series, lte_map, time_step):
    """
    Called every simulation time step.

    Arguments:
        ce_map  : CE commodity -> CapitalistProduct map
        lte_map : LTE commodity -> LaborTimeProduct map
        time_step : current simulation time step

    Updates:
        CE_Import, LTE_Import, CE_Export, LTE_Export
    """
    
    global CE_Import, LTE_Import, CE_Export, LTE_Export

    # Reset lists every timestep
    CE_Import.clear()
    LTE_Import.clear()
    CE_Export.clear()
    LTE_Export.clear()
    
    # Generate shortages for the LTE 
    generate_lte_shortages(lte_map)
    
    # Update CE supply and compute net excessive supply for this time step.
    # net_excess_supply_i(t) = current_supply_i(t) - current_total_demand_i(t)
    #
    # current_total_demand_i(t) represents the reproduction requirement
    # for commodity i at time t.
    #
    # D_i(t) = (A · q(t))_i + b_i(t) + c_i(t)
    #
    # where:
    #   - (A · q(t))_i is the intermediate input demand needed
    #     to reproduce current production,
    #   - b_i(t) is worker consumption demand,
    #   - c_i(t) is capitalist consumption demand.
    #
    # Therefore, current_total_demand_i(t) is the quantity of commodity i
    # required to reproduce the current scale of production and consumption.
    #
    # If net_excess_supply_i(t) > 0:
    #     The CE can export without disrupting reproduction.
    # If net_excess_supply_i(t) < 0:
    #     The CE cannot fully reproduce at time t and requires imports.
    # If net_excess_supply_i(t) = 0:
    #     Supply exactly meets reproduction needs.
    update_ce_map(ce_map, ce_time_series, time_step) 
    
    # Convert keys to sets for fast comparison
    ce_products = set(ce_map.keys())
    lte_products = set(lte_map.keys())
    
    # CE has but LTE doesn't - LTE must import
    for name in ce_products - lte_products:
        LTE_Import.append(name)

    # LTE has but CE doesn't - CE must import
    for name in lte_products - ce_products:
        CE_Import.append(name)
        
    for name in ce_map:                    
        product = ce_map[name]          

        # CE shortage detection 
        if product.net_excess_supply < 0:
            if name not in CE_Import:
                CE_Import.append(name)

        # CE surplus detection 
        elif product.net_excess_supply > 0:
            if name not in CE_Export:
                CE_Export.append(name)
    
    # A new melt for every time step before trading      
    compute_melt(ce)
    
    
                
    # TODO:
    # 1. We need to give LTE MELT values to convert the prices. - already done 
    # 2. We need to convert CE prices which are in ratios to actual currency values. - already done 
    # 3. We need to give LTE arbitrary shortage for random product types. - already done 
    # 4. We need to finish converting the unit of the total demand and scale the number of products available for export. - need to base on discussion  
    # 5. We need to use the trade algorithm developed to simulate trade. 
    # 6. We need to record the change in currency between the two economies and graph it. 
    
    # Note: I don't think I'll have time for applying the effect back to the economy. That will be for March and April. 

    print(f"[t={time_step}] Trade() executed")

if __name__ == "__main__":
    ce_data_path = os.path.join(
    "..",
    "Event-Based Simulation",
    "graphs",
    "trajectories_with_equilibrium.csv"
    )

    lte_data_path = os.path.join(
        "..",
        "Agent-Based Simulation",
        "data"
    )
    
    usa = CapitalistEconomy(
        name="United States",
        total_currency=85809.900385,  # GDP per capita, PPP
        melt=0.0,  # placeholder 
        gdp_ppp_per_capita=85809.900385,
        avg_hours_worked_by_working_population=1795.906,
        avg_hours_worked_by_employees_only=1809.583,
    )
    
    # Build CE and LTE maps
    ce_map = build_ce_map(ce_data_path)
    lte_map = build_lte_map(lte_data_path)
    
    # Load CE data for access in O(1) time 
    ce_time_series = load_ce_post_equilibrium_series(ce_data_path)

    # Display results
    print_maps(ce_map, lte_map)
    print_equilibrium_total_demand(ce_map)
    
    # for i in range (NUMBER_OF_TIME_STEPS):
    #     # Place holder 
    #     trade(usa, lte, ce_map, ce_time_series, lte_map, i)
