# Simulation of Trade Between Capitalist and Labor-Time Economies

## The Event-Based Simulation: A Dynamic Capitalist Economy Backend for the Trade Simulation
### Description / Overview

The Event-Based Simulation implements a dynamic capitalist economy model designed to serve as the price- and supply-generating backend for this trade interface. Its purpose is to generate internally consistent trajectories of prices, outputs, supplies, labor usage, and value magnitudes under capitalist production relations, which can then be used by this trading system. It is built as a production-grounded and class-structured model. 

### Simulation Design

- Constructs a multi-commodity capitalist economy using a generated input–output matrix and labor coefficients
- Normalizes technical coefficients and the labor vector to ensure internally consistent production costs
- Initializes prices, supplies, outputs, wages, profits, and inventories from parameterized initial conditions
- Evolves the economy forward in discrete time by repeatedly applying a single event-based update step
- Records trajectories of prices, supplies, and related state variables at each time step
- Detects equilibrium by identifying a time step with no change in prices and supplies relative to the previous step
- Upon equilibrium detection, enables externally specified random events for a fixed post-equilibrium window (40 time steps)
- Applies the effects of enabled events directly to the economic state during simulation
- Exports full trajectory data with equilibrium annotations to CSV for downstream analysis
- Produces visualizations of individual commodity prices, supplies, and aggregate averages

### Theoretical Frameworks Used

- Leontief input-output model
- Classical political economy
- Marxian value theory

## The Trade Interface 
### Description / Overview 

