#include "../include/Distributor.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <set>

#include "../include/Society.hpp"

Distributor::Distributor(Society * society) : Firm(society) {}

std::pair<double, double> Distributor::purchase(Good * good, double amount) {
    if (!inventory.count(good) || total_inventory(good) <= 0) {
        return {0, 0};
    }

    double available_amount = total_inventory(good);
    double purchase_amount = std::min(available_amount, amount);
    double cost = purchase_amount * good->value;

    inventory[good].deficit_history[society->plan_cycle] += (amount - purchase_amount);

    if (purchase_amount > 0) {
        double remaining = purchase_amount;

        // clear out old inventory first
        std::vector<Project *> old_projects;
        for (auto & [project, qty] : inventory[good].projects) {
            if (project->plan_cycle < society->plan_cycle - 1) {
                old_projects.push_back(project);
            }
        }

        if (!old_projects.empty()) {
            std::sort(old_projects.begin(), old_projects.end(), [](Project * a, Project * b) {
                return a->plan_cycle < b->plan_cycle;
            });

            for (auto & project : old_projects) {
                double & project_amount = inventory[good].projects[project];
                if (project_amount >= remaining) {
                    project_amount -= remaining;
                    if (project_amount <= 0) {
                        inventory[good].projects.erase(project);
                    }
                    remaining = 0;
                    break;
                } else {
                    remaining -= project_amount;
                    inventory[good].projects.erase(project);
                }
            }
        }

        while (remaining > 0 && !inventory[good].projects.empty()) {
            // select a random project, weighted by amount
            double total = total_inventory(good);
            double r = ((double)rand() / RAND_MAX) * total;
            // pick by cumulative weights
            Project * selected = nullptr;
            double cumulative = 0.0;
            for (auto & entry : inventory[good].projects) {
                cumulative += entry.second;
                if (r < cumulative) {
                    selected = entry.first;
                    break;
                }
            }
            if (!selected) {
                // fallback
                selected = inventory[good].projects.begin()->first;
            }
            // remove up to remaining amount
            double & proj_qty = inventory[good].projects[selected];
            double remove_amt = std::min(proj_qty, remaining);
            proj_qty -= remove_amt;
            remaining -= remove_amt;
            if (proj_qty <= 0) {
                inventory[good].projects.erase(selected);
            }
        }

        return {purchase_amount, cost};
    }
    return {0, 0};
}

double Distributor::get_project_inventory(Project * project) {
    const auto good = project->plan.good;

    if (!inventory.count(good) || !inventory[good].projects.count(project)) {
        return 0.0;
    }
    
    return inventory[good].projects[project];
}

double Distributor::get_production_deficit(Good * good, int plan_cycle) {
    if (!inventory.count(good)) {
        return 0.0;
    }
    const auto & deficit_history = inventory[good].deficit_history;
    if (plan_cycle < 0 || plan_cycle >= (int)deficit_history.size()) {
        return 0.0;
    }
    return deficit_history[plan_cycle];
}

void Distributor::display_inventory(int rows) {
    int count = 0;
    for (const auto & [good, item] : inventory) {
        if (rows > 0 && count >= rows) break;
        printf("%s: %.2f\n", good->name.c_str(), total_inventory(good));
        std::set<int> cycles;
        for (const auto & [project, qty] : item.projects) {
            cycles.insert(project->plan_cycle);
        }
        printf("  Plan cycles in inventory: ");
        for (int cycle : cycles) {
            printf("%d ", cycle);
        }
        printf("\n");
        count++;
    }
}

void Distributor::head() { display_inventory(5); }