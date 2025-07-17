#include <iostream>
#include <string>

using namespace std;

// Labor-Time Economy
class LaborTimeEconomy
{
public:
    int LTEID;
    string economyName;

    LaborTimeEconomy(int id, string name) : LTEID(id), economyName(name) {}
};

// Capitalist Economy
/**/
class CapitalistEconomy
{
public:
    int CEID;
    std::string economyName;

    double GDPPerCapita;
    double PPPPerCapita;
    double medianIncome;
    double meanIncome;

    double avgWorkHoursPerYear;

    // Private Constructor With All Fields
private:
    CapitalistEconomy(int id, const std::string &name,
                      double gdp, double ppp,
                      double median, double mean,
                      double hours)
        : CEID(id), economyName(name),
          GDPPerCapita(gdp), PPPPerCapita(ppp),
          medianIncome(median), meanIncome(mean),
          avgWorkHoursPerYear(hours) {}

    /*
    CapitalistEconomy usa = CapitalistEconomy::FromGDP(1, "USA", 80000, 1600);
    CapitalistEconomy france = CapitalistEconomy::FromPPP(2, "France", 46000, 1550);
    CapitalistEconomy canada = CapitalistEconomy::FromMedianIncome(3, "Canada", 45000, 1600);
    CapitalistEconomy germany = CapitalistEconomy::FromMeanIncome(4, "Germany", 52000, 1600);
    */
public:
    static CapitalistEconomy FromGDP(int id, const std::string &name,
                                     double gdp, double hours)
    {
        return CapitalistEconomy(id, name, gdp, -1, -1, -1, hours);
    }

    static CapitalistEconomy FromPPP(int id, const std::string &name,
                                     double ppp, double hours)
    {
        return CapitalistEconomy(id, name, -1, ppp, -1, -1, hours);
    }

    static CapitalistEconomy FromMedianIncome(int id, const std::string &name,
                                              double median, double hours)
    {
        return CapitalistEconomy(id, name, -1, -1, median, -1, hours);
    }

    static CapitalistEconomy FromMeanIncome(int id, const std::string &name,
                                            double mean, double hours)
    {
        return CapitalistEconomy(id, name, -1, -1, -1, mean, hours);
    }

    // Utility methods
    bool usesGDP() const { return GDPPerCapita != -1; }
    bool usesPPP() const { return PPPPerCapita != -1; }
    bool usesMedian() const { return medianIncome != -1; }
    bool usesMean() const { return meanIncome != -1; }
};

// Product
class Product
{
public:
    int PID;
    string productName;
    double laborTimeValue; // Used in exports (from LTE to CE)
    double currencyValue;  // Used in imports (from CE to LTE)

    // Private unified constructor
    Product(int id, const string &name, double ltv, double cv)
        : PID(id), productName(name),
          laborTimeValue(ltv), currencyValue(cv) {}

    // Named constructor for export (LTE to CE)
    // Product solarPanel = Product::ForExport(1, "Solar Panel", 20);
    // 20 labor hours
    static Product ForExport(int id, const string &name, double laborHours)
    {
        return Product(id, name, laborHours, -1); // currency not used
    }

    // Named constructor for import (CE to LTE)
    // Product laptop = Product::ForImport(2, "Laptop", 1200);
    // $1200 market price
    static Product ForImport(int id, const string &name, double currency)
    {
        return Product(id, name, -1, currency); // labor time not used
    }

    bool isExport() const { return laborTimeValue != -1; }
    bool isImport() const { return currencyValue != -1; }
};

// Export transaction
class LTEExport
{
public:
    int EPID;
    int LTEID;
    int PID;
    int CEID;

