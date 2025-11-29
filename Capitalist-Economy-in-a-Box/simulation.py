from parameters import Params
from CapitalistEconomy import *
from generate_dependency_matrix import *
import numpy as np

# generate A and l 
deps = generate_dependencies()
l = generate_l()
A_matrix, _ = construct_A_matrix()
A_matrix, l = normalize_A_and_l(A_matrix, l)

A = np.array(A_matrix)
l = np.array(l)

# Build params with generated A, l 
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
    T=100
)

# examples of using the CapitalistEconomy class. 
def get_trajectories(params):
    economy = CapitalistEconomy(params)

    e = None
    for i in range(params.T):
        try:
            economy.step()
        except Exception as error:
            e = error
            break

    traj, t = economy.traj, economy.t
    return traj, t, e

# example of perturbing the economy during simulation
def get_trajectories_supply_shock(params):
    economy = CapitalistEconomy(params)

    e = None
    for i in range(params.T):
        try:
            economy.step()
            if i == 50:
                s = economy.check_supply()
                deduction = 0.5*s
                economy.exo_supply_shock(deduction)
        except Exception as error:
            e = error
            break

    traj, t = economy.traj, economy.t
    return traj, t, e

def detect_equilibrium_timestamp(traj):
    # Detects the first time step when both prices and outputs
    # stop changing when stabilizes (Δp = 0 and Δq = 0). 

    # Parameters:
    #     traj (dict): trajectory data from the simulation,
    #                  must contain keys "p" (prices) and "q" (outputs).

    # Returns:
    #     int: timestamp (iteration index) where equilibrium is reached,
    #          or None if not stabilized by the end of the run.
             
    # Convert lists of price/output vectors into NumPy arrays for easy math
    prices = np.array(traj["p"])
    outputs = np.array(traj["q"])

    # Loop through all time steps, starting at t = 1 (since t=0 has no previous step)
    for t in range(1, len(prices)):
        # Calculate the difference between consecutive steps
        dp = prices[t] - prices[t - 1]
        dq = outputs[t] - outputs[t - 1]

        # Check if *all* elements in both Δp and Δq are exactly zero
        if np.all(dp == 0) and np.all(dq == 0):
            return t  # Found stabilization time

    # If we never found an exact zero change, return None
    return None

if __name__ == "__main__":
    traj, t, e = get_trajectories(params)
    if e:
        print("Simulation failed:", e)
    else:
        print("Simulation complete.")
        print("Final prices:", traj["p"][-1])
        print("Final outputs:", traj["q"][-1])
        
        total_steps = len(traj["p"])  # total number of time stamps
        print(f"\nTotal time steps simulated: {total_steps}")

        eq_time = detect_equilibrium_timestamp(traj)
        if eq_time is not None:
            print(f"Exact equilibrium reached at time step: {eq_time}")
        else:
            print("System never reached an exact equilibrium (no total zero change).")

# Plan for trade interface(initial thoughts): 
# These are maps that map from commodity to their prices 
# CE: [[ce_a, price_ce_a], [ce_b, price_ce_b], [ce_c, price_ce_c], …, [ce_y, price_ce_y]]
# LTE: [[lte_a, price_lte_a], [lte_b, price_lte_b], [lte_c, price_lte_c], …, [lte_y, price_lte_y]] 

# We only need to figure out when to trade for CE and when to trade for LTE in a free market 
# So we need two functions: 
# CE_Trade()

# LTE_Trade() 

# For each time stamp, 
# LTE checks if CE has a product type it doesn’t have. If so, LTE wants to trade for that product type with CE, adding it to the import list for that time stamp. 
# CE checks if LTE has a product type it doesn’t have. If so, CE wants to trade for that product type with LTE, adding it to the import list for that time stamp. 
# CE checks if there’s a shortage of products (prices that are higher than the start). If there is, add that product to the import list. 
# LTE checks if there’s a social need for a product (how to check this? Labor time doesn’t fluctuate.). If there is, add that product to the import list. 
# CE checks if there’s a surplus of products (if prices are lower than the start). If there is, add that product to the export list. 
# LTE checks if there’s a surplus of products (How to check this?). If there is, add that product to the export list. 
# Decide on a plan to trade products on the import and export lists (quantity? What for what? I can use MELT to convert labor time for price), and (record the currency change for the economy?). 
