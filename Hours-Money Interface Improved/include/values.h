#pragma once

#define INVALID_VALUE -1
#define INITIAL_HOURS 1600
#define INITIAL_EARNINGS_PER_CAPITA 80000
#define NUMBER_OF_BASE_PRODUCTS 8
#define NUMBER_OF_CYCLES 40          // Change cycles to 40 based on cycles
#define CYCLES_FOR_PRODUCT_REMOVAL 8 // 8 quarters until removal. If it stops losing money, restart the counter.
#define CYCLES_FOR_COMPANY_REMOVAL 20

// Base Product Cost Ratios
// These represent normalized fixed ratios for A-H (used in hybrid model).
// Units are arbitrary but ratio-preserving (fictional currency like CE dollar in this case)
// Final base_costs[p] = FIXED_VALUE[p] * random_factor(0.8–1.2) applied at runtime.

#define BASE_COST_A 1.0 // Common metal (iron/aluminum)
#define BASE_COST_B 0.8 // Basic organic (wood/grain)
#define BASE_COST_C 0.3 // Water/simple resource
#define BASE_COST_D 1.5 // Energy (coal/gas)
#define BASE_COST_E 2.0 // Oil/synthetic chemical
#define BASE_COST_F 1.8 // Silicon/electronic substrate
#define BASE_COST_G 2.5 // Rare mineral/alloy
#define BASE_COST_H 1.2 // Textile/biological base
