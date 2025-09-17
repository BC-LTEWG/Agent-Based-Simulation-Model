# Supply-Demand Signals Codebase Manual
**Complete Line-by-Line Analysis and Rewriting Guide**

## Project Overview
This is a C++ economic simulation system that models supply and demand dynamics using cybernetic signals and automatic responses. The system simulates a society with workers, firms, goods, and a distributor, running through plan cycles to balance production and consumption.

## Build System
- **CMakeLists.txt**: Uses CMake 3.14+, C++20 standard, depends on Matplot++ for visualization
- **External Dependencies**: 
  - Matplot++ (plotting library)
  - robin_hood hash maps (included in extern/)
- **Build files**: `build.sh`, `clean.sh`, `test.sh`, `profile.sh`

## Core Architecture

### Main Entry Point: `src/main.cpp`

#### Lines 1-13: Includes and Setup
```cpp
#include <cassert>
#include <chrono>
#include <cstdio>
#include <vector>

#include "Distributor.hpp"
#include "Firm.hpp"
#include "Plan.hpp"
#include "PlotHandler.hpp"
#include "Society.hpp"
#include "Worker.hpp"
#include "randomizer.hpp"
```
**Purpose**: Standard includes plus all custom headers for the economic simulation.

#### Lines 14-22: Argument Parser Lambda
```cpp
int main(int argc, char * argv[]) {
    auto arg_present = [&argc, &argv](const char * arg) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], arg) == 0) {
                return true;
            }
        }
        return false;
    };
```
**Purpose**: Simple command-line argument checker for `--save` and `--show` flags.

#### Lines 24-26: Society Creation
```cpp
Society * society =
    new Society(Config{.plan_cycle_duration = 90, .fic = .8, .workday_length = 8.0});
```
**Key Variables**:
- `plan_cycle_duration = 90`: Each plan cycle lasts 90 days (quarterly)
- `fic = 0.8`: Fraction of Income Consumed (workers consume 80%, 20% goes to society reserve)
- `workday_length = 8.0`: 8-hour work days

#### Lines 28-38: Good Creation
```cpp
Good * means = new Good("Flour", 5e3, {}, 1);
Good * good = new Good("Bread", 5e3, {{means, 1}}, 1);
society->add_good(means);
society->add_good(good);
```
**Economic Model**:
- `means` = "Flour" (raw material), target surplus 5000, no inputs required, 1 hour labor
- `good` = "Bread" (final product), target surplus 5000, requires 1 flour per bread, 1 hour labor
- Creates a simple production chain: Flour → Bread

#### Lines 40-49: Firm Creation
```cpp
Firm * means_firm = new Firm(society);
Firm * good_firm = new Firm(society);
society->add_firm(means_firm);
society->add_firm(good_firm);

Distributor * distributor = new Distributor(society);
society->distributors.push_back(distributor);
society->add_firm(distributor);
```
**Structure**:
- `means_firm`: Produces flour
- `good_firm`: Produces bread
- `distributor`: Handles distribution and purchasing (inherits from Firm)

#### Lines 51-65: Project Planning
```cpp
const int population_size = 100;

auto good_project = good_firm->add_project(new Project(society,
    Plan::from(good, society->config.plan_cycle_duration * population_size + good->target_surplus)));

means_firm->add_project(new Project(society,
    Plan::from(means, good_project->plan.quantity * good->means[means] + means->target_surplus)));
```
**Production Planning Logic**:
- Population needs: 90 days × 100 people = 9000 bread per cycle
- Total bread needed: 9000 + 5000 surplus = 14000 bread
- Flour needed: 14000 bread × 1 flour/bread + 5000 surplus = 19000 flour
- **Critical**: This establishes the dependency chain for production planning

#### Lines 66-97: Worker Creation
```cpp
for (int i = 0; i < population_size; i++) {
    Worker * worker = new Worker(distributor,
        {
            randomizer::need_cycle({good}, 1.0, 
                {randomizer::DistributionType::UNIFORM, 1, 0},
                {randomizer::DistributionType::NORMAL, 1, 0}),
        },
        1e4);
    society->add_worker(worker);
    society->distribute_worker(worker, i, population_size);
}
```
**Worker Configuration**:
- 100 workers total
- Each worker needs bread (priority 1.0)
- Uniform distribution for cycle timing (1±0 = exactly 1 day)
- Normal distribution for amount (1±0 = exactly 1 bread)
- Initial credits: 10,000
- Workers are distributed among firms based on ideal job ratios

### Graph Creation and Visualization: Lines 99-121