    double convertedPrice_GDP;
    double convertedPrice_PPP;
    double convertedPrice_Median;
    double convertedPrice_Mean;

private:
    LTEExport(int epid, int lteid, int pid, int ceid,
              double priceGDP, double pricePPP,
              double priceMedian, double priceMean)
        : EPID(epid), LTEID(lteid), PID(pid), CEID(ceid),
          convertedPrice_GDP(priceGDP), convertedPrice_PPP(pricePPP),
          convertedPrice_Median(priceMedian), convertedPrice_Mean(priceMean) {}

public:
    // ===== Manual Constructors =====

    static LTEExport ManualGDP(int epid, int lteid, int pid, int ceid, double priceGDP)
    {
        return LTEExport(epid, lteid, pid, ceid, priceGDP, -1, -1, -1);
    }

    static LTEExport ManualPPP(int epid, int lteid, int pid, int ceid, double pricePPP)
    {
        return LTEExport(epid, lteid, pid, ceid, -1, pricePPP, -1, -1);
    }

    static LTEExport ManualMedian(int epid, int lteid, int pid, int ceid, double priceMedian)
    {
        return LTEExport(epid, lteid, pid, ceid, -1, -1, priceMedian, -1);
    }

    static LTEExport ManualMean(int epid, int lteid, int pid, int ceid, double priceMean)
    {
        return LTEExport(epid, lteid, pid, ceid, -1, -1, -1, priceMean);
    }

    // ===== Auto Constructors =====

    static LTEExport AutoGDP(int epid, int lteid, const Product &product, const CapitalistEconomy &ce)
    {
        double priceGDP = product.laborTimeValue * (ce.GDPPerCapita / ce.avgWorkHoursPerYear);
        return LTEExport(epid, lteid, product.PID, ce.CEID, priceGDP, -1, -1, -1);
    }

    static LTEExport AutoPPP(int epid, int lteid, const Product &product, const CapitalistEconomy &ce)
    {
        double pricePPP = product.laborTimeValue * (ce.PPPPerCapita / ce.avgWorkHoursPerYear);
        return LTEExport(epid, lteid, product.PID, ce.CEID, -1, pricePPP, -1, -1);
    }

    static LTEExport AutoMedian(int epid, int lteid, const Product &product, const CapitalistEconomy &ce)
    {
        double priceMedian = product.laborTimeValue * (ce.medianIncome / ce.avgWorkHoursPerYear);
        return LTEExport(epid, lteid, product.PID, ce.CEID, -1, -1, priceMedian, -1);
    }

    static LTEExport AutoMean(int epid, int lteid, const Product &product, const CapitalistEconomy &ce)
    {
        double priceMean = product.laborTimeValue * (ce.meanIncome / ce.avgWorkHoursPerYear);
        return LTEExport(epid, lteid, product.PID, ce.CEID, -1, -1, -1, priceMean);
    }

    // ===== Setters =====

    void setConvertedPriceGDP(double newPrice)
    {
        convertedPrice_GDP = newPrice;
        convertedPrice_PPP = -1;
        convertedPrice_Median = -1;
        convertedPrice_Mean = -1;
    }

    void setConvertedPricePPP(double newPrice)
    {
        convertedPrice_GDP = -1;
        convertedPrice_PPP = newPrice;
        convertedPrice_Median = -1;
        convertedPrice_Mean = -1;
    }

    void setConvertedPriceMedian(double newPrice)
    {
        convertedPrice_GDP = -1;
        convertedPrice_PPP = -1;
        convertedPrice_Median = newPrice;
        convertedPrice_Mean = -1;
    }

    void setConvertedPriceMean(double newPrice)
    {
        convertedPrice_GDP = -1;
        convertedPrice_PPP = -1;
        convertedPrice_Median = -1;
        convertedPrice_Mean = newPrice;
    }

    // ===== Check Active Mode =====

    bool usesGDP() const { return convertedPrice_GDP != -1; }
    bool usesPPP() const { return convertedPrice_PPP != -1; }
    bool usesMedian() const { return convertedPrice_Median != -1; }
    bool usesMean() const { return convertedPrice_Mean != -1; }
};

// Import transaction
class LTEImport
{
public:
    int IMID;
    int CEID;
    int PID;
    int LTEID;

