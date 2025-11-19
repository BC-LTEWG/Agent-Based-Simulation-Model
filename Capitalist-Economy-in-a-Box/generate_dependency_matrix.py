import csv
import random

BASE_COST_A = 1.0  # Common metal (iron/aluminum)
BASE_COST_B = 0.8  # Basic organic (wood/grain)
BASE_COST_C = 0.3  # Water/simple resource
BASE_COST_D = 1.5  # Energy (coal/gas)
BASE_COST_E = 2.0  # Oil/synthetic chemical
BASE_COST_F = 1.8  # Silicon/electronic substrate
BASE_COST_G = 2.5  # Rare mineral/alloy
BASE_COST_H = 1.2  # Textile/biological base

# Create a list of 26 independent empty lists with letters as key
dependencies = {chr(ord('A') + i): [] for i in range(26)}
production_cost_map = {chr(ord('A') + i): 0.0 for i in range(26)}

def generate_dependencies(number_of_base_products: int):
    
    # Generate dependencies such that:
    #   - A-H depend only on labor ('Z')
    #   - I-Y depend on 2-5 random base products (A-H) + labor ('Z')
    #   - Labor Z has no dependencies (root input)
      
    # Base products (A–H) depend only on labor 
    for base in [chr(ord('A') + i) for i in range(number_of_base_products)]:
        dependencies[base] = ['Z']

    # Derived products (I–Y) depend on base products A–H + labor 
    for i in range(17):  # 'I' to 'Y' (18 letters total but 'Z' is handled separately)
        current_char = chr(ord('I') + i)

        num_deps = random.randint(2, 5)
        current_product_type_dependencies = []

        # Select unique base product dependencies (A–H)
        for _ in range(num_deps):
            selected_dependency = random.randint(0, number_of_base_products - 1)
            selected_dependency_char = chr(ord('A') + selected_dependency)

            if selected_dependency_char not in current_product_type_dependencies:
                current_product_type_dependencies.append(selected_dependency_char)

        # Always include labor ('Z')
        current_product_type_dependencies.append('Z')

        dependencies[current_char] = current_product_type_dependencies
        
    dependencies['Z'] = []

    return dependencies

# QUESTION: How should the value of labor ('Z') be modeled?

# Context:
# - Every product (A-Y) in this system requires labor ('Z') as an input.
# - Labor ('Z') is currently treated as the original input (no dependencies).
# - The goal is to generate a dependency / cost matrix (26×26) that can be
#   exported as a CSV and used to replace the A matrix in the Leontief equation:
#
#       x = (I - A)^(-1) * y
#
#   where A is the input-output coefficients matrix.

# Current logic:
# - Base goods (A-H) depend only on 'Z' (living labor).
# - Derived goods (I-Y) depend on random combinations of base goods + 'Z'.
# - Labor ('Z') has no dependencies (root input).

# Output plan:
# - Final dependency network converted into a numeric matrix A (26×26)
# - stored as 'dependency_matrix.csv' for simulation and Leontief analysis.
def generate_production_cost_map():
    
    # Production cost of base products (A–H)
    production_cost_map['A'] = BASE_COST_A
    production_cost_map['B'] = BASE_COST_B
    production_cost_map['C'] = BASE_COST_C
    production_cost_map['D'] = BASE_COST_D
    production_cost_map['E'] = BASE_COST_E
    production_cost_map['F'] = BASE_COST_F
    production_cost_map['G'] = BASE_COST_G
    production_cost_map['H'] = BASE_COST_H

    # Production cost for derived products (I–Z)
    for i in range(18):  # I to Z (18 products)
        current_product_type = chr(ord('I') + i)

        # Base processing cost between 0.2 and 0.5
        base_processing_cost = random.uniform(0.2, 0.5)

        # Sum of dependent base product costs
        sum_of_dependent_cost = 0.0
        total_weight = 0.0
        specific_weights = []

        # Assign random weights for each dependency
        for dependency in dependencies[current_product_type]:
            w = random.uniform(0.1, 1.0)
            specific_weights.append(w)
            total_weight += w

        # Compute weighted average of dependency costs
        if total_weight > 0:
            for j, dependency in enumerate(dependencies[current_product_type]):
                weight_ratio = specific_weights[j] / total_weight
                sum_of_dependent_cost += weight_ratio * production_cost_map[dependency]

        # Calculate total cost for the derived product
        production_cost_map[current_product_type] = base_processing_cost + sum_of_dependent_cost

    return production_cost_map

if __name__ == "__main__":
    deps = generate_dependencies(8)
    costs = generate_production_cost_map()

    print("\nDEPENDENCY MAP:")
    for k in list(deps.keys())[0:26]:
        print(f"{k}: {deps[k]}")
        
    # print("\nPRODUCTION COST MAP:")
    # for k in list(costs.keys())[0:26]:
    #     print(f"{k}: {costs[k]:.3f}")
