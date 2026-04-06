import csv
import random 
import os
import sys
import importlib.util
import numpy as np
import matplotlib.pyplot as plt

from CapitalistEconomy import CapitalistEconomy as LocalCapitalistEconomy
from CapitalistProduct import CapitalistProduct
from LaborTimeEconomy import LaborTimeEconomy
from LaborTimeProduct import LaborTimeProduct 

# Load live CE modules from Event-Based Simulation.
EVENT_BASED_SIM_DIR = os.path.join(
    os.path.dirname(__file__),
    "..",
    "Event-Based Simulation"
)

sys.path.insert(0, EVENT_BASED_SIM_DIR)


def load_module_from_event_based_sim(filename, module_name):
    """Load one module from Event-Based Simulation."""
    module_path = os.path.join(EVENT_BASED_SIM_DIR, filename)
    spec = importlib.util.spec_from_file_location(module_name, module_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

# Import the live CE class and its setup helpers.
event_params_module = load_module_from_event_based_sim(
    "parameters.py",
    "event_params_module",
)
event_dep_module = load_module_from_event_based_sim(
    "generate_dependency_matrix.py",
    "event_dep_module",
)
event_random_module = load_module_from_event_based_sim(
    "random_events.py",
    "event_random_module",
)
event_ce_module = load_module_from_event_based_sim(
    "CapitalistEconomy.py",
    "event_ce_module",
)

Params = event_params_module.Params
generate_dependencies = event_dep_module.generate_dependencies
generate_l = event_dep_module.generate_l
construct_A_matrix_and_production_cost_map = (
    event_dep_module.construct_A_matrix_and_production_cost_map
)
normalize_A_and_l = event_dep_module.normalize_A_and_l
load_event_table = event_random_module.load_event_table
EventBasedSimulationCapitalistEconomy = event_ce_module.CapitalistEconomy

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

# Build the live CE instance 
def build_live_ce(debug=True):
    """Build the live CE instance from Event-Based Simulation."""
    generate_dependencies()
    l = generate_l()
    A_matrix, _, normalizing_value = construct_A_matrix_and_production_cost_map()
    A_matrix, l = normalize_A_and_l(A_matrix, l, normalizing_value)

    A = np.array(A_matrix)
    l = np.array(l)

    params = Params(
        A=A,
        l=l,
        b_bar=np.ones(len(l)) * 0.1,
        c_bar=np.ones(len(l)) * 0.05,
        alpha_w=0.8,
        alpha_c=0.7,
        alpha_L=0.0,
        kappa=np.ones(len(l)),
        eta=np.ones(len(l)) * 2,
        eta_w=0.25,
        L=1.0,
        eta_r=2.0,
        q0=np.ones(len(l)) * 0.1,
        p0=np.ones(len(l)) * 0.8,
        s0=np.ones(len(l)) * 0.2,
        m_w0=0.5,
        w0=0.5,
        r0=0.0,
        T=200,
    )

    ce = EventBasedSimulationCapitalistEconomy(params, debug=debug)
    ce.events_catalog = load_event_table()
    return ce

# Print a small state check for the live CE instance.
def debug_live_ce_instance(ce, num_steps=3):
    """Print a small live-state check for the CE instance."""
    print("\n--- LIVE CE DEBUG START ---")
    print(f"Number of commodities: {ce.n}")
    print(f"Initial current_t: {ce.current_t}")

    q, p, s, l, m_w, L = ce._split_state(ce.y)

    print("\nInitial state snapshot:")
    print(f"Commodity A price: {p[0]:.6f}")
    print(f"Commodity A supply: {s[0]:.6f}")
    print(f"Commodity A output: {q[0]:.6f}")
    print(f"m_w: {m_w:.6f}")
    print(f"L: {L:.6f}")

    for step_index in range(num_steps):
        ce.step()
        q, p, s, l, m_w, L = ce._split_state(ce.y)

        print(f"\nAfter ce.step() #{step_index + 1}:")
        print(f"current_t: {ce.current_t}")
        print(f"Commodity A price: {p[0]:.6f}")
        print(f"Commodity A supply: {s[0]:.6f}")
        print(f"Commodity A output: {q[0]:.6f}")
        print(f"m_w: {m_w:.6f}")
        print(f"L: {L:.6f}")

    print("\nHistory lengths:")
    print(f"len(ce.t): {len(ce.t)}")
    print(f"history['p'] shape: {ce.history['p'].shape}")
    print(f"history['s'] shape: {ce.history['s'].shape}")
    print("--- LIVE CE DEBUG END ---\n")

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

# Update the live CE instance per step 
# def update_ce():
    
def compute_melt(ce):
    # update the two fields here 
    ce.new_gdp_ppp_per_capita()
    ce.new_average_work_hours()
    # Compute MELT (money per labor-hour) using working population hours (incl. self-employed)
    ce.melt = ce.current_gdp_ppp_per_capita / ce.current_avg_hours_worked
    
def update_lte_map(ce, lte_map):
    for name, product in lte_map.items():
        product.value_of_goods_for_trade = (
            product.labor_time * ce.melt * product.order_size 
        )
    
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
    
    # Update LTE value of goods for trade at a given time step with the new melt 
    update_lte_map(ce, lte_map)
    
    # Determine tradable goods 
    # First convert lists to sets 
    # lte_export is the same of ce_import since lte would not produce excessively 
    # unless it is for the purpose of trade so there would only be arbitrary surplus 
    ce_import_set = set(CE_Import)
    ce_export_set = set(CE_Export)
    lte_import_set = set(LTE_Import)

    # CE exports goods LTE wants 
    ce_to_lte = ce_export_set & lte_import_set

    # LTE exports goods CE wants
    lte_to_ce = ce_import_set
    
    total_value_of_ce_to_lte = sum(
        ce_map[name].value_of_goods_for_trade
        for name in ce_to_lte
    )
    
    # The current set is always empty and I think it's due to the following reason: 
    #   CE only exports the protion that it can safely give up without harming reproduction, 
    #   so the only time it needs import is when some random events are so harmful that 
    #   it hurts CE's reproduction demand at that specific time step.
    #
    # Therefore, ce_import currently only activates only under reproduction stress.
    #
    # If CE remains self-reproducing, it never needs imports.
    # Balanced trade rule:
    # trade_value = min(total_exports, total_imports)
    #
    # If CE has no import demand, trade_value = 0.
    #
    # SInce CE is currently structurally self-sufficient,
    # trade does not occur.
    total_value_of_lte_to_ce = sum(
        lte_map[name].value_of_goods_for_trade
        for name in lte_to_ce
    )
    
    trade_value = min(
        total_value_of_ce_to_lte,
        total_value_of_lte_to_ce
    )
    
    print(f"Total CE->LTE value: {total_value_of_ce_to_lte:.4f}")
    print(f"Total LTE->CE value: {total_value_of_lte_to_ce:.4f}")
    print(f"Balanced trade value: {trade_value:.4f}")
    
    # Since we're only trading when trade is applicable, 
    # there would not be any surplus monetary value for both economies from trade 
    # CE pays for LTE exports
    ce.total_currency -= trade_value
    lte.total_currency += trade_value

    # LTE pays for CE exports
    lte.total_currency -= trade_value
    ce.total_currency += trade_value

    print(f"[t={time_step}] Trade() executed")

# Plot function for graphing 
def plot_currency(time, ce_history, lte_history):
    plt.figure(figsize=(10, 6))

    plt.plot(time, ce_history, label="CE Total Currency", linewidth=2)
    plt.plot(time, lte_history, label="LTE Total Currency", linewidth=2)

    plt.axhline(0, linestyle="--", linewidth=1)

    plt.xlabel("Time Step")
    plt.ylabel("Total Currency")
    plt.title("Monetary Values of CE and LTE Over Time")
    plt.legend()
    plt.grid(True)

    plt.savefig("graphs/Monetary Values.png", dpi=300)
    plt.close()

if __name__ == "__main__":

    # Build a live CE instance 

    lte_data_path = os.path.join(
        "..",
        "Agent-Based Simulation",
        "data"
    )
    
    usa = LocalCapitalistEconomy(
        name="United States",
        total_currency=0, 
        melt=0.0,  # placeholder 
        gdp_ppp_per_capita=85809.900385, # GDP per capita, PPP
        avg_hours_worked_by_working_population=1795.906,
        avg_hours_worked_by_employees_only=1809.583,
    )
    
    ce = build_live_ce(debug=True)
    
    lte = LaborTimeEconomy(
        name="Labor Time Economy",
        total_currency=0,
    )
    
    # Build CE instance
    
    # Build LTE map
    lte_map = build_lte_map(lte_data_path)

    # Display results
 #   print_maps(ce_map, lte_map)
 #   print_equilibrium_total_demand(ce_map)
    debug_live_ce_instance(ce, num_steps=3)
    
    # Record for graphing 
    # ce_currency_history = []
    # lte_currency_history = []
    # time_axis = []
    
    # for i in range (NUMBER_OF_TIME_STEPS):
    #     trade(usa, lte, ce_map, ce_time_series, lte_map, i)
        
    #     # Record currency values
    #     ce_currency_history.append(usa.total_currency)
    #     lte_currency_history.append(lte.total_currency)
    #     time_axis.append(i + 1)
        
    #     # Print for debug 
    #     print(f"Time step: {i+1}")
    #     print(f"CE currency: {usa.total_currency:.4f}")
    #     print(f"LTE currency: {lte.total_currency:.4f}")
    
    # plot_currency(time_axis, ce_currency_history, lte_currency_history)
