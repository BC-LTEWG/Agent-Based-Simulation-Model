#include "../include/Simulation.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <limits>
#include <matplot/matplot.h>
#include <sstream>

Simulation::Simulation() : priceController(std::map<std::string, double>(), std::map<std::string, std::string>()), gen(std::random_device{}()), probability(0.0, 1.0), nextProjectId(1), nextFirmId(1), planCycle(0), innerCycle(0), outerCycle(0) {
    
    // Initialize PriceController with proper product data
    std::map<std::string, double> initialPrices;
    std::map<std::string, std::string> units;
    
    // Create initial prices map from products data
    for (const auto& [productName, productData] : products) {
        double initialPrice = std::get<1>(productData); // Get initial price from tuple
        initialPrices[productName] = initialPrice;
        units[productName] = "hours"; // All products use hours as unit
    }
    
    // Reinitialize PriceController with the proper data
    priceController = PriceController(initialPrices, units);
    
    saveInitialPrices();
    categorizeProductsByLaborIntensity();
    createFirms();

    // currentProductCosts removed - using PriceController's current_prices instead

    // Initialize labor intensity bias (research-based values)
    // Higher values = more labor-intensive = harder to innovate/automate
    for (const auto& [productName, firmData] : products) {
        double intensity = 0.5; // default medium
        
        // Healthcare: Highly labor-intensive (personal care, skilled professionals)
        if (productName == "healthcare") {
            intensity = 0.85; // Very high - difficult to automate personal care
        }
        
        // Food/Agriculture: High labor-intensity (delicate handling, perishable)
        else if (productName == "apples" || productName == "bread") {
            intensity = 0.65; // High - fruit picking, baking require manual work
        }
        
        // Textiles/Apparel: Medium-high labor-intensity (cutting, sewing)
        else if (productName == "shirts" || productName == "shorts") {
            intensity = 0.55; // Medium-high - sewing still requires significant manual work
        }
        
        // Furniture: Medium labor-intensity (some automation possible)
        else if (productName == "chairs" || productName == "tables") {
            intensity = 0.45; // Medium - woodworking can be partially automated
        }
        
        // Footwear: Medium labor-intensity (similar to textiles but more automated)
        else if (productName == "shoes") {
            intensity = 0.50; // Medium - shoe manufacturing has some automation
        }
        
        productLaborIntensity[productName] = intensity;
        // Removed output for speed: Labor intensity info
    }
    
    // Initialize workday tracking with initial value
    double initialWorkday = 0.0;
    for (const auto& [productName, productData] : products) {
        double initialPrice = std::get<1>(productData); // Get initial price from tuple
        initialWorkday += initialPrice;
    }
    workDayAtInnerCycle.push_back(initialWorkday);
    
    // Save initial state for cycle 0 before any modifications
    // Store the initial current costs for cycle 0 pie chart
    std::map<std::string, double> cycle0Costs;
    for (const auto& [productName, productData] : products) {
        double initialPrice = std::get<1>(productData); // Get initial price from tuple
        cycle0Costs[productName] = initialPrice;
    }
    
    // Generate initial pie chart showing baseline prices using saved cycle 0 data
    generateWorkerBudgetPieChart(0, cycle0Costs);
    
    // Add initial workday tracking for cycle 0
    workDayAtOuterCycle.push_back(0);
    workDayValuesAtOuterCycle.push_back(initialWorkday); // Use calculated initial workday value
    
    // Removed output for speed: Constructor completed
}

void Simulation::innovationDiscoveryPhase() {
    for (auto& [productName, productData] : products) {
        bool& innovation_occurred = std::get<2>(productData);
        if (innovation_occurred) {
            double lowestPrice = std::numeric_limits<double>::max();
            
            // Get the vector of firm names from the tuple
            const std::vector<std::string>& firms = std::get<0>(productData);
            
            // Find the lowest price among firms making this product
            for (const auto& firmName : firms) {
                auto it = firmPrices.find(firmName);
                if (it != firmPrices.end()) {
                    lowestPrice = std::min(lowestPrice, it->second);
                }
            }
            
            // Set all firms making this product to the lowest price
            for (const auto& firmName : firms) {
                auto it = firmPrices.find(firmName);
                if (it != firmPrices.end()) {
                    it->second = lowestPrice;
                }
            }
            
            // Reset innovation flag for this product
            innovation_occurred = false;
        }
    }
}

void Simulation::saveInitialPrices() {
    // Save initial prices for each product
    for (const auto& [productName, productData] : products) {
        double initialPrice = std::get<1>(productData); // Get initial price from tuple
        initialPrices[productName] = initialPrice;
        priceHistory[productName] = std::vector<double>();
    }
}

void Simulation::trackPricesAndWorkDay(int cycle) {
    cycleNumbers.push_back(cycle);
    
    // Track current prices for each firm
    for (const auto& [firmName, price] : firmPrices) {
        priceHistory[firmName].push_back(price);
    }
    
    // Track work day hours as sum of all firm prices (stepwise)
    double sumBasket = 0.0;
    for (const auto& [firmName, price] : firmPrices) sumBasket += price;
    workDayHours.push_back(sumBasket);
    
    // Only generate pie chart when we have a price cycle (not every inner cycle)
    // This will be called separately when price cycles advance
}

