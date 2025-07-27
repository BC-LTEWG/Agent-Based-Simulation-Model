import world_bank_data as wb
import csv

# Country name → ISO-3 code
countries = {
    "United States": "USA", "China": "CHN", "Germany": "DEU", "India": "IND", "Japan": "JPN",
    "UK": "GBR", "France": "FRA", "Italy": "ITA", "Canada": "CAN", "Brazil": "BRA", "Russia": "RUS",
    "South Korea": "KOR", "Australia": "AUS", "Mexico": "MEX", "Turkey": "TUR", "Indonesia": "IDN",
    "Netherlands": "NLD", "Saudi Arabia": "SAU", "Poland": "POL", "Argentina": "ARG", "Sweden": "SWE",
    "Israel": "ISR", "Singapore": "SGP", "Thailand": "THA", "Austria": "AUT", "Philippines": "PHL",
    "Vietnam": "VNM", "Colombia": "COL", "South Africa": "ZAF", "Romania": "ROU", "Egypt": "EGY",
    "Iran": "IRN", "Peru": "PER", "Khazahkstan": "KAZ", "Algeria": "DZA", "Greece": "GRC", "Qatar": "QAT",
    "Nigeria": "NGA", "Uzbekistan": "UZB", "Kenya": "KEN", "Dominican Republic": "DOM",
    "Oman": "OMN", "Ivory Coast": "CIV", "Serbia": "SRB", "Belarus": "BLR", "Cambodia": "KHM", "Libya": "LBY",
    "Nepal": "NPL", "Zimbabwe": "ZWE", "Georgia": "GEO", "Mongolia": "MNG", "Lebanon": "LBN", "Syria": "SYR",
    "Afghanistan": "AFG", "Rwanda": "RWA", "Somalia": "SOM",
    "Maldives": "MDV", "Fiji": "FJI", "Solomon Islands": "SLB", "Samoa": "WSM",
    "Marshall Islands": "MHL", "Tuvalu": "TUV"
}

# World Bank indicators
indicators = {
    "GDP_Nominal_Per_Capita": "NY.GDP.PCAP.CD",
    "GDP_PPP_Per_Capita": "NY.GDP.PCAP.PP.CD"
}

# Compile results
results = []

for name, iso3 in countries.items():
    row = {"Country": name}
    for label, code in indicators.items():
        data = wb.get_series(code, country=iso3, date="2023")
        row[label] = float(data.dropna().values[0]) if not data.empty else "Missing"
    results.append(row)

# Save results to CSV
with open("data.csv", "w", newline="", encoding="utf-8") as f:
    writer = csv.DictWriter(f, fieldnames=["Country"] + list(indicators.keys()))
    writer.writeheader()
    writer.writerows(results)

print(f"{len(results)} countries' data has been fetched.")
print("Data exported successfully to data.csv")

