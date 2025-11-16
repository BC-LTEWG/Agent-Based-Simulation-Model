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

#define BASE_COST_A 1.0 // Common metal (iron/aluminum), the numeraire
#define BASE_COST_B 0.8 // Basic organic (wood/grain)
#define BASE_COST_C 0.3 // Water/simple resource
#define BASE_COST_D 1.5 // Energy (coal/gas)
#define BASE_COST_E 2.0 // Oil/synthetic chemical
#define BASE_COST_F 1.8 // Silicon/electronic substrate
#define BASE_COST_G 2.5 // Rare mineral/alloy
#define BASE_COST_H 1.2 // Textile/biological base

// Base Labor Cost Ratios

// These represent normalized labor intensities for producing 1 unit of base products A–H.
// Definition of the concept:
// Labor intensity is how intense in terms of labor hours producing 1 unit of (any base product A-H) is.
// It is measured in labor-hours per unit mass of product (kg/tons/...).

// The numeraire (A) = 1.0, meaning:
//     It takes 1 labor hour to produce 1 unit of product A. (1 unit of product A can be 26 kg of product A, for example)
// Therefore, BASE_LABOR_X expresses how many labor-hours are required
// to produce 1 unit of product X, relative to A.
// Example:
//  - if BASE_LABOR_B = 1.2 -> producing 1 unit of B requires 1.2 labor-hours.
//  - if BASE_LABOR_C = 0.4 -> producing 1 unit of C requires 0.4 labor-hours.

#define BASE_LABOR_A 1.0 // Common metal — numeraire (1 labor-hour per ton)
                         // Baseline standard: moderately mechanized mining and smelting.
                         // Serves as the reference for all other labor intensities.

#define BASE_LABOR_B 1.2 // Basic organic — higher manual labor (1.2 hr/unit)
                         // Farming, forestry, and crop processing are more human-labor-intensive.
                         //  Despite lower material cost, requires more direct labor than metals.

#define BASE_LABOR_C 0.4 // Water/simple resource — low labor requirement
                         // Abundant and easily extracted; mostly pumping and transport.
                         // Reflects high automation and natural availability.

#define BASE_LABOR_D 0.8 // Energy (coal/gas) — semi-mechanized extraction
                         // Extraction and refining are mechanized but still require oversight.
                         // Labor share reduced by machinery; under 1.0 relative to A.

#define BASE_LABOR_E 0.6 // Oil/synthetic chemical — automated refining
                         // High capital intensity and refinery automation lower human input.
                         // Skilled operation but low hours per output unit.

#define BASE_LABOR_F 0.9 // Silicon/electronic substrate — skilled but efficient
                         // Requires fewer workers but high skill level.
                         // Labor cost balanced by productivity gains and precision tooling.

#define BASE_LABOR_G 1.5 // Rare mineral/alloy — labor-intensive, scarce
                         // Difficult extraction and refinement of rare ores.
                         // High human labor per ton, limited automation, small yields.

#define BASE_LABOR_H 1.3 // Textile/biological base — manual, cultivation-based
                         // Spinning, weaving, or growing fiber materials are labor-heavy.
                         // Mechanization helps but cannot eliminate manual work entirely.
