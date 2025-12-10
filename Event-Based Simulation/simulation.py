from parameters import Params
from CapitalistEconomy import *
from generate_dependency_matrix import *
import matplotlib.pyplot as plt
import numpy as np

# generate A and l 
deps = generate_dependencies()
l = generate_l()
A_matrix, _, normalizing_value = construct_A_matrix_and_production_cost_map()
A_matrix, l = normalize_A_and_l(A_matrix, l, normalizing_value)

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

# Global to record equilibrium time index (first time detected)
EQUILIBRIUM_INDEX = None

# examples of using the CapitalistEconomy class. 
def get_trajectories(params):
    global EQUILIBRIUM_INDEX
    economy = CapitalistEconomy(params)

    e = None
    for i in range(params.T):
        try:
            economy.step()
            
            if EQUILIBRIUM_INDEX is None and detect_price_equilibrium(economy.traj):
                EQUILIBRIUM_INDEX = i
                
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

def detect_price_equilibrium(traj):
    # Returns True if equilibrium (no change in prices and outputs)
    # is reached at any time step, otherwise returns False.
    prices = np.array(traj["p"])
    outputs = np.array(traj["q"])

    for t in range(1, len(prices)):
        dp = prices[t] - prices[t - 1]
        dq = outputs[t] - outputs[t - 1]

        # Check if *all* elements in both Δp and Δq are exactly zero
        # if np.all(dp == 0) and np.all(dq == 0):
        # We need exact 0 for the simulation, but now for the graph we use this 
        if np.all(np.abs(dp) < 1e-4) and np.all(np.abs(dq) < 1e-4):
            return True  # Equilibrium found

    # Not yet reached by end of simulation
    return False

# Plot the graph for visualization here 
def plot_avg_price_output(traj, t, equilibrium_index):
    prices = np.array(traj["p"])
    outputs = np.array(traj["q"])

    avg_p = np.mean(prices, axis=1)
    avg_q = np.mean(outputs, axis=1)

    plt.figure(figsize=(10, 6))
    plt.plot(t, avg_p, label="Average Price", color="gold")
    plt.plot(t, avg_q, label="Average Output", color="black")

    if equilibrium_index is not None:
        plt.axvline(x=t[equilibrium_index], color="red", linestyle="--", label=f"Equilibrium (t={t[equilibrium_index]:.2f})")

    plt.xlabel("Time")
    plt.ylabel("Value")
    plt.title("Average Price and Output with Equilibrium Marker")
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    traj, t, e = get_trajectories(params)
    if e:
        print("Simulation failed:", e)
    else:
        print("Simulation complete.")
        print("Final prices:", traj["p"][-1])
        print("Final outputs:", traj["q"][-1])
        
    if EQUILIBRIUM_INDEX is not None:
        print(f"Equilibrium detected at time step {EQUILIBRIUM_INDEX}")
    else:
        print("No equilibrium detected during simulation.")

    plot_avg_price_output(traj, t, EQUILIBRIUM_INDEX)

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
