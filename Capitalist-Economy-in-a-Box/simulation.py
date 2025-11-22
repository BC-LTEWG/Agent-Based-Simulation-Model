from CapitalistEconomy import *
from parameters import Params
from generate_dependency_matrix import *
import numpy as np

# generate A and l 
deps = generate_dependencies()
costs = generate_production_cost_map()
labor = generate_labor()
A_matrix = construct_A_matrix()
A_matrix, labor = normalize_A_and_l(A_matrix, labor)

A = np.array(A_matrix)
l = np.array(labor)

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

if __name__ == "__main__":
    traj, t, e = get_trajectories(params)
    if e:
        print("Simulation failed:", e)
    else:
        print("Simulation complete.")
        print("Final prices:", traj["p"][-1])
        print("Final outputs:", traj["q"][-1])
