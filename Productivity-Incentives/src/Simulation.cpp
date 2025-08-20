#include "../include/Simulation.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <matplot/matplot.h>
#include <sstream>

Simulation::Simulation() : gen(std::random_device{}()), probability(0.0, 1.0), nextProjectId(1), nextFirmId(1), planCycle(0), innerCycle(0),
    // Initial official prices (hours) sum to 8.0
    priceController({{"shirts", 0.6}, {"shoes", 0.9}, {"shorts", 0.5}, {"apples", 0.8}, {"bread", 1.0}, {"chairs", 1.2}, {"tables", 1.3}, {"healthcare", 1.7}}, 
                    {{"shirts", "units"}, {"shoes", "pairs"}, {"shorts", "units"}, {"apples", "kg"}, {"bread", "loaves"}, {"chairs", "units"}, {"tables", "units"}, {"healthcare", "hours"}}) {
    saveInitialPrices();
    categorizeProductsByLaborIntensity();
    createFirms();

    // currentProductCosts removed - using PriceController's current_prices instead

    // Initialize labor intensity bias (research-based values)
    // Higher values = more labor-intensive = harder to innovate/automate
    for (const auto& product : products) {
        double intensity = 0.5; // default medium
        
        // Healthcare: Highly labor-intensive (personal care, skilled professionals)
        if (product == "healthcare") {
            intensity = 0.85; // Very high - difficult to automate personal care
        }
        
        // Food/Agriculture: High labor-intensity (delicate handling, perishable)
        else if (product == "apples" || product == "bread") {
            intensity = 0.65; // High - fruit picking, baking require manual work
        }
        
        // Textiles/Apparel: Medium-high labor-intensity (cutting, sewing)
        else if (product == "shirts" || product == "shorts") {
            intensity = 0.55; // Medium-high - sewing still requires significant manual work
        }
        
        // Furniture: Medium labor-intensity (some automation possible)
        else if (product == "chairs" || product == "tables") {
            intensity = 0.45; // Medium - woodworking can be partially automated
        }
        
        // Footwear: Medium labor-intensity (similar to textiles but more automated)
        else if (product == "shoes") {
            intensity = 0.50; // Medium - shoe manufacturing has some automation
        }
        
        productLaborIntensity[product] = intensity;
        std::cout << "Labor intensity for " << product << ": " << intensity << std::endl;
    }
    
    // Initialize workday tracking with initial value
    double initialWorkday = 0.0;
    for (const auto& product : products) {
        initialWorkday += priceController.getCurrentCost(product);
    }
    workDayAtInnerCycle.push_back(initialWorkday);
    
    // Generate initial pie chart showing baseline prices
    generateWorkerBudgetPieChart(0);
}

void Simulation::saveInitialPrices() {
    for (const auto& product : products) {
        initialPrices[product] = priceController.getOfficialPrice(product);
        priceHistory[product] = std::vector<double>();
    }
}

void Simulation::trackPricesAndWorkDay(int cycle) {
    cycleNumbers.push_back(cycle);
    
    // Track current prices for each product
    for (const auto& product : products) {
        priceHistory[product].push_back(priceController.getCurrentCost(product));
    }
    
    // Track work day hours as sum of basket costs (stepwise)
    double sumBasket = 0.0;
    for (const auto& product : products) sumBasket += priceController.getCurrentCost(product);
    workDayHours.push_back(sumBasket);
    
    // Only generate pie chart when we have a price cycle (not every inner cycle)
    // This will be called separately when price cycles advance
}

void Simulation::createFirms() {
    // Create NUM_FIRMS firms
    for (int i = 0; i < NUM_FIRMS; ++i) {
        auto firm = std::make_shared<Firm>(nextFirmId++, firmNames[i], priceController);
        
        // Add some workers to each firm
        for (int j = 0; j < 3; ++j) {
            auto worker = std::make_shared<Worker>(j + 1, *firm);
            firm->addWorker(worker);
        }
        
        firms.push_back(firm);
    }
    
    firmSelector = std::uniform_int_distribution<>(0, firms.size() - 1);
    std::cout << "Created " << firms.size() << " firms with workers\n\n";
}

