import pandas as pd
import matplotlib.pyplot as plt
import os

# Load the CSV
df = pd.read_csv("../data/output.csv")

# Define columns
columns = [
    "initial_value", "year_one", "year_two", "year_three", "year_four",
    "year_five", "year_six", "year_seven", "year_eight", "year_nine", "year_ten"
]

# Extract values
# ce_name = df.loc[0, "country_name"]
lte_name = df.loc[1, "country_name"]
# ce_values = df.loc[0, columns].values.astype(float)
lte_values = df.loc[1, columns].values.astype(float)

# X-axis positions and labels
x = list(range(len(columns)))  # [0, 1, ..., 10]
x_labels = ["Initial"] + [f"Y{i}" for i in range(1, 11)]

# Plotting
plt.figure(figsize=(10, 6))

# Fix axis position
ax = plt.gca()
ax.spines['left'].set_position('zero')  
ax.spines['left'].set_linewidth(1.2)
ax.spines['right'].set_color('none') 

plt.xlim(-0.5, len(x) - 0.5)

# Plot lines
# plt.plot(x, ce_values, label=ce_name.upper(), marker='o', linewidth=2)
plt.plot(x, lte_values, label=lte_name.upper(), marker='o', linewidth=2, color = "brown")
plt.axhline(0, color='black', linestyle='--', linewidth=1)

# Axis config
plt.xticks(ticks=x, labels=x_labels)
plt.xlabel("Year")
plt.ylabel("Currency Amount")
plt.title("Currency Amount Over Time")
plt.grid(True, linestyle='--', alpha=0.5)
plt.legend()
plt.tight_layout()

# Output folder
os.makedirs("../graphs", exist_ok=True)
plt.savefig("../graphs/free_market_simulation.png")

# Show plot
plt.show()
