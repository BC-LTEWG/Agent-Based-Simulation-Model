import random
import numpy as np

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

# Minimum/Maximum hours required for random generation of the number of hours worth of 
# product i to make a unit of product j 
MINIMUM_HOURS_REQUIRED = 10
MAXIMUM_HOURS_REQUIRED = 90

# Normalizing Entry
# Normalize to ensure shares sum of products for 
# any product type in A is less than 1 
# If sum of products is greater than 1, it is not sustainable (using more than 1 to produce 1 product)
normalizing_value = 0

# Create a list of 26 independent empty lists with letters as key
dependencies = {chr(ord('A') + i): [] for i in range(NUMBER_OF_TOTAL_PRODUCTS)}
production_cost_map = {chr(ord('A') + i): 0.0 for i in range(NUMBER_OF_TOTAL_PRODUCTS)}

# Matrix for simulation 
A = np.zeros((NUMBER_OF_TOTAL_PRODUCTS, NUMBER_OF_TOTAL_PRODUCTS))
l = np.zeros(NUMBER_OF_TOTAL_PRODUCTS)

def generate_dependencies():
    
    # Generate dependencies such that:
    #   - I-Z depend on 2-5 random base products (A-H)
        
    number_of_dependent_product = NUMBER_OF_TOTAL_PRODUCTS - NUMBER_OF_BASE_PRODUCTS 

    # Derived products (I–Z) depend on base products A–H + labor 
    for i in range(number_of_dependent_product):  # 'I' to 'Z' 
        current_char = chr(ord('I') + i)

        num_deps = random.randint(2, 5)
        current_product_type_dependencies = []

        # Select unique base product dependencies (A–H)
        for _ in range(num_deps):
            selected_dependency = random.randint(0, NUMBER_OF_BASE_PRODUCTS - 1)
            selected_dependency_char = chr(ord('A') + selected_dependency)

            if selected_dependency_char not in current_product_type_dependencies:
                current_product_type_dependencies.append(selected_dependency_char)

        dependencies[current_char] = current_product_type_dependencies

    return dependencies

def generate_l():
    
    number_of_dependent_product = NUMBER_OF_TOTAL_PRODUCTS - NUMBER_OF_BASE_PRODUCTS
    
    # Labor values for base products 
    for i in range(NUMBER_OF_BASE_PRODUCTS):
        labor_value = random.uniform(MINIMUM_HOURS_REQUIRED, MAXIMUM_HOURS_REQUIRED) / 100
        l[i] = labor_value
    
    # Labor values for dependent products
    for i in range(number_of_dependent_product):
        current_product_type = chr(ord('I') + i)
        labor_value = random.uniform(MINIMUM_HOURS_REQUIRED, MAXIMUM_HOURS_REQUIRED) / 100
        for dependency in dependencies[current_product_type]:
            index_of_dependency = ord(dependency) - ord('A')
            labor_value += l[index_of_dependency]
        current_product_index = ord(current_product_type) - ord('A')
        l[current_product_index] = labor_value
        
    return l 

# This has to be run after generate l as it uses l value to get the largest column + l for the normalizing value 
# It should only be run once per simulation 
def construct_A_matrix_and_production_cost_map():
    global normalizing_value
    normalizing_value = 0
    
    production_cost_map['A'] = BASE_COST_A 
    production_cost_map['B'] = BASE_COST_B 
    production_cost_map['C'] = BASE_COST_C 
    production_cost_map['D'] = BASE_COST_D 
    production_cost_map['E'] = BASE_COST_E 
    production_cost_map['F'] = BASE_COST_F 
    production_cost_map['G'] = BASE_COST_G 
    production_cost_map['H'] = BASE_COST_H 
    
    number_of_dependent_product = NUMBER_OF_TOTAL_PRODUCTS - NUMBER_OF_BASE_PRODUCTS
    
    # Production cost for derived products (I–Z) 
    for i in range(number_of_dependent_product): 
        
        # I to Z (18 products) 
        current_product_type = chr(ord('I') + i) 
        col = ord(current_product_type) - ord('A')
        
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
            j = 0
            for dependency in dependencies[current_product_type]:
                weight_ratio = specific_weights[j] / total_weight
                sum_of_dependent_cost += weight_ratio * production_cost_map[dependency]
                j += 1
        
        # Calculate total cost for the derived product 
        production_cost_map[current_product_type] = base_processing_cost + sum_of_dependent_cost
        
        # Total = intermediate input + primary input (labor)
        # A_ij = value of input i used by industry j / total output of industry j
        if total_weight > 0:
            j = 0
            col_sum = 0
            for dependency in dependencies[current_product_type]:
                row = ord(dependency) - ord('A')
                cost = production_cost_map.get(dependency, 1.0)
                weight = specific_weights[j]
                A[row , col] = (weight * cost) / total_weight
                col_sum += A[row, col]
                j += 1
            
            # Calculating the normalizing value while constructing the matrix
            total = col_sum + l[col]
            if total > normalizing_value:
                normalizing_value = total
    
    # Return A_max, production_cost_map, normalizing value
    return A, production_cost_map, normalizing_value

# I chose to use l to normalize due to wanting to preserve 
# The ratio between each product i used to produce j in A to living labor to produce j
def normalize_A_and_l(A, l, normalizing_value):
    # scale both A-column and labor proportionally
    # sum * 1/sum = 1
    scale = 1.0 / normalizing_value
    A *= scale
    l *= scale
    return A, l

if __name__ == "__main__":
    deps = generate_dependencies()
    l = generate_l()
    A_matrix, costs, normalizing_value = construct_A_matrix_and_production_cost_map() 

    print(f"\nNormalizing Value (max column total A + l): {normalizing_value:.3f}") 

    A_matrix, l = normalize_A_and_l(A_matrix, l, normalizing_value)

    print("\nDEPENDENCY MAP:")
    for k in list(deps.keys()):
        print(f"{k}: {deps[k]}")

    print("\nPRODUCTION COST MAP:")
    for k, v in costs.items():
        print(f"{k}: {v:.3f}")

    print("\nLABOR VALUES:")
    for i, val in enumerate(l):
        print(f"{chr(ord('A') + i)}: {val:.3f}")

    print("\nA MATRIX:")
    for row in A_matrix:
        print(["{:.3f}".format(x) for x in row])
        
    print("\nCOLUMN TOTALS (A + l):")
    for j in range(len(A_matrix)):
        col_sum = sum(A_matrix[i][j] for i in range(len(A_matrix)))
        total = col_sum + l[j]
        print(f"{chr(ord('A') + j)}: {total:.3f}")