    double convertedLaborTime_GDP;
    double convertedLaborTime_PPP;
    double convertedLaborTime_Median;
    double convertedLaborTime_Mean;

private:
    LTEImport(int imid, int ceid, int pid, int lteid,
              double timeGDP, double timePPP,
              double timeMedian, double timeMean)
        : IMID(imid), CEID(ceid), PID(pid), LTEID(lteid),
          convertedLaborTime_GDP(timeGDP),
          convertedLaborTime_PPP(timePPP),
          convertedLaborTime_Median(timeMedian),
          convertedLaborTime_Mean(timeMean) {}

public:
    // ===== Manual Constructors =====

    static LTEImport ManualGDP(int imid, int ceid, int pid, int lteid, double timeGDP)
    {
        return LTEImport(imid, ceid, pid, lteid, timeGDP, -1, -1, -1);
    }

    static LTEImport ManualPPP(int imid, int ceid, int pid, int lteid, double timePPP)
    {
        return LTEImport(imid, ceid, pid, lteid, -1, timePPP, -1, -1);
    }

    static LTEImport ManualMedian(int imid, int ceid, int pid, int lteid, double timeMedian)
    {
        return LTEImport(imid, ceid, pid, lteid, -1, -1, timeMedian, -1);
    }

    static LTEImport ManualMean(int imid, int ceid, int pid, int lteid, double timeMean)
    {
        return LTEImport(imid, ceid, pid, lteid, -1, -1, -1, timeMean);
    }

    // ===== Auto Constructors =====

    static LTEImport AutoGDP(int imid, const Product &product, const CapitalistEconomy &ce, int lteid)
    {
        double laborTime = product.currencyValue * (ce.avgWorkHoursPerYear / ce.GDPPerCapita);
        return LTEImport(imid, ce.CEID, product.PID, lteid, laborTime, -1, -1, -1);
    }

    static LTEImport AutoPPP(int imid, const Product &product, const CapitalistEconomy &ce, int lteid)
    {
        double laborTime = product.currencyValue * (ce.avgWorkHoursPerYear / ce.PPPPerCapita);
        return LTEImport(imid, ce.CEID, product.PID, lteid, -1, laborTime, -1, -1);
    }

    static LTEImport AutoMedian(int imid, const Product &product, const CapitalistEconomy &ce, int lteid)
    {
        double laborTime = product.currencyValue * (ce.avgWorkHoursPerYear / ce.medianIncome);
        return LTEImport(imid, ce.CEID, product.PID, lteid, -1, -1, laborTime, -1);
    }

    static LTEImport AutoMean(int imid, const Product &product, const CapitalistEconomy &ce, int lteid)
    {
        double laborTime = product.currencyValue * (ce.avgWorkHoursPerYear / ce.meanIncome);
        return LTEImport(imid, ce.CEID, product.PID, lteid, -1, -1, -1, laborTime);
    }

    // ===== Setters =====

    void setConvertedLaborTimeGDP(double newTime)
    {
        convertedLaborTime_GDP = newTime;
        convertedLaborTime_PPP = -1;
        convertedLaborTime_Median = -1;
        convertedLaborTime_Mean = -1;
    }

    void setConvertedLaborTimePPP(double newTime)
    {
        convertedLaborTime_PPP = newTime;
        convertedLaborTime_GDP = -1;
        convertedLaborTime_Median = -1;
        convertedLaborTime_Mean = -1;
    }

    void setConvertedLaborTimeMedian(double newTime)
    {
        convertedLaborTime_Median = newTime;
        convertedLaborTime_GDP = -1;
        convertedLaborTime_PPP = -1;
        convertedLaborTime_Mean = -1;
    }

    void setConvertedLaborTimeMean(double newTime)
    {
        convertedLaborTime_Mean = newTime;
        convertedLaborTime_GDP = -1;
        convertedLaborTime_PPP = -1;
        convertedLaborTime_Median = -1;
    }

