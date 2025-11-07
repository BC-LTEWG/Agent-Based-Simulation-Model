#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "../header/Firm.h"
#include "../header/Person.h"
#include "../header/Product.h"
#include "../header/Machine.h"
#include "../header/Producer.h"


int main() {
    std::cout << "=== Testing classes ===" << std::endl << std::endl;

    std::cout << "Creating workers..." << std::endl;
    std::vector<Person*> workers = {
        new Person({{"Programming", 10}, {"Marketing", 5}}, 25, HEALTHY, {"Programming", "Marketing"}), 
        new Person({{"Accounting", 8}, {"Design", 6}}, 30, HEALTHY, {"Accounting", "Design"}), 
        new Person({{"Management", 12}}, 35, HEALTHY, {"Management"})
    };
    std::cout << "Created " << workers.size() << " workers" << std::endl << std::endl;

    std::cout << "Creating machines..." << std::endl;
    std::vector<Machine*> machines = {
        new Machine("Computer", 10000), 
        new Machine("Printer", 5000), 
        new Machine("Scanner", 3000)
    };
    std::cout << "Created " << machines.size() << " machines" << std::endl << std::endl;

    std::cout << "Creating products..." << std::endl;
    Product* software = new Product("Software Application", 1000.0, 10);
    Product* hardware = new Product("Hardware Device", 500.0, 50);
    std::cout << "Product 1: " << software->productName << " - $" << software->pricePerUnit << " (order: " << software->orderSize << ")" << std::endl;
    std::cout << "Product 2: " << hardware->productName << " - $" << hardware->pricePerUnit << " (order: " << hardware->orderSize << ")" << std::endl << std::endl;

    std::cout << "Creating firm..." << std::endl;
    Firm firm(machines, workers, {});
    std::cout << "Firm has " << firm.workers.size() << " workers" << std::endl;
    std::cout << "Firm has " << firm.machines.size() << " machines" << std::endl;
    std::cout << "Firm has " << firm.plans.size() << " plans" << std::endl << std::endl;

    std::cout << "Creating plan with back-reference to firm..." << std::endl;
    Plan* plan1 = new Plan{*software, {*machines[0], *machines[1]}, firm, 100, 50, 10, 5, 100, 50};
    std::cout << "Plan product: " << plan1->product.productName << std::endl;
    std::cout << "Plan labor hours: " << plan1->laborHours << std::endl;
    std::cout << "Plan labor hours remaining: " << plan1->laborHoursRemaining << std::endl;
    std::cout << "Plan raw materials remaining: " << plan1->rawMaterialsRemaining << std::endl << std::endl;

    firm.plans.push_back(plan1);
    std::cout << "Added plan to firm. Firm now has " << firm.plans.size() << " plans" << std::endl << std::endl;

    std::cout << "Creating producer (derived from Firm)..." << std::endl;
    Producer producer(machines, workers);
    std::cout << "Producer has " << producer.workers.size() << " workers" << std::endl;
    std::cout << "Producer has " << producer.machines.size() << " machines" << std::endl << std::endl;

    std::cout << "Testing producer-specific features..." << std::endl;
    Producer producer2(machines, workers, {plan1});
    std::cout << "Producer2 created with plans: " << producer2.plans.size() << " plans" << std::endl << std::endl;

    std::cout << "Testing machine usage..." << std::endl;
    machines[0]->hoursUsed = 500;
    machines[1]->hoursUsed = 1200;
    machines[2]->hoursUsed = 300;
    std::cout << machines[0]->name << " has been used for " << machines[0]->getHoursUsed() << " hours (lifetime: " << machines[0]->lifetime << ")" << std::endl;
    std::cout << machines[1]->name << " has been used for " << machines[1]->getHoursUsed() << " hours (lifetime: " << machines[1]->lifetime << ")" << std::endl;
    std::cout << machines[2]->name << " has been used for " << machines[2]->getHoursUsed() << " hours (lifetime: " << machines[2]->lifetime << ")" << std::endl << std::endl;

    std::cout << "Testing worker expertise..." << std::endl;
    for(size_t i = 0; i < workers.size(); i++) {
        std::cout << "Worker " << (i+1) << " (age " << workers[i]->age << "): ";
        for(const auto& skill : workers[i]->expertise) {
            std::cout << skill.first << "(" << skill.second << ") ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Testing multiple plans..." << std::endl;
    Plan* plan2 = new Plan{*hardware, {*machines[2]}, firm, 200, 100, 50, 10, 200, 100};
    firm.plans.push_back(plan2);
    std::cout << "Firm now has " << firm.plans.size() << " plans" << std::endl;
    for(size_t i = 0; i < firm.plans.size(); i++) {
        std::cout << "  Plan " << (i+1) << ": " << firm.plans[i]->product.productName << " - " 
                  << firm.plans[i]->totalQuantity << " units, " 
                  << firm.plans[i]->laborHours << " labor hours" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Testing plan history map..." << std::endl;
    firm.planHistory[software].push_back(plan1);
    firm.planHistory[hardware].push_back(plan2);
    std::cout << "Plan history contains " << firm.planHistory.size() << " products" << std::endl;
    std::cout << "Plans for " << software->productName << ": " << firm.planHistory[software].size() << std::endl;
    std::cout << "Plans for " << hardware->productName << ": " << firm.planHistory[hardware].size() << std::endl << std::endl;

    std::cout << "Testing worker health status..." << std::endl;
    workers[1]->healthStatus = UNHEALTHY;
    workers[2]->healthStatus = RECOVERING;
    for(size_t i = 0; i < workers.size(); i++) {
        std::string status;
        if(workers[i]->getHealthStatus() == HEALTHY) status = "HEALTHY";
        else if(workers[i]->getHealthStatus() == UNHEALTHY) status = "UNHEALTHY";
        else status = "RECOVERING";
        std::cout << "Worker " << (i+1) << " health: " << status << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Testing Product struct (now public members)..." << std::endl;
    software->pricePerUnit = 1200.0;
    software->orderSize = 15;
    std::cout << "Updated " << software->productName << " - new price: $" << software->pricePerUnit << ", new order size: " << software->orderSize << std::endl << std::endl;

    std::cout << "Testing Plan remaining resources..." << std::endl;
    plan1->laborHoursRemaining -= 25;
    plan1->rawMaterialsRemaining -= 10;
    std::cout << "Plan 1 progress: " << (100 - plan1->laborHoursRemaining) << "/" << plan1->laborHours << " labor hours used" << std::endl;
    std::cout << "Plan 1 materials: " << (50 - plan1->rawMaterialsRemaining) << "/" << plan1->rawMaterials << " raw materials used" << std::endl << std::endl;

    std::cout << "=== All tests completed successfully! ===" << std::endl;

    for(auto w : workers) delete w;
    for(auto m : machines) delete m;
    delete software;
    delete hardware;
    delete plan1;
    delete plan2;
    
    return 0;
}