#### Lines 99-106: Main Plot Setup
```cpp
auto plot_handler = PlotHandler(1200, 900);
auto main_plot = plot_handler.create_plot("main",
    "Economic statistics over time",
    "Plan cycle count",
    "Quantity");
main_plot->define_line("Median Worker Wealth", {0.8f, 0.1f, 0.8f});  // Magenta
main_plot->define_line("Society Reserve", {0.2f, 0.2f, 0.2f});       // Dark Gray
```
**Graph Setup**:
- Creates 1200×900 pixel plotting area
- Main plot tracks overall economic health
- Two lines: worker wealth (magenta) and society reserve (dark gray)

#### Lines 107-121: Per-Good Plots
```cpp
robin_hood::unordered_map<Good *, Plot *> good_plots;
for (auto & good : society->goods) {
    good_plots[good] = plot_handler.create_plot(good->name,
        good->name + " over time",
        "Plan cycle count",
        "Quantity");

    good_plots[good]->define_line("Inventory", {0.2f, 0.6f, 0.9f});              // Blue
    good_plots[good]->define_line("Production Deficit", {0.9f, 0.2f, 0.2f});     // Red
    good_plots[good]->define_line("Planned Production", {0.2f, 0.9f, 0.2f});     // Green
    good_plots[good]->define_line("Actual Production", {0.9f, 0.7f, 0.2f});      // Orange
    good_plots[good]->define_line("Ideal Workers", {0.6f, 0.2f, 0.9f})->hide();  // Purple
}
```
**Per-Good Tracking**:
- Creates separate plot for each good (Flour, Bread)
- Five metrics per good:
  - Inventory (blue): Current stock levels
  - Production Deficit (red): Unmet demand
  - Planned Production (green): Target production
  - Actual Production (orange): Realized production
  - Ideal Workers (purple, hidden): Optimal workforce size

### Main Simulation Loop: Lines 124-180

#### Lines 124-136: Loop Setup and Timing
```cpp
std::vector<double> tick_times;
for (int i = 0; i < 350; i++) {
    printf("\n ------- Tick cycle %d -------\n", i + 1);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    society->tick_cycle();
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    tick_times.push_back(duration.count() / 1000.0);
```
**Loop Structure**:
- Runs 350 plan cycles (350 × 90 days = 31,500 simulated days)
- Performance monitoring: tracks execution time per cycle
- Each cycle calls `society->tick_cycle()` which runs 90 daily ticks

#### Lines 137-150: Data Collection
```cpp
distributor->head();

main_plot->add_x(i);

for (auto & [good, plot] : good_plots) {
    plot->add_x(i);
    plot->add_y("Inventory", distributor->total_inventory(good));
    plot->add_y("Production Deficit", distributor->get_production_deficit(good, i));

    auto stats = society->aggregate_good_stats(good);

    plot->add_y("Planned Production", stats.planned_quantity);
    plot->add_y("Actual Production", stats.produced);
    plot->add_y("Ideal Workers", stats.ideal_workers * 1000);
}
```
**Data Points Collected**:
- X-axis: cycle number (i)
- Per-good metrics from distributor and aggregated statistics
- Worker count scaled by 1000 for visibility
- **Key Methods**:
  - `distributor->total_inventory(good)`: Current stock
  - `distributor->get_production_deficit(good, i)`: Unmet demand
  - `society->aggregate_good_stats(good)`: Production statistics

#### Lines 152-170: Worker Wealth Calculation
```cpp
double median_worker_wealth = 0;
std::vector<double> worker_wealths;
worker_wealths.reserve(society->workers.size());
for (auto & worker : society->workers) {
    worker_wealths.push_back(worker->wealth());
}
std::sort(worker_wealths.begin(), worker_wealths.end());
if (worker_wealths.size() % 2 == 0) {
    median_worker_wealth = (worker_wealths[worker_wealths.size() / 2 - 1] +
                           worker_wealths[worker_wealths.size() / 2]) / 2;
} else {
    median_worker_wealth = worker_wealths[worker_wealths.size() / 2];
}

main_plot->add_y("Median Worker Wealth", median_worker_wealth);
main_plot->add_y("Society Reserve", society->reserve);
```
**Wealth Tracking**:
- Collects all worker wealth values
- Calculates true median (handles even/odd population sizes)
- Tracks society reserve (accumulated from 20% of wages)