bool Simulation::priceControllerPhase() {
    std::cout << "=== Phase 5: Price Controller Update ===\n";
    
    // Collect all completed projects
    std::vector<Project> completedProjects;
    for (const auto& firm : firms) {
        const auto& projects = firm->getProjects();
        for (const auto& project : projects) {
            if (project.actualCost > 0) {
                completedProjects.push_back(project);
            }
        }
    }
    
    std::cout << "Analyzing " << completedProjects.size() << " completed projects...\n";
    
    // Update current costs based on actual production data
    priceController.updateCurrentCosts(completedProjects);
    
    // currentProductCosts removed - using PriceController's current_prices directly
    
    // Show current vs official prices
    std::cout << "\nCurrent costs vs Official prices:\n";
    for (const auto& product : products) {
        double official = priceController.getOfficialPrice(product);
        double current = priceController.getCurrentCost(product);
        double savings = official - current;
        double percentage = (savings / official) * 100.0;
        
        std::cout << product << ": Official=" << official 
                 << ", Current=" << current 
                 << " (savings: " << savings << " hours, " << percentage << "%)\n";
    }
    
    // Update official prices if conditions are met
    std::cout << "\nChecking for price updates...\n";
    double beforeSum = 0.0; for (const auto& p : products) beforeSum += priceController.getOfficialPrice(p);
    priceController.updateOfficialPrices(threshold_percentage_firms, threshold_percentage_products);
    double afterSum = 0.0; for (const auto& p : products) afterSum += priceController.getOfficialPrice(p);
    if (afterSum < beforeSum) {
        // Price cycle increment - this is when we generate pie charts
        int nextPriceCycle = priceCycleNumbers.empty() ? 1 : (priceCycleNumbers.back() + 1);
        priceCycleNumbers.push_back(nextPriceCycle);
        double workday = 0.0; for (const auto& p : products) workday += priceController.getCurrentCost(p);
        workDayAtPriceCycle.push_back(workday);
        
        std::cout << "\n*** PRICE CYCLE " << nextPriceCycle << " ***\n";
        std::cout << "Official prices updated! New workday: " << workday << " hours\n";
        
        // Generate pie chart for this price cycle
        generateWorkerBudgetPieChart(nextPriceCycle);
        
        return true; // Signal that prices were updated
    }
    return false; // No price update
}

bool Simulation::runCycle(int cycleNumber) {
    std::cout << "==================== CYCLE " << cycleNumber << " ====================\n";
    
    // Plan-cycle: apply stochastic innovations to basket costs
    applyPlanCycleInnovations();

    // Phase 1: Reset firm labor time tracking for this cycle
    for (auto& firm : firms) {
        firm->resetCycleLaborTime();
    }
    
    // Phase 2: Each firm creates projects
    std::cout << "=== Phase 2: Project Creation ===\n";
    for (auto& firm : firms) {
        firm->create_projects();
    }
    std::cout << "\n";
    
    // Phase 3: Each firm executes projects (includes learning, innovation, production)
    std::cout << "=== Phase 3: Project Execution ===\n";
    for (auto& firm : firms) {
        firm->execute_projects(firms);
    }
    std::cout << "\n";
    
    // Phase 4: Record labor time for this cycle
    std::cout << "=== Phase 4: Labor Time Recording ===\n";
    for (auto& firm : firms) {
        double cycleLaborTime = firm->getTotalLaborTimeSpent() - 
            (firm->getLaborTimeHistory().size() > 0 ? 
             std::accumulate(firm->getLaborTimeHistory().begin(), firm->getLaborTimeHistory().end(), 0.0) : 0.0);
        firm->recordLaborTimeForCycle(cycleLaborTime);
        
        std::cout << firm->getName() << " labor time this cycle: " << cycleLaborTime << " hours\n";
    }
    std::cout << "\n";
    
    // Phase 5: Price Controller updates
    bool priceUpdated = priceControllerPhase();
    trackPricesAndWorkDay(cycleNumber);
    
    std::cout << "Cycle " << cycleNumber << " completed.\n\n";
    
    // Brief pause for readability
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    return priceUpdated;
}

