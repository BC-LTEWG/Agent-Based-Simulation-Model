from parameters import Params
from CapitalistEconomy import *
from generate_dependency_matrix import *
from random_events import load_event_table
import matplotlib.pyplot as plt
import numpy as np
import csv

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
    T=200
)

# Global to record continuous equilibrium time (first time detected)
EQUILIBRIUM_TIME = None

def get_trajectories(params):
    global EQUILIBRIUM_TIME
    economy = CapitalistEconomy(params, debug=True)
    
    economy.events_catalog = load_event_table()

    e = None
    for i in range(params.T):
        try:
            economy.step()
            
            if not economy.detected_equilibrium:
                if detect_price_equilibrium(economy):
                    EQUILIBRIUM_TIME = economy.current_t
                    economy.detected_equilibrium = True   
                    economy.allow_random_events = True
                    economy.event_steps_remaining = 40
                    print(f"Equilibrium detected at continuous time t = {EQUILIBRIUM_TIME}")
                    
            if economy.allow_random_events and economy.event_steps_remaining == 0:
                economy.allow_random_events = False
                economy.force_end_events()
                            
        except Exception as error:
            e = error
            break

    traj, t = economy.history, economy.t
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

    traj, t = economy.history, economy.t
    return traj, t, e

def detect_price_equilibrium(economy):
    # Returns True if equilibrium (no change in prices and outputs)
    # is reached at any time step, otherwise returns False.
    if len(economy.history["p"]) < 2:
        return False
    
    dp = economy.history["p"][-1] - economy.history["p"][-2]
    dq = economy.history["q"][-1] - economy.history["q"][-2]
    
    return np.all(dp == 0) and np.all(dq == 0)

def plot_prices(time, prices, product_labels, equilibrium_time):
    # 1. Plot individual prices
    plt.figure(figsize=(12, 7))

    num_products = prices.shape[1]

    for i in range(num_products):
        plt.plot(time, prices[:, i], linewidth=1, alpha=0.7,
                 label=f"Price {product_labels[i]}")

    # draw continuous equilibrium time
    if equilibrium_time is not None:
        plt.axvline(x=equilibrium_time, color="red", linestyle="--", linewidth=2)

    plt.xlabel("Continuous Time (t)")
    plt.ylabel("Price Level")
    plt.title("Price Trajectories for All 26 Commodities")
    plt.grid(True)
    plt.legend(loc="upper right", fontsize=8, ncol=2)

    # Show all 200 time steps
    plt.xlim(0, time[-1])
    plt.savefig("graphs/Price Trajectories for All 26 Commodities.png", dpi=300)
    plt.close()


def plot_outputs(time, outputs, product_labels, equilibrium_time):
    # 2. Plot individual outputs 
    plt.figure(figsize=(12, 7))

    num_products = outputs.shape[1]

    for i in range(num_products):
        plt.plot(time, outputs[:, i], linewidth=1, alpha=0.7,
                 label=f"Output {product_labels[i]}")

    if equilibrium_time is not None:
        plt.axvline(x=equilibrium_time, color="red", linestyle="--", linewidth=2)

    plt.xlabel("Continuous Time (t)")
    plt.ylabel("Output Level")
    plt.title("Output Trajectories for All 26 Commodities")
    plt.grid(True)
    plt.legend(loc="upper right", fontsize=8, ncol=2)

    # Show all 200 time steps
    plt.xlim(0, time[-1])
    plt.savefig("graphs/Output Trajectories for All 26 Commodities.png", dpi=300)
    plt.close()

def plot_averages(time, avg_price, avg_output, equilibrium_time):
    # 3. Plot average prices + average outputs 
    plt.figure(figsize=(10, 6))
    plt.plot(time, avg_price, label="Average Price", color="gold")
    plt.plot(time, avg_output, label="Average Output", color="black")

    if equilibrium_time is not None:
        plt.axvline(x=equilibrium_time, color="red", linestyle="--",
                    label=f"Equilibrium at t={equilibrium_time:.2f}")

    plt.xlabel("Continuous Time (t)")
    plt.ylabel("Value")
    plt.title("Average Price and Average Output")
    plt.legend()
    plt.grid(True)

    # Show all 200 time steps
    plt.xlim(0, time[-1])
    plt.savefig("graphs/Average Price and Average Output.png", dpi=300)
    plt.close()

# Plot graphs for visualization 
def plot_graphs(traj, t, equilibrium_time):
    prices = np.array(traj["p"])
    outputs = np.array(traj["q"])

    avg_price = np.mean(prices, axis=1)
    avg_output = np.mean(outputs, axis=1)

    # Use continuous time array t for x-axis
    time = np.array(t)

    num_products = prices.shape[1]
    product_labels = [chr(ord('A') + i) for i in range(num_products)]

    plot_prices(time, prices, product_labels, equilibrium_time)
    plot_outputs(time, outputs, product_labels, equilibrium_time)
    plot_averages(time, avg_price, avg_output, equilibrium_time)
    
def export_trajectories_with_equilibrium(traj, t, equilibrium_time,
                                         filename="graphs/trajectories_with_equilibrium.csv"):
    prices = np.array(traj["p"])   # (T, 26)
    outputs = np.array(traj["q"])  # (T, 26)
    time = np.array(t)

    num_products = prices.shape[1]
    labels = [chr(ord('A') + i) for i in range(num_products)]

    # Find equilibrium index (closest time)
    eq_index = None
    if equilibrium_time is not None:
        eq_index = int(np.argmin(np.abs(time - equilibrium_time)))

    # Build header
    header = ["time", "is_equilibrium_step"]
    header += [f"price_{lbl}" for lbl in labels]
    header += [f"output_{lbl}" for lbl in labels]

    # Extra equilibrium summary columns
    header += ["equilibrium_time"]
    header += [f"equilibrium_price_{lbl}" for lbl in labels]
    header += [f"equilibrium_output_{lbl}" for lbl in labels]

    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)

        for i in range(len(time)):
            is_eq = 1 if (eq_index is not None and i == eq_index) else 0

            row = [time[i], is_eq]
            row += prices[i].tolist()
            row += outputs[i].tolist()

            # Only record equilibrium data at the exact equilibrium step
            if eq_index is not None and i == eq_index:
                row.append(time[eq_index])
                row += prices[eq_index].tolist()
                row += outputs[eq_index].tolist()
            else:
                row.append("")
                row += [""] * num_products
                row += [""] * num_products

            writer.writerow(row)

    print(f"Trajectory table exported to: {filename}")

if __name__ == "__main__":
    traj, t, e = get_trajectories(params)

    if e:
        print("Simulation failed:", e)
    else:
        print("Simulation complete.")
        print("Final prices:", traj["p"][-1])
        print("Final outputs:", traj["q"][-1])
    
    total_steps = len(t)
    print(f"\nTotal discrete time steps recorded: {total_steps}")

    if EQUILIBRIUM_TIME is not None:
        print(f"Equilibrium detected at continuous time t = {EQUILIBRIUM_TIME}")
    else:
        print("Equilibrium not reached.")
        
    # Export data points
    export_trajectories_with_equilibrium(traj, t, EQUILIBRIUM_TIME)
    
    # Plot results
    plot_graphs(traj, t, EQUILIBRIUM_TIME) 
