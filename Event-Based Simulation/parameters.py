from numpy import array, ndarray
from dataclasses import dataclass, asdict, is_dataclass
import numpy as np

# This dataclass contains all of the settings which are needed to specify the model as well as anything else of significance to the simulation
# (such as tolerances, time steps, etcetera)

@dataclass
class Params:
    A: ndarray # Initial requirements matrix. The number of commodities is inferred to be the dimension of this matrix, assume it equals n
    l: ndarray # Initial living labor n-vector.
    b_bar: ndarray # Worker consumption baseline. Workers will consume a scalar multiple of this n-vector each period using their savings
    c_bar: ndarray # The same as b_bar but for capitalists
    alpha_w: float # Worker propensity to consume. Workers will spend exactly this proportion of their savings on as many multiples of b_bar as that can purchase
    alpha_c: float # Capitalist propensity to consume. 
    alpha_L: float # The rate of growth of the labor pool
    kappa: ndarray # n-vector of elasticity constants. ith entry controls how dramatically ith output fluctuates with respect to the fluctuation of profit rates
    eta: ndarray # n-vector of elascitity constants. ith entry controls how dramatically ith price fluctuates with respect to changes in supply
    eta_w: float # controls how dramatically wages change with respect to employment and size of the reserve army
    eta_r: float # controls how dramatically the interest rate changes with respect to capitalist savings
    L: float # initial pool of available labor
    w0: float # initial hourly wage
    r0: float # initial interest rate. if set to zero, it will never change, i.e. the model will not have a credit system and capitalists draw their means of production from a free communal pool
    q0: ndarray # initial output n-vector
    p0: ndarray # initial price n-vector
    s0: ndarray # initial supply n-vector
    m_w0: float # initial worker savings. implicitly, there is a parameter M=1=total money in circulation. All money is posessed by either workers or capitalists. Thus whatever m_w0 is, initial capitalist savings will be 1-m_w
    alpha_l: float = 0.0 # rate of technological innovation. Each period the living labor vector will get scaled down by 1-this proportion
    # the remainder of these constants are purely technical in that they don't directly pertain to the economic scenario
    s_floor: float = 1e-5 # this and the next two constants just prevent the model from accidentally dividing by zero, they are not economically relevant
    eps_u: float = 1e-8
    eps_m: float = 1e-8
    T: int = 200 # number of time steps to simulate
    res: int = 3 # resolution, i.e. the number of points to sample per time step
