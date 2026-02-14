import csv
import os

# Product classes for each economy
from CapitalistProduct import CapitalistProduct
from LaborTimeProduct import LaborTimeProduct 

# Use labor time value + order size for LTE products 
# use MELT for the US, perhaps the mechanics from the summer project 
# Prices and supply needs to be recomputed as well 
# Order size does not vary 

CE_NUMBER_OF_PRODUCT_TYPES = 26
LTE_NUMBER_OF_PRODUCT_TYPES = 26
NUMBER_OF_TIME_STEPS = 40


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

        eq_price = float(price_val)
        eq_supply = float(supply_val)
        eq_total_demand = float(demand_val)

        # Current values initialized at equilibrium
        current_price = eq_price
        current_supply = eq_supply

        ce_map[name] = CapitalistProduct(
            name=name,
            price=current_price,
            supply=current_supply,
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

        for i in range(CE_NUMBER_OF_PRODUCT_TYPES):
            name = chr(ord("A") + i)
            supply[name] = float(row[f"supply_{name}"])
            price[name] = float(row[f"price_{name}"])

        ce_time_series.append({
            "supply": supply,
            "price": price
        })

    return ce_time_series

# Lists for import/export         
CE_Import = []
LTE_Import = []
CE_Export = []
LTE_Export = []

def recalculate_export_capacity_ratio(ce_map, ce_time_series, time_step):
    """
    Recalculate the export capacity ratio for each CE product at a given time step.

    The export capacity ratio measures how much of a product's *normal internal demand*
    can be released for trade, given *current available supply*.

    Formula (for each product i):

        export_capacity_ratio_i(t)
            = current_supply_i(t) / equilibrium_total_demand_i

    where:
        current_supply_i(t) is the CE supply at time t
        equilibrium_total_demand_i is fixed and structural
    """

    for name, product in ce_map.items():
        # Update current supply from the simulation time step
        product.supply = ce_time_series[time_step]["supply"][name]

        # Recompute export capacity ratio using fixed equilibrium demand
        product.export_capacity_ratio = (
            product.supply / product.eq_total_demand
        )

# Note: This trade function only handles trade on a perticular time step. 
# If an item on the trading list can not be traded, then the shortage/surplus persists 
# On the next time step, the trade function will be called again and attempt to resolve the 
# previous shortage/surplus issue while the economy adjusts itself to try to satisfy its own needs 
def trade(ce_map, ce_time_series, lte_map, time_step):
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
    
    # Recalculate export capacity ratios for this time step.
    # This is the the reproduction requirement, 
    # as it measures how much the system can safely give up without harming reproduction.
    # For each CE product i,
    #   export_capacity_ratio_i(t) = current_supply_i(t) / equilibrium_total_demand_i
    # where equilibrium_total_demand_i = A·q + b + c 
    #  (What the letters represent is in simulation.py in the event based simulation)
    # The ratio therefore measures how much surplus supply capacity
    # is available for trade relative to internal needs.
    recalculate_export_capacity_ratio(ce_map, ce_time_series, time_step) 
    
    # Convert keys to sets for fast comparison
    ce_products = set(ce_map.keys())
    lte_products = set(lte_map.keys())
    
    # CE has but LTE doesn't - LTE must import
    for name in ce_products - lte_products:
        LTE_Import.append(name)

    # LTE has but CE doesn't - CE must import
    for name in lte_products - ce_products:
        CE_Import.append(name)
        
    for name in ce_map:                     # loop over product names
        product = ce_map[name]             # get the CapitalistProduct object

        # CE shortage detection 
        if product.supply < product.eq_supply:
            if name not in CE_Import:
                CE_Import.append(name)

        # CE Surplus detection 
        elif product.supply > product.eq_supply:
            CE_Export.append(name)

    # TODO:
    # 1. Check which products CE is missing - add to CE_Import - implemented 
    # 2. Check which products LTE is missing - add to LTE_Import - implemented 
    # 3. Detect shortages (CE) - add to CE_Import - implemented 
    # 4. Detect surpluses (CE) - add to CE_Export - implemented 
    # 5. Remove items when conditions disappear - implemented (by the design)
    # 6. Use a trade algorithm to determine which product should be traded for what, how much it should be traded, and for what price 
    # Ideas for 8: 
    # 1. Calculate the value of the products LTE would like to purchase from LTE_Import 
    # 2. Calculate the value of the products CE would like to purchase from CE_Import 
    # 2. Try to trade the amount of values desired by both economies. If not possible, there would be no trade. 
    
    # Note: The LTE is only going to trade if it is missing something. It will only overproduce when it needs to trade. 
    # We will make the LTE to be missing a few base products, put placeholders for the quantities of societal needs of that base product 
    # For the algorithm, each time step the LTE will be wanting to trade an amount of the base product which has a value of x labor time that can be converted to currency using MELT
    # Using the amount of total value, we can trade with the CE who has the list of things it wants to trade and try to add up everything 
    # We then apply the effects back to the CE, and CE will be wanting to trade for a new list of items each time step 
    
    # Questions:
    # Need to ask how to get data from LTE 
    # How to develop an algorithm for trading? (Or we can do this next week?) 
    # (Note: If can't get data from LTE, I need data from LTE for testing if the trading function actually work. What kind of data should I use?) 
    # What is a shortage? Is it when price > eq_price, supply < eq_supply, or both? I think it's both since it should happen at the same time. (Use the supply level to measure)
    
    # What is the equilibrium price and supply when it stablizes? 
    
    # A shortage for CE is when the supply level of a product is less than its equilibrium supply level 
    # Shortage should be detected by equilibrium supply  
    # Output is the activity level 

    print(f"[t={time_step}] Trade() executed")

if __name__ == "__main__":
    ce_csv_path = os.path.join(
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
    
    # Build CE and LTE maps
    ce_map = build_ce_map(ce_csv_path)
    lte_map = build_lte_map(lte_data_path)
    
    # Load CE data for access in O(1) time 
    ce_time_series = load_ce_post_equilibrium_series(ce_csv_path)

    # Display results
    print_maps(ce_map, lte_map)
    
    for i in range (NUMBER_OF_TIME_STEPS):
        # Place holder 
        trade(ce_map, ce_time_series, lte_map, i)