void Simulation::saveResultsToCSV() {
    std::ofstream csvFile("simulation_results.csv");
    
    // Header
    csvFile << "Cycle,OuterCycle,InnerCycle,WorkDayHours,EventType,Product,OldPrice,NewPrice,InnovationAmount\n";
    
    // Track all events
    for (int cycle = 0; cycle <= cycleNumbers.back(); ++cycle) {
        // Find if this cycle had an outer cycle increment
        bool hadOuterCycle = false;
        int outerCycleNum = 0;
        for (size_t i = 0; i < workDayAtOuterCycle.size(); ++i) {
            if (workDayAtOuterCycle[i] == cycle) {
                hadOuterCycle = true;
                outerCycleNum = i + 1;
                break;
            }
        }
        
        // Find if this cycle had an inner cycle increment
        bool hadInnerCycle = false;
        int innerCycleNum = 0;
        for (size_t i = 0; i < workDayAtInnerCycleGeneral.size(); ++i) {
            if (workDayAtInnerCycleGeneral[i] == cycle) {
                hadInnerCycle = true;
                innerCycleNum = i + 1;
                break;
            }
        }
        
        // Get workday value for this cycle
        double workdayValue = 8.0; // Default starting value
        if (hadOuterCycle && outerCycleNum <= workDayAtInnerCycle.size()) {
            workdayValue = workDayAtInnerCycle[outerCycleNum - 1];
        } else if (hadInnerCycle && innerCycleNum <= workDayAtInnerCycle.size()) {
            workdayValue = workDayAtInnerCycle[innerCycleNum - 1];
        }
        
        // Write cycle data
        csvFile << cycle << "," 
                << (hadOuterCycle ? outerCycleNum : 0) << ","
                << (hadInnerCycle ? innerCycleNum : 0) << ","
                << std::fixed << std::setprecision(3) << workdayValue << ","
                << (hadOuterCycle ? "OUTER_CYCLE" : (hadInnerCycle ? "INNER_CYCLE" : "NO_CHANGE")) << ","
                << "N/A" << ","  // Product
                << "N/A" << ","  // OldPrice
                << "N/A" << ","  // NewPrice
                << "N/A" << "\n"; // InnovationAmount
    }
    
    csvFile.close();
    // Results saved to simulation_results.csv
    
    // Also save detailed innovation tracking
    std::ofstream innovationFile("innovation_details.csv");
    innovationFile << "Cycle,SimulationType,EventType,Product,OldPrice,NewPrice,InnovationAmount,WorkDayBefore,WorkDayAfter\n";
    
    // Track innovations from both simulations
    // This would need to be enhanced to capture actual innovation data during runtime
    innovationFile.close();
    // Innovation details saved to innovation_details.csv
}

void Simulation::createFirms() {
    // Create NUM_FIRMS firms
    for (int i = 0; i < NUM_FIRMS; ++i) {
        auto firm = std::make_shared<Firm>(nextFirmId++, firmNames[i], priceController);
        
        
        
        firms.push_back(firm);
    }
    
    firmSelector = std::uniform_int_distribution<>(0, firms.size() - 1);
    // Removed output for speed: Created firms with workers
}

bool Simulation::priceControllerPhase() {
    // Collect all completed projects (no output)
    std::vector<Project> completedProjects;
    for (const auto& firm : firms) {
        const auto& projects = firm->getProjects();
        for (const auto& project : projects) {
            if (project.actualCost > 0) {
                completedProjects.push_back(project);
            }
        }
    }
    
    // Update official prices if conditions are met (no output)
    double beforeSum = 0.0; for (const auto& [productName, productData] : products) beforeSum += priceController.getOfficialPrice(productName);
    // updateOfficialPrices removed - no longer using official price system
    double afterSum = 0.0; for (const auto& [productName, productData] : products) afterSum += priceController.getOfficialPrice(productName);
    if (afterSum < beforeSum) {
        // Price cycle increment - this is when we generate pie charts
        int nextPriceCycle = priceCycleNumbers.empty() ? 1 : (priceCycleNumbers.back() + 1);
        priceCycleNumbers.push_back(nextPriceCycle);
        double workday = 0.0; for (const auto& [productName, productData] : products) workday += priceController.getCurrentCost(productName);
        workDayAtPriceCycle.push_back(workday);
        
        // Generate pie chart for this price cycle (no console output)
        generateWorkerBudgetPieChart(nextPriceCycle);
        
        return true; // Signal that prices were updated
    }
    return false; // No price update
}