#### Lines 193-204: Graph Output
```cpp
if (!arg_present("--save") && !arg_present("--show")) {
    return 0;
}

plot_handler.plot();

if (arg_present("--save")) {
    plot_handler.save("img/output.png");
}
if (arg_present("--show")) {
    plot_handler.show();
}
```
**Visualization Control**:
- Only generates graphs if `--save` or `--show` flags present
- `plot_handler.plot()`: Renders all plots to figure
- Saves to `img/output.png` or displays interactively

## Core Classes

### Society Class (`include/Society.hpp`, `src/Society.cpp`)

#### Header Structure (Lines 11-50)
```cpp
struct Config {
    double plan_cycle_duration = 90;
    double fic = 0.8;  // Fraction of income consumed
    double workday_length = 8.0;
};

class Society {
    std::vector<Worker *> workers;
    std::vector<Firm *> firms;
    std::vector<Good *> goods;
    std::vector<Distributor *> distributors;
    
    Config config;
    Accountant * accountant;
    int plan_cycle = 0;
    double reserve = 0.0;
};
```

#### Key Methods Analysis

**`Society::tick_cycle()` (Lines 42-86)**:
```cpp
void Society::tick_cycle() {
    // Daily ticks for entire cycle
    for (int day = 0; day < config.plan_cycle_duration; day++) {
        tick();
    }
    
    plan_cycle++;
    
    // Generate new plans
    for (auto & firm : firms) {
        firm->new_plans();
    }
    
    // Worker redistribution logic...
}
```
**Critical Function**: Runs 90 daily ticks, then redistributes workers based on understaffing ratios.

**`Society::distribute_worker()` (Lines 17-34)**:
```cpp
void Society::distribute_worker(Worker * worker, int index, int total) {
    double frac = (index + 1) / static_cast<double>(total);
    
    int total_jobs = 0;
    for (auto & firm : firms) {
        total_jobs += firm->total_ideal_jobs();
    }
    
    // Proportional assignment based on job availability
    double running = 0;
    int firm_idx = 0;
    while (running < frac) {
        double firm_share = firms[firm_idx]->total_ideal_jobs() / static_cast<double>(total_jobs);
        running += firm_share;
        firm_idx++;
    }
    
    firms[firm_idx - 1]->employ(worker, true);
}
```
**Algorithm**: Distributes workers proportionally based on firms' ideal job counts.

### PlotHandler Class (`include/PlotHandler.hpp`, `src/PlotHandler.cpp`)

#### Core Data Structures (Lines 7-35)
```cpp
struct PlotLine {
    std::string key;
    std::initializer_list<float> color;
    bool hidden = false;
    std::vector<double> y_data;
};

class Plot {
    std::vector<double> x_data;
    robin_hood::unordered_map<std::string, PlotLine> lines;
    std::string title, xlabel, ylabel;
    bool hidden = false;
};

class PlotHandler {
    int width, height;
    matplot::figure_handle figure;
    robin_hood::unordered_map<std::string, Plot> plots;
};
```

#### Graph Rendering Logic (`PlotHandler::plot()`, Lines 53-98)
```cpp
void PlotHandler::plot() {
    matplot::figure(this->figure);
    matplot::cla();  // Clear entire figure
    figure->size(width, height);
    
    // Count visible plots
    size_t num_visible_plots = 0;
    for (auto const & [name, plot] : plots) {
        if (!plot.hidden) {
            num_visible_plots++;
        }
    }
    
    size_t grid_dim = static_cast<size_t>(ceil(sqrt(num_visible_plots)));
    
    size_t current_plot_idx = 0;
    for (auto const & [name, plot] : plots) {
        if (plot.hidden) continue;
        
        matplot::subplot(grid_dim, grid_dim, current_plot_idx);
        matplot::cla();
        matplot::hold(matplot::on);
        
        // Draw each line
        for (auto const & [key, line] : plot.lines) {
            if (!line.hidden) {
                auto p = matplot::plot(plot.x_data, line.y_data);
                p->color(line.color);
                p->display_name(line.key);
                p->line_width(2);
            }
        }
        
        matplot::title(plot.title);
        matplot::xlabel(plot.xlabel);
        matplot::ylabel(plot.ylabel);
        matplot::legend(matplot::on);
        
        current_plot_idx++;
    }
}
```
**Grid Layout**: Automatically arranges plots in square grid based on `sqrt(visible_plots)`.

### Good Class (`include/Good.hpp`, `src/Good.cpp`)

#### Structure (Lines 13-36)
```cpp
struct Good {
    std::string name;
    double target_surplus;
    robin_hood::unordered_map<Good *, double> means;  // Input requirements
    double labor_required;
    bool discrete;
    double value;  // Computed from labor + means
};
```