void Simulation::showSummary() {
    std::cout << "=== SIMULATION SUMMARY ===\n";
    std::cout << "Total firms: " << firms.size() << "\n";
    std::cout << "Total projects created: " << nextProjectId - 1 << "\n";
    
    std::cout << "\nInitial official prices:\n";
    for (const auto& product : products) {
        std::cout << product << ": " << initialPrices[product] << " hours\n";
    }
    
    std::cout << "\nFinal official prices:\n";
    for (const auto& product : products) {
        double initial = initialPrices[product];
        double final = priceController.getOfficialPrice(product);
        double delta = final - initial;
        double percentChange = (initial > 0) ? (delta / initial) * 100.0 : 0.0;
        
        std::cout << product << ": " << final << " hours (delta: " << std::showpos 
                 << delta << std::noshowpos << " hours, " << std::showpos 
                 << percentChange << std::noshowpos << "%)\n";
    }
    
    std::cout << "\nFinal current costs (actual productivity):\n";
    for (const auto& product : products) {
        double initial = initialPrices[product];
        double current = priceController.getCurrentCost(product);
        double delta = current - initial;
        double percentChange = (initial > 0) ? (delta / initial) * 100.0 : 0.0;
        
        std::cout << product << ": " << current << " hours (delta: " << std::showpos 
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
        for (const auto& product : products) {
            double productTime = firm->getLaborTimeForProduct(product);
            if (productTime > 0) {
                std::cout << product << ":" << std::fixed << std::setprecision(1) << productTime << "h ";
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
        for (const auto& product : products) baseline += priceController.getCurrentCost(product);
        workDayHours.push_back(baseline);
    }
    priceCycleNumbers.clear();
    workDayAtPriceCycle.clear();
    priceCycleNumbers.push_back(0);
    workDayAtPriceCycle.push_back(workDayHours.back());
    
    // Track initial prices for cycle 0
    for (const auto& product : products) {
        priceHistory[product].push_back(initialPrices[product]);
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
        for (const auto& product : products) {
            if (i < priceHistory[product].size()) {
                sum += priceHistory[product][i];
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
void Simulation::generateWorkerBudgetPieChart(int cycle) {
    using namespace matplot;
    
    // Calculate current costs for each product
    std::vector<double> productCosts;
    std::vector<std::string> productLabels;
    double totalCost = 0.0;
    
    // Use the PriceController's current prices (which are reduced by innovations)
    for (const auto& product : products) {
        double cost = priceController.getCurrentCost(product);
        productCosts.push_back(cost);
        totalCost += cost;
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
        productLabels.push_back(products[i] + " (" + std::to_string(int(percentage)) + "%, " + 
                               oss.str() + "h)");
    }
    
    auto fig = figure(true);
    fig->size(1000, 700);
    
    pie(percentages, productLabels);
    {
        std::ostringstream tss;
        tss << std::fixed << std::setprecision(3) << totalCost;
        title("Worker Budget Allocation - Cycle " + std::to_string(cycle) + 
              "\nTotal Budget: " + tss.str() + " labor hours");
    }
    
    std::string filename = "worker_budget_cycle_" + std::to_string(cycle) + ".png";
    save(filename);
    
    std::cout << "Generated worker budget pie chart for cycle " << cycle << ": " << filename << "\n";
    
    // Console output showing detailed budget breakdown
    std::cout << "=== WORKER BUDGET BREAKDOWN - CYCLE " << cycle << " ===\n";
    std::cout << "Total monthly budget: " << std::fixed << std::setprecision(1) << totalCost << " labor hours\n";
    
    // Show changes from initial prices
    double initialTotal = 0.0;
    for (const auto& product : products) {
        initialTotal += initialPrices[product];
    }
    double savings = initialTotal - totalCost;
    double savingsPercent = (savings / initialTotal) * 100.0;
    
    std::cout << "Initial budget would have been: " << std::fixed << std::setprecision(1) << initialTotal << " hours\n";
    std::cout << "Total savings from productivity: " << std::fixed << std::setprecision(1) << savings 
              << " hours (" << std::setprecision(1) << savingsPercent << "%)\n\n";
    
    // Individual product breakdown
    std::cout << "Product breakdown:\n";
    for (size_t i = 0; i < products.size(); ++i) {
        double initialPrice = initialPrices[products[i]];
        double currentPrice = productCosts[i];
        double productSavings = initialPrice - currentPrice;
        double productSavingsPercent = (productSavings / initialPrice) * 100.0;
        
        std::cout << "  " << products[i] << ": " << std::fixed << std::setprecision(3) 
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

void Simulation::applyPlanCycleInnovations() {
    // Random innovations based on research data - only some products get innovations randomly
    // Base probability per inner cycle: research-based rates
    const double LAMBDABASEPROBABIILITY = WORKER_PARTICIPATION_RATE * EMPLOYEE_SUGGESTION_RATE * SUGGESTION_IMPLEMENTATION_RATE;
    std::poisson_distribution<int> poissonDist(LAMBDABASEPROBABIILITY);
    const double meanReduction = 0.25; // 25% average productivity gain per innovation (much more impactful)
    const double stdReduction = 0.08; // moderate variance for realistic variation
    std::normal_distribution<double> reductionDist(meanReduction, stdReduction);

    bool anyInnovation = false;
    int numInnovations = poissonDist(gen);
    // Using Poisson distribution to generate the number of innovations

    std::vector<double> weights;
    weights.reserve(products.size());
    for (const auto& product : products) {
        auto it = productLaborIntensity.find(product);
        double intensity = (it == productLaborIntensity.end() ? 0.5 : it->second);
        weights.push_back(std::max(0.01, 1.0 - intensity));
    }     
    std::discrete_distribution<int> weightedDist(weights.begin(), weights.end());   

    // Check if innovations should happen this cycle (3% chance - less frequent but more impactful)
    std::uniform_real_distribution<double> innovationChance(0.0, 1.0);
    if (innovationChance(gen) <= 0.03) {
        for (int i = 0; i < numInnovations; i++) {
            double reduction = std::clamp(reductionDist(gen), 0.10, 0.40);
            const std::string &product = products[weightedDist(gen)];
            double oldCost = priceController.getCurrentCost(product);
            double newCost = oldCost * (1 - reduction);
            // Directly update the PriceController's current prices
            priceController.updateCurrentPrice(product, newCost);
            std::cout << "Innovation in " << product << ": " << std::fixed << std::setprecision(3) 
                                             << oldCost << "h -> " << newCost << "h (" 
                       << (reduction * 100.0) << "% reduction)\n";
        }
        anyInnovation = true;
    }
    
    // Only increment inner cycle when innovations actually happen
    if (anyInnovation) {
        innerCycle++;
        double currentWorkday = 0.0;
        for (const auto& product : products) {
            currentWorkday += priceController.getCurrentCost(product);
        }
        workDayAtInnerCycle.push_back(currentWorkday);
        std::cout << "Workday after innovations: " << std::fixed << std::setprecision(3) << currentWorkday << " hours\n";
    } else {
        std::cout << "No innovations this inner cycle.\n";
    }
}

void Simulation::generateWorkDayPlot() {
    using namespace matplot;
    auto fig = figure(true);
    fig->size(1000, 600);

    // Plot workday as a true step function - only when innovations occur
    std::vector<double> outerCyclesD;
    std::vector<double> workDayValues;
    
    // Create step function data: each innovation creates a horizontal line until the next innovation
    for (size_t i = 0; i < workDayAtInnerCycle.size(); i++) {
        outerCyclesD.push_back(static_cast<double>(i));
        workDayValues.push_back(workDayAtInnerCycle[i]);
        
        // If this isn't the last point, add the next cycle with the same value to create horizontal steps
        if (i < workDayAtInnerCycle.size() - 1) {
            outerCyclesD.push_back(static_cast<double>(i + 1));
            workDayValues.push_back(workDayAtInnerCycle[i]);
        }
    }
    
    auto p = plot(outerCyclesD, workDayValues);
    p->line_width(3);
    p->color("blue");
    xlabel("Outer Plan Cycle");
    ylabel("Hours in Work Day");
    title("Work Day Over Time - Inner Plan Cycles (Step Function)");
    grid(on);
    
    // Set reasonable axis limits
    ylim({4.0, 8.5}); // Workday should stay between 4-8.5 hours
    
    save("graphs/workday_over_time.png");
    
    std::cout << "Generated workday plot: workday_over_time.png\n";
    std::cout << "Inner plan cycles completed: " << innerCycle << "\n";
} 


