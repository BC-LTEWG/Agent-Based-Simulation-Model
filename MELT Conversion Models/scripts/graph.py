import pandas as pd
import matplotlib.pyplot as plt
import os

# Load the CSV
df = pd.read_csv("../output.csv")

# Columns selected
columns_to_plot = [
    "export_price_gdp_nominal(20 labor hours to usd 2023)",
    "export_price_gdp_ppp(20 labor hours to international usd 2023)",
    "export_price_mean_income(20 labor hours to international usd 2023)",
    "import_price_gdp_nominal(2000 usd 2023 to labor hours)",
    "import_price_gdp_ppp(2000 usd 2023 to international usd 2023 to labor hours)",
    "import_price_mean_income(2000 usd 2023 to international usd 2023 to labor hours)"
]

# Output folder
os.makedirs("graphs", exist_ok=True)

# Create bar charts
for idx, column in enumerate(columns_to_plot, start=1):
    for i in range(2):  # First 10, Next 10
        subset = df.iloc[i*10:(i+1)*10]
        countries = subset["country_name"]
        values = subset[column]

        plt.figure(figsize=(12, 6))
        bars = plt.bar(countries, values)

        # Add value labels on top of each bar
        for bar in bars:
            height = bar.get_height()
            plt.text(
                bar.get_x() + bar.get_width() / 2,
                height,
                f"{height:.2f}",
                ha='center',
                va='bottom',
                fontsize=9,
                fontweight='bold'
            )

        plt.xlabel("Country")
        plt.ylabel("Value")
        plt.title(f"Graph {idx*2 - 1 + i}: {column}")
        plt.xticks(rotation=45, ha="right")
        plt.tight_layout()
        plt.savefig(f"graphs/graph_{idx*2 - 1 + i}.png")
        plt.close()