#### Value Calculation (Lines 13-21)
```cpp
double Good::means_value() {
    double total = 0.0;
    for (const auto & [resource, amount] : means) {
        total += resource->value * amount;
    }
    return total;
}

void Good::recompute_value() { 
    value = labor_required + means_value(); 
}
```
**Economic Model**: Good value = direct labor + sum of input values.

### Project Class (`include/Project.hpp`, `src/Project.cpp`)

#### Core Production Logic (`Project::tick()`, Lines 21-80)
```cpp
void Project::tick() {
    double workday_length = society->config.workday_length;
    if (hours_left <= 0 || workers.empty()) return;
    
    double production_cost_per = plan.good->value;
    
    // Calculate maximum producible amount
    double amount_to_produce = std::min(
        hours_left / production_cost_per,
        workday_length * std::min(ideal_workers, num_workers()) / plan.good->labor_required
    );
    
    if (amount_to_produce <= 0) return;
    
    // Check input availability and purchase if needed
    double max_production_fraction = 1;
    for (auto [good, quantity] : plan.good->means) {
        double total_needed = quantity * amount_to_produce;
        double available = firm->total_inventory(good);
        double required = std::max(0.0, total_needed - available);
        
        if (required > 0) {
            auto [purchased, cost] = society->distributors[0]->purchase(good, required);
            hours_left -= cost;
            firm->add_stock(good, nullptr, purchased);
            available += purchased;
        }
        
        max_production_fraction = std::min(max_production_fraction, available / total_needed);
    }
    
    amount_to_produce *= max_production_fraction;
    
    // Extract inputs from inventory
    for (auto [good, quantity] : plan.good->means) {
        firm->take_from_inventory(good, quantity * amount_to_produce);
    }
    
    // Pay workers and track production
    double hours_worked = amount_to_produce * plan.good->labor_required / num_workers();
    double labor_done = hours_worked * num_workers();
    hours_left -= labor_done;
    
    for (auto & worker : workers) {
        society->pay(worker, workday_length);
    }
    
    goods_produced += amount_to_produce;
    
    // Distribute output to distributors
    double per_dist = amount_to_produce / society->distributors.size();
    for (auto & distributor : society->distributors) {
        distributor->add_stock(plan.good, this, per_dist);
    }
}
```
**Production Algorithm**:
1. Calculate max production based on budget and workforce
2. Check input availability, purchase if needed
3. Scale production by input constraints
4. Extract inputs, pay workers, record output
5. Distribute finished goods to distributors

### Firm Class (`include/Firm.hpp`, `src/Firm.cpp`)

#### Plan Generation (`Firm::generate_plan()`, Lines 249-311)
```cpp
Plan Firm::generate_plan(Project * project) {
    if (project->plan_cycle < 2) {
        // First two cycles: use existing plan
        return project->plan;
    }
    
    // Calculate remaining inventory for this project
    const double remaining_inventory =
        society->distributors[0]->get_project_inventory(project) -
        (project->plan.good->target_surplus - 
         (society->distributors[0]->total_inventory(project->plan.good) -
          society->distributors[0]->get_project_inventory(project)));
    
    // Get production deficit
    const double deficit =
        society->distributors[0]->get_production_deficit(project->plan.good, project->plan_cycle);
    
    // Estimate actual consumption
    const double consumption = std::max(0.0, 
        project->goods_produced - remaining_inventory + deficit);
    
    // Calculate surplus needed
    const double surplus_needed = std::max(0.0,
        project->plan.good->target_surplus -
        society->distributors[0]->total_inventory(project->plan.good));
    
    // New plan quantity
    const double estimated_necessity = std::max(0.0, consumption + surplus_needed);
    const double quantity = estimated_necessity;
    
    return Plan{
        .fixed_capital = 0,
        .means = project->plan.good->means_value() * quantity,
        .labor = project->plan.good->labor_required * quantity,
        .good = project->plan.good,
        .quantity = quantity,
    };
}
```
**Planning Algorithm**: Uses consumption history and deficit tracking to estimate future demand.

### Worker Class (`include/Worker.hpp`, `src/Worker.cpp`)

