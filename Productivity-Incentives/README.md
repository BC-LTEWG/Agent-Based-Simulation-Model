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
└── README.md           # This file
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

## Economic Insights

This simulation demonstrates how:
- Innovations can spread through an economy
- Productivity improvements can reduce overall labor requirements
- Dynamic pricing can reflect real production efficiencies  
- Competition drives continuous improvement
- Everyone benefits when productivity increases (shorter work days)

