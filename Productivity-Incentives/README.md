## Overview

The simulation models an economy where:
- **Firms** produce goods and services using labor time
- **Workers** are associated with firms and work on projects
- **Projects** represent production tasks with specific products, quantities, and labor requirements
- **PriceController** manages global price discovery and updates based on actual production costs

## Key Features

### Global Price Maps
- `official_prices`: Official labor time prices for goods/services (shirts, shoes, shorts, apples, etc.)
- `current_costs`: Average actual costs computed from production data
- `PriceController`: Agent that recalculates labor time costs for each cycle (look at simulation.cpp and Simulation.cpp to get a better understanding of how it works)

### Innovation System
- Random innovations reduce labor time requirements (10-30% reduction)
- Firms can discover and adopt innovations from other firms
- Successful innovations spread through the economy

### Dynamic Pricing
- PriceController monitors actual vs. official costs
- When improvements exceed thresholds (20% improvement, 0.5+ hour savings), official prices are updated
- This represents everyone's work day becoming shorter due to productivity gains

## Simulation Loop

Each cycle consists of 5 phases:

1. **Project Setting**: Firms create new production projects
2. **Innovation Discovery**: Firms may discover others' innovations
3. **Random Innovation**: Some firms randomly develop new efficiency improvements
4. **Production**: Projects are completed with actual labor time tracking
5. **Price Updates**: PriceController analyzes results and may update official prices



### Build Commands

```
# Compile simulation
make

# Run simulation
make run

# Clean build
make clean

# Debug build
make debug
```

### Compilation

```
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src/main.cpp src/Firm.cpp src/Worker.cpp -o simulation
./simulation
```

## Project Structure

```
Productivity-Incentives/
├── include/
│   ├── Firm.h          # Firm class and Project struct definitions
│   └── Worker.h        # Worker class definition
├── src/
│   ├── main.cpp        # Main simulation loop
│   ├── Firm.cpp        # Firm and PriceController implementations  
│   └── Worker.cpp      # Worker implementation
├── Makefile            # Build configuration
└── README.md         
```

## Key Classes

### Firm
- Manages workers and projects
- Tracks production costs and labor time
- Can develop and adopt innovations
- Records project history

### Worker  
- Associated with a specific firm
- Tracks labor time and current project
- Can work on projects and accumulate hours

### Project
- Represents a production task
- Contains product name, quantity, unit type
- Tracks expected vs. actual labor time
- Has an account for approved projects

### PriceController
- Manages global pricing system
- Computes average costs from all firms
- Updates official prices when efficiency thresholds are met
- Handles the transition to shorter work days

## Simulation Parameters

- **Innovation Probability**: 15% chance per firm per cycle
- **Discovery Probability**: 25% chance to find others' innovations  
- **Update Threshold**: 20% improvement needed for price updates
- **Minimum Savings**: 0.5 hours minimum improvement required
- **Firms**: 10 firms with 3 workers each
- **Products**: shirts, shoes, shorts, apples, bread, chairs, tables

## Expected Output

The simulation will show:
- Project creation and labor time estimates
- Innovation discoveries and developments
- Production results with actual labor time
- Price comparisons between official and current costs
- Price updates when efficiency improvements are significant
- Final summary of all firm activities


# The Productivity Incentives Sub-Project

This sub-project attempts to solve the problem of encouraging improvements in productivity within a labor-time-based economy.
When workers at a company discover a time-saving innovation, their actual work day becomes shorter, but the computed average
socially necessary work time for the same goods or services remains the same.  We must keep track of _both_ the current
average socially necessary work time _and_ the _actual_ work time required in actual companies.  Companies plan according 
to the official average socially necessary work time, not the actual work time.  Every so often, the actual average work time
averages are computed.  If enough sectors or products have improved their productivity enough to maintain about the same
cost of living when everybody's work day is shortened, the new averages replace the old official socially necessary work time
figures, which become the new standard.  Either everyone's work time is shortened in all sectors (so that costs relative
to each other stay about the same), or the society agrees to produce more of everything.

Probably, the recomputation of necessary work hours would go hand-in-hand with a recomputation of the social budget
in the Supply-Demand sub-project, when these two projects are integrated together.