bool Simulation::runCycle(int cycleNumber) {
    // Minimal output - only show every 500th cycle
    if (cycleNumber % 500 == 0) {
        std::cout << "Cycle " << cycleNumber << std::endl;
    } 
    innovationDiscoveryPhase();
    // Plan-cycle: apply stochastic innovations to basket costs
    applyPlanCycleInnovations(cycleNumber);

    // Phase 1: Reset firm labor time tracking for this cycle
    for (auto& firm : firms) {
        firm->resetCycleLaborTime();
    }
    
    // Phase 2: Each firm creates projects (no output)
    for (auto& firm : firms) {
        firm->create_projects();
    }
    
    // Phase 3: Each firm executes projects (no output)
    for (auto& firm : firms) {
        firm->execute_projects(firms);
    }
    
    // Phase 4: Record labor time for this cycle (no output)
    for (auto& firm : firms) {
        double cycleLaborTime = firm->getTotalLaborTimeSpent() - 
            (firm->getLaborTimeHistory().size() > 0 ? 
             std::accumulate(firm->getLaborTimeHistory().begin(), firm->getLaborTimeHistory().end(), 0.0) : 0.0);
        firm->recordLaborTimeForCycle(cycleLaborTime);
    }
    
    // Phase 5: Price Controller updates (minimal output)
    bool priceUpdated = priceControllerPhase();
    trackPricesAndWorkDay(cycleNumber);
    
    // No delay for speed
    
    return priceUpdated;
}

void Simulation::showSummary() {
    std::cout << "=== SIMULATION SUMMARY ===\n";
    std::cout << "Total firms: " << firms.size() << "\n";
    std::cout << "Total projects created: " << nextProjectId - 1 << "\n";
    
    std::cout << "\nInitial official prices:\n";
    for (const auto& [productName, productData] : products) {
        std::cout << productName << ": " << initialPrices[productName] << " hours\n";
    }
    
    std::cout << "\nFinal official prices:\n";
    for (const auto& [productName, productData] : products) {
        double initial = initialPrices[productName];
        double final = priceController.getOfficialPrice(productName);
        double delta = final - initial;
        double percentChange = (initial > 0) ? (delta / initial) * 100.0 : 0.0;
        
        std::cout << productName << ": " << final << " hours (delta: " << std::showpos 
                 << delta << std::noshowpos << " hours, " << std::showpos 
                 << percentChange << std::noshowpos << "%)\n";
    }
    
    std::cout << "\nFinal current costs (actual productivity):\n";
    for (const auto& [productName, productData] : products) {
        double initial = initialPrices[productName];
        double current = priceController.getCurrentCost(productName);
        double delta = current - initial;
        double percentChange = (initial > 0) ? (delta / initial) * 100.0 : 0.0;
        
        std::cout << productName << ": " << current << " hours (delta: " << std::showpos 
                 << delta << std::noshowpos << " hours, " << std::showpos 
                 << percentChange << std::noshowpos << "%)\n";
    }
    
    std::cout << "\nFirm production summary:\n";
    for (const auto& firm : firms) {
        std::cout << firm->getName() << ": " << firm->getProjects().size() 
                 << " projects, " << firm->getTotalLaborTimeSpent() << " total hours\n";
        
        // Show labor time history
        const auto& history = firm->getLaborTimeHistory();
        std::cout << "  Labor time by cycle: ";
        for (size_t i = 0; i < history.size(); ++i) {
            std::cout << "C" << (i + 1) << ":" << std::fixed << std::setprecision(1) << history[i] << "h ";
        }
        std::cout << "\n";
        
        // Show labor time by product
        std::cout << "  Labor time by product: ";
        for (const auto& [productName, productData] : products) {
            double productTime = firm->getLaborTimeForProduct(productName);
            if (productTime > 0) {
                std::cout << productName << ":" << std::fixed << std::setprecision(1) << productTime << "h ";
            }
        }
        std::cout << "\n";
    }
    
}

void Simulation::run(int numCycles) {
    std::cout << "Starting Economic Simulation with Labor Time Value\n";
    std::cout << "================================================\n\n";
    
    // Add cycle 0 to show the baseline of 8 hours before any simulation
    cycleNumbers.push_back(0);
    {
        double baseline = 0.0;
        for (const auto& [firmName, price] : firmPrices) baseline += price;
        workDayHours.push_back(baseline);
    }
    priceCycleNumbers.clear();
    workDayAtPriceCycle.clear();
    priceCycleNumbers.push_back(0);
    workDayAtPriceCycle.push_back(workDayHours.back());
    
    // Track initial prices for cycle 0
    for (const auto& [firmName, price] : firmPrices) {
        priceHistory[firmName].push_back(initialPrices[firmName]);
    }
    
    std::cout << "Cycle 0 (Baseline): Work day = " << workDayHours.back() << " hours\n\n";
    // Run for exactly 3000 plan cycles, tracking when outer cycles (price recomputations) occur
    int priceCyclesAchieved = 0;
    
    for (int cycle = 1; cycle <= numCycles; cycle++) {
        planCycle = cycle;
        bool priceUpdated = runCycle(planCycle);
        
        if (priceUpdated) {
            priceCyclesAchieved++;
            std::cout << "\n=== OUTER CYCLE " << priceCyclesAchieved << " ACHIEVED at plan cycle " << cycle << " ===\n\n";
        }
        
        // Show progress every 500 cycles
        if (cycle % 500 == 0) {
            std::cout << "Progress: " << cycle << "/" << numCycles << " plan cycles completed\n";
        }
    }
    
    showSummary();
    generateWorkDayPlot();
    
    std::cout << "\nSimulation completed successfully!\n";
    std::cout << "Total plan cycles: " << numCycles << "\n";
    std::cout << "Total inner cycles (innovations): " << innerCycle << "\n";
    std::cout << "Total outer cycles (price recomputations): " << priceCyclesAchieved << "\n";
}

// Economic shock methods removed