#### Need Fulfillment (`Worker::fulfill_needs()`, Lines 79-109)
```cpp
void Worker::fulfill_needs() {
    std::vector<WorkerNeed *> kept;
    kept.reserve(needs.size());
    
    for (size_t i = 0; i < needs.size(); ++i) {
        WorkerNeed * need = needs[i];
        const double purchaseable_amount = credits / need->good->value;
        const double amount_to_buy = std::min(purchaseable_amount, need->amount);
        
        if (amount_to_buy <= 0) {
            // Preserve remaining needs and exit
            kept.insert(kept.end(), needs.begin() + i, needs.end());
            break;
        }
        
        auto [purchased_amount, cost] = distributor->purchase(need->good, amount_to_buy);
        if (!spend(cost)) {
            kept.insert(kept.end(), needs.begin() + i, needs.end());
            break;
        }
        
        need->amount -= purchased_amount;
        if (need->amount > 0) {
            kept.push_back(need);
        } else {
            delete need;
        }
    }
    needs.swap(kept);
}
```
**Purchase Logic**: Workers buy goods in priority order until credits exhausted.

### Distributor Class (`include/Distributor.hpp`, `src/Distributor.cpp`)

#### Purchase Mechanism (`Distributor::purchase()`, Lines 80-115)
```cpp
std::pair<double, double> Distributor::purchase(Good * good, double amount) {
    if (!inventory.count(good) || total_inventory(good) <= 0) {
        return {0, 0};
    }
    
    double available_amount = total_inventory(good);
    double purchase_amount = std::min(available_amount, amount);
    double cost = purchase_amount * good->value;
    
    // Track deficit
    auto & deficit_history = inventory[good].deficit_history;
    size_t needed = society->plan_cycle + 1;
    if (deficit_history.size() < needed) {
        deficit_history.resize(needed, 0.0);
    }
    inventory[good].deficit_history[society->plan_cycle] += (amount - purchase_amount);
    
    if (purchase_amount <= 0) return {0, 0};
    
    double remaining = purchase_amount;
    
    // Clear old inventory first
    remaining = take_from_old_projects(good, remaining);
    
    // Take from new projects
    take_from_new_projects(good, remaining);
    
    purchase_records.push_back(PurchaseRecord{
        .purchaser = this,
        .good = good,
        .amount = purchase_amount,
        .plan_cycle = society->plan_cycle
    });
    
    return {purchase_amount, cost};
}
```
**Inventory Management**: Prioritizes old stock, tracks deficits for planning.

## Key Algorithms and Data Flows

### 1. Economic Cycle Flow
```
Society::tick_cycle() → 90 × Society::tick() → Firm::tick() → Project::tick()
                    ↓
                Plan Generation → Worker Redistribution → Next Cycle
```

### 2. Production Chain
```
Project::tick() → Check Inputs → Purchase from Distributor → Produce → 
Pay Workers → Add to Distributor Inventory
```

### 3. Demand-Supply Feedback
```
Worker Needs → Distributor Purchase → Deficit Tracking → 
Plan Generation → Production Adjustment
```

### 4. Graph Data Collection
```
Main Loop → Collect Stats → Plot Updates → Final Rendering
```

## Critical Variables and Their Roles

### In main.cpp:
- `population_size = 100`: Total worker count
- `plan_cycle_duration = 90`: Quarterly cycles
- `fic = 0.8`: 80% consumption, 20% savings
- `target_surplus = 5000`: Safety stock levels

### Graph Variables:
- `plot_handler`: Master plotting object (1200×900)
- `main_plot`: Economic overview (wealth, reserves)
- `good_plots`: Per-good production tracking
- `tick_times`: Performance monitoring

### Economic Flow Variables:
- `society->reserve`: Accumulated savings (20% of wages)
- `project->hours_left`: Remaining production budget
- `worker->credits`: Individual purchasing power
- `distributor->inventory`: Stock tracking by project

## Rewriting Checklist

### Core Systems to Recreate:
1. **Society**: Central coordinator with config, workers, firms, goods
2. **PlotHandler**: Multi-plot visualization with automatic grid layout
3. **Good**: Value calculation from labor + inputs
4. **Project**: Production logic with input constraints
5. **Firm**: Plan generation based on consumption history
6. **Worker**: Priority-based need fulfillment
7. **Distributor**: Inventory management with deficit tracking

### Key Relationships:
- Society owns all entities
- Projects belong to Firms
- Workers have needs for Goods
- Distributors manage inventory from Projects
- Plans link Goods to production targets

### Critical Algorithms:
- Worker distribution by firm job ratios
- Production constrained by inputs and labor
- Plan generation from consumption + deficit
- Graph rendering with automatic layout
- Need fulfillment in priority order

### Data Structures:
- robin_hood hash maps for performance
- Vectors for collections
- Plan cycles for temporal coordination
- Deficit history for demand tracking

This manual provides the complete blueprint for recreating the economic simulation system with all critical implementation details preserved.
