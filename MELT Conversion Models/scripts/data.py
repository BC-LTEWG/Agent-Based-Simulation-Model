import world_bank_data as wb
import csv
import pandas as pd

# Top 20 OECD countries with valid data in 2023 (using ISO 3-letter codes)
countries = {
    "Germany": "DEU",
    "Japan": "JPN",
    "United Kingdom": "GBR",
    "France": "FRA",
    "Italy": "ITA",
    "Canada": "CAN",
    "Spain": "ESP",
    "Australia": "AUS",
    "Mexico": "MEX",
    "Netherlands": "NLD",
    "Poland": "POL",
    "Belgium": "BEL",
    "Sweden": "SWE",
    "Ireland": "IRL",
    "Austria": "AUT",
    "Norway": "NOR",
    "Denmark": "DNK",
    "Czechia": "CZE",
    "Chile": "CHL",
    "Portugal": "PRT"  
}

# World Bank indicators
indicators = {
    "GDP_Nominal_Per_Capita": "NY.GDP.PCAP.CD",
    "GDP_PPP_Per_Capita": "NY.GDP.PCAP.PP.CD"
}

# Load CPI data for 2023 and 2024 from US_CPI.xlsx for deflator 
try:
    # Get data from file 
    cpi_df = pd.read_excel("US_CPI.xlsx", engine="openpyxl", skiprows=11)

    # Extract 2023 and 2024 rows
    cpi_2023 = cpi_df[cpi_df["Year"] == 2023].iloc[0]
    cpi_2024 = cpi_df[cpi_df["Year"] == 2024].iloc[0]

    # Compute annual averages 
    months = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
    avg_2023 = sum([cpi_2023[month] for month in months]) / 12
    avg_2024 = sum([cpi_2024[month] for month in months]) / 12
    deflator = avg_2023 / avg_2024

except Exception as e:
    print("Failed to load CPI data:", e)

# Load average annual wages (2023, measured in USD PPP converted 2024)
# Converted to USD PPP converted 2023 with deflator 
wage_df = pd.read_csv("OECD_mean_income.csv", skiprows=1, header=None)
wage_map = {}
adjusted_wage_map = {}
for country in countries:
    matched_row = wage_df[wage_df.iloc[:, 5] == country]
    if not matched_row.empty:
        wage_2024 = matched_row.iloc[0, 20]
        try:
            adjusted_wage_map[country] = float(wage_2024) * deflator
        except:
            adjusted_wage_map[country] = "Error"
    else:
        adjusted_wage_map[country] = "Missing"

# Load average hours worked (2023)
hours_df = pd.read_csv("OECD_average_hours_worked_2023.csv")
hours_map = dict(zip(hours_df["Country"], hours_df["Average hours worked per year"]))

# Compile results
results = []
for name, iso3 in countries.items():
    row = {"Country": name}
    for label, code in indicators.items():
        try:
            data = wb.get_series(code, country=iso3, date="2023")
            row[label] = float(data.dropna().values[0]) if not data.empty else "Missing"
        except Exception:
            row[label] = "Error"
    row["Avg_Annual_Wage_USD_PPP_2023"] = adjusted_wage_map.get(name, "Missing")
    row["Average_Hours_Worked_2023"] = hours_map.get(name, "Missing")
    results.append(row)

# Save results to CSV
with open("data.csv", "w", newline="", encoding="utf-8") as f:
    fieldnames = ["Country"] + list(indicators.keys()) + [
    "Avg_Annual_Wage_USD_PPP_2023",
    "Average_Hours_Worked_2023"
]
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(results)

print(f"{len(results)} countries' data has been fetched and combined.")
print("Data exported successfully to data.csv")

