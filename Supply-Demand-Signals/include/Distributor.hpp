#pragma once

#include <utility>
#include <variant>
#include <vector>

#include "Firm.hpp"
#include "Project.hpp"

struct PurchaseRecord {
        std::variant<Firm *, Worker *> purchaser;
        Good * good;
        double amount;
        int plan_cycle;
};

class Distributor : public Firm {
    private:
        std::vector<Project *> old_projects(Good * good);
        // Returns the remaining amount needed to be purchased that can not be taken from old
        // projects
        double take_from_old_projects(Good * good, double remaining);
        // Precondition: there is enough inventory of the good to purchase the amount
        void take_from_new_projects(Good * good, double remaining);

        std::vector<PurchaseRecord> purchase_records;

    public:
        Distributor(Society * society);

        // Returns <quantity purchased, credits spent>
        std::pair<double, double> purchase(Worker * worker, Good * good, double amount);

        // Get the remaining inventory for a given project
        double get_project_inventory(Project * project);

        double get_production_deficit(Good * good, int plan_cycle);

        // Used when a worker needs something, but can't afford it
        void simulate_need(Good * good, double amount);

        // 0 rows = all
        void display_inventory(int rows = 0);

        // display first few rows of inventory
        void head();
};