// Categorize products by labor intensity (healthcare = high, manufacturing = medium, etc.)
void Simulation::categorizeProductsByLaborIntensity() {
    // We're now focusing on individual products rather than sectors
    // This function kept for compatibility but simplified
    std::cout << "Products categorized by individual labor requirements\n";
}

// Generate plot of specific product price vs workday
void Simulation::generateProductWorkdayPlot(const std::string& productName) {
    if (priceHistory.find(productName) == priceHistory.end()) {
        std::cout << "Product " << productName << " not found in price history.\n";
        return;
    }
    
    using namespace matplot;
    
    std::vector<double> cycles;
    for (int cycle : cycleNumbers) {
        cycles.push_back(static_cast<double>(cycle));
    }
    
    auto fig = figure(true);
    fig->size(1000, 600);
    
    // Plot product price
    auto p1 = plot(cycles, priceHistory[productName], "b-o");
    p1->line_width(2);
    p1->marker_size(6);
    xlabel("Cycle");
    ylabel("Labor Hours");
    title(productName + " Labor Time vs Work Day");
    
    // Add work day as second line (scaled for visibility)
    hold(on);
    auto p2 = plot(cycles, workDayHours, "r-s");
    p2->line_width(2);
    p2->marker_size(6);
    
    legend({productName + " Labor Time", "Work Day Hours"});
    grid(on);
    save(productName + "_vs_workday.png");
    
    std::cout << "Generated plot: " << productName << " labor time vs work day\n";
}

// Generate plot of average price vs workday
void Simulation::generateAveragePriceWorkdayPlot() {
    using namespace matplot;
    
    // Calculate average price across all products for each cycle
    std::vector<double> averagePrices;
    for (size_t i = 0; i < cycleNumbers.size(); ++i) {
        double sum = 0.0;
        int count = 0;
        for (const auto& [productName, productData] : products) {
            if (i < priceHistory[productName].size()) {
                sum += priceHistory[productName][i];
                count++;
            }
        }
        averagePrices.push_back(count > 0 ? sum / count : 0.0);
    }
    
    std::vector<double> cycles;
    for (int cycle : cycleNumbers) {
        cycles.push_back(static_cast<double>(cycle));
    }
    
    auto fig = figure(true);
    fig->size(1000, 600);
    
    auto p1 = plot(cycles, averagePrices, "b-o");
    p1->line_width(3);
    p1->marker_size(8);
    xlabel("Cycle");
    ylabel("Labor Hours");
    title("Average Labor Time vs Work Day");
    
    // Add work day as second line
    hold(on);
    auto p2 = plot(cycles, workDayHours, "r-s");
    p2->line_width(3);
    p2->marker_size(8);
    
    legend({"Average Labor Time per Product", "Work Day Hours"});
    grid(on);
    save("graphs/average_labortime_vs_workday.png");
    
    std::cout << "Generated plot: Average labor time vs work day\n";
}