    // ===== Active Mode Checkers =====

    bool usesGDP() const { return convertedLaborTime_GDP != -1; }
    bool usesPPP() const { return convertedLaborTime_PPP != -1; }
    bool usesMedian() const { return convertedLaborTime_Median != -1; }
    bool usesMean() const { return convertedLaborTime_Mean != -1; }
};

// Test main
int main()
{
    // Create export product (same labor-time for all)
    Product solarPanel = Product::ForExport(101, "Solar Panel", 20); // 20 labor hours

    // Create import products in local currency $1200
    Product laptopCanada = Product::ForImport(201, "Laptop", 1650.38);
    Product laptopFrance = Product::ForImport(202, "Laptop", 1035.88);
    Product laptopGermany = Product::ForImport(203, "Laptop", 1035.88);
    Product laptopJapan = Product::ForImport(204, "Laptop", 178390.2);

    // Median income countries
    CapitalistEconomy canada = CapitalistEconomy::FromMedianIncome(1, "Canada", 74200, 1875);
    CapitalistEconomy france = CapitalistEconomy::FromMedianIncome(2, "France", 42800, 1487.1);

    // Mean income countries
    CapitalistEconomy germany = CapitalistEconomy::FromMeanIncome(3, "Germany", 51876, 1343);
    CapitalistEconomy japan = CapitalistEconomy::FromMeanIncome(4, "Japan", 4600000, 1611);

    // Export calculations
    LTEExport expCanada = LTEExport::AutoMedian(301, 1001, solarPanel, canada);
    LTEExport expFrance = LTEExport::AutoMedian(302, 1001, solarPanel, france);
    LTEExport expGermany = LTEExport::AutoMean(303, 1001, solarPanel, germany);
    LTEExport expJapan = LTEExport::AutoMean(304, 1001, solarPanel, japan);

    // Import calculations (different product instances with local prices)
    LTEImport impCanada = LTEImport::AutoMedian(401, laptopCanada, canada, 1001);
    LTEImport impFrance = LTEImport::AutoMedian(402, laptopFrance, france, 1001);
    LTEImport impGermany = LTEImport::AutoMean(403, laptopGermany, germany, 1001);
    LTEImport impJapan = LTEImport::AutoMean(404, laptopJapan, japan, 1001);

    // Output results
    cout << "===== Export Prices (20 labor hours) =====" << endl;
    cout << "Canada (Median):   C$" << expCanada.convertedPrice_Median << endl;
    cout << "France (Median):   €" << expFrance.convertedPrice_Median << endl;
    cout << "Germany (Mean):    €" << expGermany.convertedPrice_Mean << endl;
    cout << "Japan (Mean):      ¥" << expJapan.convertedPrice_Mean << endl;

    cout << "\n===== Import Labor Time (Laptop in Local Currency) =====" << endl;
    cout << "Canada (C$):   " << impCanada.convertedLaborTime_Median << " hrs" << endl;
    cout << "France (€):    " << impFrance.convertedLaborTime_Median << " hrs" << endl;
    cout << "Germany (€):   " << impGermany.convertedLaborTime_Mean << " hrs" << endl;
    cout << "Japan (¥):   " << impJapan.convertedLaborTime_Mean << " hrs" << endl;

    return 0;
}

/*
Things to consider #1
Adding companies as independent agents along side the government
with their own surplus values, breaking LTE and CE down into agents.
Both the LTE's government + companies and the CE's government + companies
will be charging each other surplus values to reach a balance.

This will break the current structure since we are
currently only talking about trading on the national economy level as a whole,
without considering LTE charging surplus values.
Allowing trading between companies and government agencies as agents
will make it a whole other story.
The entity relationship diagram will have to be reconsidered.
*/

/*
Things to consider #2
We should also be adding an environmental class,
since international relationships and inflation will affect trading.

We will only be having one LTE economy for now, but more can be added later.
LTE to LTE trade is easier to set up.
*/
