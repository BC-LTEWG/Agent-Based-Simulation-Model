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

NUMBER_OF_TOTAL_PRODUCTS = 26
NUMBER_OF_BASE_PRODUCTS = 8

MINIMUM_HOURS_WORKED = 10
MAXIMUM_HOURS_WORKED = 90

# Create a list of 26 independent empty lists with letters as key
dependencies = {chr(ord('A') + i): [] for i in range(NUMBER_OF_TOTAL_PRODUCTS)}
production_cost_map = {chr(ord('A') + i): 0.0 for i in range(NUMBER_OF_TOTAL_PRODUCTS)}

# Matrix for output 
A = [[0.0 for _ in range(NUMBER_OF_TOTAL_PRODUCTS)] for _ in range(NUMBER_OF_TOTAL_PRODUCTS)]
labor = [0.0 for _ in range(NUMBER_OF_TOTAL_PRODUCTS)]

def generate_dependencies():
    
    # Generate dependencies such that:
    #   - A-H depend only on labor ('Z')
    #   - I-Y depend on 2-5 random base products (A-H) + labor ('Z')
    #   - Labor Z has no dependencies (root input)
      
    # Base products (A–H) depend only on labor 
    for base in [chr(ord('A') + i) for i in range(NUMBER_OF_BASE_PRODUCTS)]:
        dependencies[base] = ['Z']
        
    number_of_dependent_product = NUMBER_OF_TOTAL_PRODUCTS - NUMBER_OF_BASE_PRODUCTS - 1

    # Derived products (I–Y) depend on base products A–H + labor 
    for i in range(number_of_dependent_product):  # 'I' to 'Y' (18 letters total but 'Z' is handled separately)
        current_char = chr(ord('I') + i)

        num_deps = random.randint(2, 5)
        current_product_type_dependencies = []

        # Select unique base product dependencies (A–H)
        for _ in range(num_deps):
            selected_dependency = random.randint(0, NUMBER_OF_BASE_PRODUCTS - 1)
            selected_dependency_char = chr(ord('A') + selected_dependency)

            if selected_dependency_char not in current_product_type_dependencies:
                current_product_type_dependencies.append(selected_dependency_char)

        # Always include labor ('Z')
        current_product_type_dependencies.append('Z')

        dependencies[current_char] = current_product_type_dependencies
        
    dependencies['Z'] = []

    return dependencies

# Current logic:
# - Base goods (A-H) depend only on 'Z' (living labor).
# - Derived goods (I-Y) depend on random combinations of base goods + 'Z'.
# - Labor ('Z') has no dependencies (root input).

# Notes from meeting (deleted when officially released)
# uniformly disttributed labor hours for Z (min/max) 
# Give it its own Z value, then add up the Z value of its base products 
# Two pull requests with and without graphing 
# 26 product types for LTE, assume labor time already built to explore trade 
def generate_production_cost_map(): 
    
    production_cost_map['A'] = BASE_COST_A 
    production_cost_map['B'] = BASE_COST_B 
    production_cost_map['C'] = BASE_COST_C 
    production_cost_map['D'] = BASE_COST_D 
    production_cost_map['E'] = BASE_COST_E 
    production_cost_map['F'] = BASE_COST_F 
    production_cost_map['G'] = BASE_COST_G 
    production_cost_map['H'] = BASE_COST_H 
    
    number_of_dependent_product = NUMBER_OF_TOTAL_PRODUCTS - NUMBER_OF_BASE_PRODUCTS - 1 
    
    # Production cost for derived products (I–Y) 
    for i in range(number_of_dependent_product): 
        # I to Y (17 products) 
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

def generate_labor():
    
    number_of_dependent_product = NUMBER_OF_TOTAL_PRODUCTS - NUMBER_OF_BASE_PRODUCTS - 1
    
    # Labor values for base products 
    for i in range(NUMBER_OF_BASE_PRODUCTS):
        labor_value = random.uniform(MINIMUM_HOURS_WORKED, MAXIMUM_HOURS_WORKED) / 100
        labor[i] = labor_value
    
    # Labor values for dependent products
    for i in range(number_of_dependent_product):
        current_product_type = chr(ord('I') + i)
        labor_value = random.uniform(MINIMUM_HOURS_WORKED, MAXIMUM_HOURS_WORKED) / 100
        for dependency in dependencies[current_product_type]:
            if dependency != 'Z':
                index_of_dependency = ord(dependency) - ord('A')
                labor_value += labor[index_of_dependency]
        current_product_index = ord(current_product_type) - ord('A')
        labor[current_product_index] = labor_value
        
    return labor 

def construct_A_matrix():
    
    for col in range(NUMBER_OF_TOTAL_PRODUCTS - 1): 
        output_char = chr(ord('A') + col)
        dependencies_list = dependencies.get(output_char, []) # fetch dependency list 
        
        # Random weights for dependencies (excluding Z)
        weights_for_dependencies = []
        total_weight = 0
        for dependency in dependencies_list:
            if dependency != 'Z':
                w = random.uniform(0.1, 1.0)
                weights_for_dependencies.append((dependency, w))
                total_weight += w
        
        # Total = intermediate input + primary input (labor)
        # A_ij = value of input i used by industry j / total output of industry j
        # Normalize to ensure shares sum to < 1 (leaving room for labor)
        for dependency, weight in weights_for_dependencies:
            row = ord(dependency) - ord('A')
            cost = production_cost_map.get(dependency, 1.0)
            if total_weight > 0:
                A[row][col] = (weight * cost) / total_weight 
            else: 
                A[row][col] = 0
    
    return A

def normalize_A_and_l(A, labor):
    n = len(A)
    for j in range(n):
        # column sum of intermediate inputs
        col_sum = sum(A[i][j] for i in range(n))
        total = col_sum + labor[j]

        if total > 1.0:
            # scale both A-column and labor proportionally
            # sum * 1/sum = 1
            scale = 1.0 / total
            for i in range(n):
                A[i][j] *= scale
            labor[j] *= scale
    return A, labor

if __name__ == "__main__":
    deps = generate_dependencies()
    costs = generate_production_cost_map()
    labor = generate_labor()
    A_matrix = construct_A_matrix()
    A_matrix, labor = normalize_A_and_l(A_matrix, labor)

    print("\nDEPENDENCY MAP:")
    for k in list(deps.keys()):
        print(f"{k}: {deps[k]}")

    print("\nPRODUCTION COST MAP:")
    for k, v in costs.items():
        print(f"{k}: {v:.3f}")

    print("\nLABOR VALUES:")
    for i, val in enumerate(labor):
        print(f"{chr(ord('A') + i)}: {val:.3f}")

    print("\nA MATRIX:")
    for row in A_matrix:
        print(["{:.3f}".format(x) for x in row])
        
    print("\nCOLUMN TOTALS (A + l):")
    for j in range(len(A_matrix)):
        col_sum = sum(A_matrix[i][j] for i in range(len(A_matrix)))
        total = col_sum + labor[j]
        print(f"{chr(ord('A') + j)}: {total:.3f}")