// Generate pie chart showing how much of a worker's budget goes to each product
void Simulation::generateWorkerBudgetPieChart(int cycle, const std::map<std::string, double>& customCosts) {
    using namespace matplot;
    
    // Calculate current costs for each product
    std::vector<double> productCosts;
    std::vector<std::string> productLabels;
    double totalCost = 0.0;
    
    // Use custom costs if provided (for cycle 0), otherwise use current prices
    if (!customCosts.empty()) {
        for (const auto& [productName, productData] : products) {
            double cost = customCosts.at(productName);
            productCosts.push_back(cost);
            totalCost += cost;
        }
    } else {
        // Use the PriceController's current prices (which are reduced by innovations)
        for (const auto& [productName, productData] : products) {
            double cost = priceController.getCurrentCost(productName);
            productCosts.push_back(cost);
            totalCost += cost;
        }
    }
    
    std::cout << "Current basket total: " << std::fixed << std::setprecision(3) << totalCost << " hours\n";
    
    // Convert to percentages of total worker budget
    std::vector<double> percentages;
    for (size_t i = 0; i < productCosts.size(); ++i) {
        double percentage = (productCosts[i] / totalCost) * 100.0;
        percentages.push_back(percentage);
        
        // Show both percentage and actual labor hours for each product (3 decimal places)
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << productCosts[i];
        
        // Get the product name from the products map
        auto it = products.begin();
        std::advance(it, i);
        std::string productName = it->first;
        
        productLabels.push_back(productName + " (" + std::to_string(int(percentage)) + "%, " + 
                               oss.str() + "h)");
    }
    
    auto fig = figure(true);
    fig->size(1000, 700);
    
    pie(percentages, productLabels);
    {
        std::ostringstream tss;
        tss << std::fixed << std::setprecision(3) << totalCost;
        title("Worker Budget Allocation - Outer Cycle " + std::to_string(cycle) + 
              "\nTotal Budget: " + tss.str() + " labor hours");
        // Move legend to the right side of the pie chart, not in the middle
        legend()->position({0.85, 0.5});
        legend()->title("Sum of basket costs: " + tss.str() + " labor hours");
    }
    
    std::string filename = "worker_budget_cycle_" + std::to_string(cycle) + ".png";
    save("graphs/" + filename);
    
    std::cout << "Generated worker budget pie chart for cycle " << cycle << ": " << filename << "\n";
    
    // Console output showing detailed budget breakdown
    std::cout << "=== WORKER BUDGET BREAKDOWN - CYCLE " << cycle << " ===\n";
    std::cout << "Total monthly budget: " << std::fixed << std::setprecision(1) << totalCost << " labor hours\n";
    
    // Show changes from initial prices
    double initialTotal = 0.0;
    for (const auto& [productName, productData] : products) {
        initialTotal += initialPrices[productName];
    }
    double savings = initialTotal - totalCost;
    double savingsPercent = (savings / initialTotal) * 100.0;
    
    std::cout << "Initial budget would have been: " << std::fixed << std::setprecision(1) << initialTotal << " hours\n";
    std::cout << "Total savings from productivity: " << std::fixed << std::setprecision(1) << savings 
              << " hours (" << std::setprecision(1) << savingsPercent << "%)\n\n";
    
    // Individual product breakdown
    std::cout << "Product breakdown:\n";
    for (size_t i = 0; i < products.size(); ++i) {
        // Get the product name from the products map
        auto it = products.begin();
        std::advance(it, i);
        std::string productName = it->first;
        
        double initialPrice = initialPrices[productName];
        double currentPrice = productCosts[i];
        double productSavings = initialPrice - currentPrice;
        double productSavingsPercent = (productSavings / initialPrice) * 100.0;
        
        std::cout << "  " << productName << ": " << std::fixed << std::setprecision(3) 
                  << currentPrice << "h (" << int(percentages[i]) << "% of budget)";
        
        if (productSavings > 0) {
            std::cout << " [SAVED " << std::setprecision(3) << productSavings << "h, " 
                      << std::fixed << std::setprecision(1) << productSavingsPercent << "%]";
        } else if (productSavings < 0) {
            std::cout << " [INCREASED " << std::fixed << std::setprecision(3) << -productSavings << "h]";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
} 

void Simulation::applyPlanCycleInnovations(int cycleNumber) {
    // Random innovations based on research data - more realistic and conservative parameters
    // Base probability per inner cycle: research-based rates
    const double LAMBDABASEPROBABIILITY = WORKER_PARTICIPATION_RATE * EMPLOYEE_SUGGESTION_RATE * SUGGESTION_IMPLEMENTATION_RATE;
    std::poisson_distribution<int> poissonDist(LAMBDABASEPROBABIILITY);
    const double meanReduction = 0.03; // 3% average productivity gain per innovation (very small)
    const double stdReduction = 0.015; // 1.5% standard deviation for small variance
    std::normal_distribution<double> reductionDist(meanReduction, stdReduction);

    bool anyInnovation = false;
    // numInnovations will be set inside the innovation block
    // Using Poisson distribution to generate the number of innovations

    std::vector<double> weights;
    weights.reserve(products.size());
    for (const auto& [productName, productData] : products) {
        auto it = productLaborIntensity.find(productName);
        double intensity = (it == productLaborIntensity.end() ? 0.5 : it->second);
        weights.push_back(std::max(0.01, 1.0 - intensity));
    }     
    std::discrete_distribution<int> weightedDist(weights.begin(), weights.end());   

    // Check if innovations should happen this cycle (8% chance per cycle - balanced frequency)
    std::uniform_real_distribution<double> innovationChance(0.0, 1.0);
    if (innovationChance(gen) <= 0.08) {
        // When innovation happens, ensure at least 1 product gets improved
        int numInnovations = std::max(1, poissonDist(gen));
        std::cout << "Debug: Innovation happening! " << numInnovations << " improvements" << std::endl;
        for (int i = 0; i < numInnovations; i++) {
            double innovation = std::clamp(reductionDist(gen), 0.002, 0.02); // Range: 0.2% to 2%
            // Get a random product from the products map
            auto it = products.begin();
            std::advance(it, weightedDist(gen) % products.size());
            const std::string &productName = it->first;
            products_innovation_rates[productName] += innovation;
            
            std::cout << "  -> " << productName << " improved by " << (innovation * 100) << "%" << std::endl;
            // Directly update the PriceController's current prices
            // priceController.updateCurrentPrice(productName, newCost);
            // Removed output for speed: Innovation in product by amount
        }
        anyInnovation = true;
    }
    
    // Only increment inner cycle when innovations actually happen
    if (anyInnovation) {
        innerCycle++;
        
        int num_firms_innovated = 0;
        std::cout << "Debug: Checking innovation rates:" << std::endl;
        for (auto& [product, innovation_rate] : products_innovation_rates) {
            std::cout << "  " << product << ": " << (innovation_rate * 100) << "%" << std::endl;
            if (innovation_rate > THRESHOLD_PERCENTAGE_PRODUCTS) {
                num_firms_innovated++;
                std::cout << "  -> " << product << " exceeds threshold!" << std::endl;
            }
        }
        std::cout << "Debug: " << num_firms_innovated << " products exceed threshold, need " << THRESHOLD_INNOVATION_AMONGST_PRODUCTS << std::endl;

        if (num_firms_innovated >= THRESHOLD_INNOVATION_AMONGST_PRODUCTS) {
            std::cout << "Debug: OUTER CYCLE TRIGGERED! Updating prices and resetting innovation rates" << std::endl;
            priceController.updateCurrentPrice();
            for (auto& product : products_innovation_rates) {
                product.second = 0;
            }
            outerCycle++;
            
            // Track the general cycle number when outer cycle increments
            workDayAtOuterCycle.push_back(cycleNumber);
            
            // Calculate and store the workday value at this outer cycle increment using PriceController
            double currentWorkday = 0.0;
            const auto& currentPrices = priceController.getCurrentPrices();
            for (const auto& [productName, price] : currentPrices) {
                currentWorkday += price;
            }
            workDayValuesAtOuterCycle.push_back(currentWorkday);
            
            std::cout << "Debug: Outer cycle " << outerCycle << " at cycle " << cycleNumber << ", workday: " << currentWorkday << " hours" << std::endl;
            
            generateWorkerBudgetPieChart(outerCycle);
        }

        // Calculate current workday for inner cycle tracking
        double currentWorkday = 0.0;
        const auto& currentPrices = priceController.getCurrentPrices();
        for (const auto& [productName, price] : currentPrices) {
            currentWorkday += price;
        }
        workDayAtInnerCycle.push_back(currentWorkday);
        // Removed output for speed: Workday after innovations
    } else {
        // Removed output for speed: No innovations this inner cycle
    }
}

void Simulation::generateWorkDayPlot() {
    using namespace matplot;
    auto fig = figure(true);
    fig->size(1000, 600);
    
    std::vector<double> x_tick;
    for(int i = 0; i <= 10000; i += 1000) {
        x_tick.push_back(static_cast<double>(i));
    }

    // Check if we have any data to plot
    if (workDayValuesAtOuterCycle.empty()) {
        std::cout << "Warning: No workday data to plot. Adding initial data point." << std::endl;
        // Add initial data point if none exists
        workDayAtOuterCycle.push_back(0);
        double initialWorkday = 0.0;
        for (const auto& [productName, productData] : products) {
            double initialPrice = std::get<1>(productData); // Get initial price from tuple
            initialWorkday += initialPrice;
        }
        workDayValuesAtOuterCycle.push_back(initialWorkday);
    }
    
    std::cout << "Debug: workDayAtOuterCycle size: " << workDayAtOuterCycle.size() << std::endl;
    std::cout << "Debug: workDayValuesAtOuterCycle size: " << workDayValuesAtOuterCycle.size() << std::endl;
    std::cout << "Debug: workDayAtInnerCycle size: " << workDayAtInnerCycle.size() << std::endl;

    // Plot workday as a step function - use general cycle numbers when outer cycles increment
    std::vector<double> generalCyclesD;
    std::vector<double> workDayValues;
    
    // Create step function data: each outer cycle increment creates a horizontal line until the next outer cycle
    for (size_t i = 0; i < workDayAtOuterCycle.size(); ++i) {
        generalCyclesD.push_back(static_cast<double>(workDayAtOuterCycle[i]));
        workDayValues.push_back(workDayValuesAtOuterCycle[i]); // Use the correct array
        
        // If this isn't the last point, add the next outer cycle with the same value to create horizontal steps
        if (i < workDayAtOuterCycle.size() - 1) {
            generalCyclesD.push_back(static_cast<double>(workDayAtOuterCycle[i + 1]));
            workDayValues.push_back(workDayValuesAtOuterCycle[i]); // Use the correct array
        }
    }

    // Add final point to extend the last step to the end
    if (!workDayAtOuterCycle.empty()) {
        generalCyclesD.push_back(10000.0);
        workDayValues.push_back(workDayValuesAtOuterCycle.back());
    }

    std::cout << "Plotting " << generalCyclesD.size() << " data points:" << std::endl;
    for (size_t i = 0; i < generalCyclesD.size(); ++i) {
        std::cout << "Cycle " << generalCyclesD[i] << ": " << workDayValues[i] << " hours" << std::endl;
    }

    auto p = plot(generalCyclesD, workDayValues);
    matplot::xticks(x_tick);
    p->line_width(3);
    p->color("blue");
    xlabel("General Simulation Cycle");
    ylabel("Hours in Work Day");
    title("Work Day Over Time - Outer Cycle Increments (Step Function)");
    grid(on);
    
    // Set reasonable axis limits
    ylim({4.0, 8.5}); // Workday should stay between 4-8.5 hours
    xlim({0.0, 10000.0}); // Force x-axis to go from 0 to 10000
    
    save("graphs/workday_over_time.png");
    
    std::cout << "Generated workday plot with " << generalCyclesD.size() << " data points" << std::endl;
} 



// SimulationB method implementations

SimulationB::SimulationB() : Simulation() {
    // Generate initial pie chart for SimulationB in graphs2 directory
    std::map<std::string, double> cycle0Costs;
    for (const auto& [firmName, price] : firmPrices) {
        cycle0Costs[firmName] = price;
    }
    
    // Generate cycle 0 pie chart in graphs2 directory
    generateWorkerBudgetPieChart(0, cycle0Costs);
    
    // Add initial workday tracking for cycle 0
    workDayAtInnerCycleGeneral.push_back(0);
    
    // Removed output for speed: SimulationB constructor completed
}

void SimulationB::applyPlanCycleInnovations(int cycleNumber) {
    // Random innovations based on research data - more realistic and conservative parameters
    // Base probability per inner cycle: research-based rates
    const double LAMBDABASEPROBABIILITY = WORKER_PARTICIPATION_RATE * EMPLOYEE_SUGGESTION_RATE * SUGGESTION_IMPLEMENTATION_RATE;
    std::poisson_distribution<int> poissonDist(LAMBDABASEPROBABIILITY);
    const double meanReduction = 0.005; // 0.5% average productivity gain per innovation (very very very low)
    const double stdReduction = 0.002; // 0.2% standard deviation (very very very low)
    std::normal_distribution<double> reductionDist(meanReduction, stdReduction);

    bool anyInnovation = false;
    // numInnovations will be set inside the innovation block
    // Using Poisson distribution to generate the number of innovations

    std::vector<double> weights;
    weights.reserve(products.size());
    for (const auto& [productName, productData] : products) {
        auto it = productLaborIntensity.find(productName);
        double intensity = (it == productLaborIntensity.end() ? 0.5 : it->second);
        weights.push_back(std::max(0.01, 1.0 - intensity));
    }     
    std::discrete_distribution<int> weightedDist(weights.begin(), weights.end());   

    // Check if innovations should happen this cycle (8% chance per cycle - balanced frequency, same as Simulation A)
    std::uniform_real_distribution<double> innovationChance(0.0, 1.0);
    if (innovationChance(gen) <= 0.08) {
        // When innovation happens, ensure at least 1 product gets improved
        int numInnovations = std::max(1, poissonDist(gen));
        for (int i = 0; i < numInnovations; i++) {
            double innovation = std::clamp(reductionDist(gen), 0.0001, 0.001); // Range: 0.01% to 0.1%, very very very low
            // Get a random product from the products map
            auto it = products.begin();
            std::advance(it, weightedDist(gen) % products.size());
            const std::string &productName = it->first;
            products_innovation_rates[productName] += innovation;
            priceController.updateCurrentPriceForSimulationB(productName, priceController.getCurrentCost(productName) * (1.0 - innovation));
            // Directly update the PriceController's current prices
            // priceController.updateCurrentPrice(product, newCost);
            // Removed output for speed: Innovation in product by amount
        }
        anyInnovation = true;
    }
    
    // Only increment inner cycle when innovations actually happen
    if (anyInnovation) {
        innerCycle++;
        double currentWorkday = 0.0;
        
        // Track the general cycle number when inner cycle increments
        workDayAtInnerCycleGeneral.push_back(cycleNumber);
            
        // Calculate current costs after innovations
        std::map<std::string, double> currentCosts;
        for (const auto& [firmName, price] : firmPrices) {
            currentCosts[firmName] = price;
        }
            
        generateWorkerBudgetPieChart(innerCycle, currentCosts);

        for (const auto& [firmName, price] : firmPrices) {
            currentWorkday += price;
        }
        workDayAtInnerCycle.push_back(currentWorkday);
        // Removed output for speed: Workday after innovations
    } else {
        // Removed output for speed: No innovations this inner cycle
    }    


}

void SimulationB::generateWorkerBudgetPieChart(int cycle, const std::map<std::string, double>& customCosts) {
    // Don't call base class method - only generate charts in graphs2 directory
    
    // Create the pie chart for SimulationB
    using namespace matplot;
    
    // Calculate current costs for each product (same logic as base class)
    std::vector<double> productCosts;
    std::vector<std::string> productLabels;
    double totalCost = 0.0;
    
    // Use custom costs if provided (for cycle 0), otherwise use current prices
    if (!customCosts.empty()) {
        for (const auto& [productName, productData] : products) {
            // Safely access customCosts with a fallback to current price
            auto it = customCosts.find(productName);
            double cost;
            if (it != customCosts.end()) {
                cost = it->second;
            } else {
                cost = priceController.getCurrentCost(productName);
            }
            productCosts.push_back(cost);
            totalCost += cost;
        }
    } else {
        for (const auto& [productName, productData] : products) {
            double cost = priceController.getCurrentCost(productName);
            productCosts.push_back(cost);
            totalCost += cost;
        }
    }
    
    // Convert to percentages
    std::vector<double> percentages;
    for (size_t i = 0; i < productCosts.size(); ++i) {
        double percentage = (productCosts[i] / totalCost) * 100.0;
        percentages.push_back(percentage);
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << productCosts[i];
        
        // Get the product name from the products map
        auto it = products.begin();
        std::advance(it, i);
        std::string productName = it->first;
        
        productLabels.push_back(productName + " (" + std::to_string(int(percentage)) + "%, " + 
                               oss.str() + "h)");
    }
    
    auto fig = figure(true);
    fig->size(1000, 700);
    
    pie(percentages, productLabels);
    std::ostringstream tss;
    tss << std::fixed << std::setprecision(3) << totalCost;
    title("SimulationB - Worker Budget Allocation - Outer Cycle " + std::to_string(cycle) + 
          "\nTotal Budget: " + tss.str() + " labor hours");
    // Move legend to the right side of the pie chart, not in the middle
    legend()->position({0.85, 0.5});
    legend()->title("Sum of basket costs: " + tss.str() + " labor hours");
    
    save("graphs2/worker_budget_cycle_" + std::to_string(cycle) + "_B.png");
    // Removed output for speed: Generated pie chart
}

void SimulationB::generateWorkDayPlot() {
    // Don't call base class method - only generate charts in graphs2 directory
    
    std::string newFilename = "graphs2/workday_over_time_B.png";
    
    // Create the workday plot for SimulationB
    using namespace matplot;
    auto fig = figure(true);
    fig->size(1000, 600);
    
    // Create x-axis ticks that go up to 10000
    std::vector<double> x_tick;
    for(int i = 0; i <= 10000; i += 1000) {
        x_tick.push_back(static_cast<double>(i));
    }

    // Plot workday as a step function - use general cycle numbers when inner cycles increment
    std::vector<double> generalCyclesD;
    std::vector<double> workDayValues;
    
    // Create step function data: each inner cycle increment creates a horizontal line until the next inner cycle
    if (workDayAtInnerCycleGeneral.empty()) {
        // No inner cycles yet, just show initial workday at 8.0 for full range
        generalCyclesD.push_back(0.0);
        generalCyclesD.push_back(10000.0);
        workDayValues.push_back(8.0);
        workDayValues.push_back(8.0);
    } else {
        for (size_t i = 0; i < workDayAtInnerCycleGeneral.size(); ++i) {
            // Add the inner cycle point
            generalCyclesD.push_back(static_cast<double>(workDayAtInnerCycleGeneral[i]));
            
            // Get the corresponding workday value (ensure we don't go out of bounds)
            double workdayValue = 8.0; // Default to initial workday
            if (i < workDayAtInnerCycle.size()) {
                workdayValue = workDayAtInnerCycle[i];
            }
            workDayValues.push_back(workdayValue);
            
            // If this isn't the last point, add the next inner cycle with the same value to create horizontal steps
            if (i < workDayAtInnerCycleGeneral.size() - 1) {
                generalCyclesD.push_back(static_cast<double>(workDayAtInnerCycleGeneral[i + 1]));
                workDayValues.push_back(workdayValue);
            } else {
                // For the last inner cycle, extend the line to the end of simulation
                generalCyclesD.push_back(10000.0);
                workDayValues.push_back(workdayValue);
            }
        }
    }

    auto p = plot(generalCyclesD, workDayValues);
    xticks(x_tick);
    p->line_width(3);
    p->color("red");  // Different color to distinguish from base simulation
    xlabel("General Simulation Cycle");
    ylabel("Hours in Work Day");
    title("SimulationB - Work Day Over Time - Inner Cycle Increments (Step Function)");
    grid(on);
    
    // Set x-axis limits to go from 0 to 10000
    xlim({0.0, 10000.0});
    ylim({4.0, 8.5});
    
    save(newFilename);
    // Removed output for speed: Generated workday plot
}

void Simulation::fixWorkDayPlotXAxis() {
    using namespace matplot;
    
    // Load the existing plot data
    auto fig = figure(true);
    fig->size(1000, 600);
    
    // Create x-axis ticks that go up to 10000
    std::vector<double> x_tick;
    for(int i = 0; i <= 10000; i += 1000) {
        x_tick.push_back(static_cast<double>(i));
    }

    // Plot workday as a step function
    std::vector<double> outerCyclesD;
    std::vector<double> workDayValues;
    
    for (size_t i = 0; i < workDayAtInnerCycle.size(); ++i) {
        outerCyclesD.push_back(static_cast<double>(i));
        workDayValues.push_back(workDayAtInnerCycle[i]);
        
        if (i < workDayAtInnerCycle.size() - 1) {
            outerCyclesD.push_back(static_cast<double>(i + 1));
            workDayValues.push_back(workDayAtInnerCycle[i]);
        }
    }

    auto p = plot(outerCyclesD, workDayValues);
    xticks(x_tick);
    p->line_width(3);
    p->color("blue");
    xlabel("Inner Plan Cycle");
    ylabel("Hours in Work Day");
    title("Work Day Over Time - Inner Plan Cycles (Step Function)");
    grid(on);
    
    // Force x-axis to go from 0 to 10000
    xlim({0.0, 10000.0});
    ylim({4.0, 8.5});
    
    save("graphs/workday_over_time_fixed.png");
    std::cout << "Generated fixed workday plot with x-axis 0-3000: graphs/workday_over_time_fixed.png" << std::endl;
}

void Simulation::saveSummaryStatistics() {
    std::ofstream summaryFile("simulation_summary.csv");
    summaryFile << "SimulationType,TotalFirms,TotalCycles,TotalInnerCycles,TotalOuterCycles,FinalWorkDay,InitialWorkDay,WorkDayReduction\n";
    
    summaryFile << "SimulationA," << firms.size() << "," << cycleNumbers.back() << "," << innerCycle << "," << outerCycle << ",";
    if (!workDayAtInnerCycle.empty()) {
        summaryFile << std::fixed << std::setprecision(3) << workDayAtInnerCycle.back() << ",8.000,";
        summaryFile << std::fixed << std::setprecision(3) << (8.0 - workDayAtInnerCycle.back()) << "\n";
    } else {
        summaryFile << "8.000,8.000,0.000\n";
    }
    
    summaryFile.close();
    // Summary statistics saved to simulation_summary.csv
}