#include "Person.h"

struct Plan;

class Logger {
    public:
        static Logger * get_instance();
        static void log_event(std::string message);
        static void log_firm_shipment_received(std::string product_name, int quantity);
        static void log_firm_inventory_level(std::string product_name, int level);
        static void log_firm_reorder(std::string product_name, int quantity);
        static void log_firm_accepted_order(
                std::string product_name,
                int turnaround_time,
                bool accepted
                );
        static void log_person_payment(int payment);
        static void log_person_shopping(std::string product_name, int quantity);
        static void log_person_state(
                int age,
                double account,
                Person::HealthStatus health_status
                );
        static void log_producer_plans(std::vector<Plan *> plans);
        static void log_producer_draft_plan(std::string product_name, int quantity);
        static void log_producer_dropped_order(std::string product_name, int quantity);
        static void log_producer_pursued_plan(Plan * draft_plan);
    private:
        Logger();
        std::vector<std::tuple<int, std::string, int>> firm_shipments_received;
        std::vector<std::tuple<int, std::string, int>> firm_inventory_levels;
        std::vector<std::tuple<int, std::string, int>> firm_reorders;
        std::vector<std::tuple<int, std::string, int>> firm_accepted_orders;
        std::vector<std::tuple<int, std::string, int>> firm_no_producer_found;
        std::vector<std::pair<int, int>> person_payments;
        std::vector<std::tuple<int, std::string, int>> person_shopping;
        std::vector<std::tuple<int, int, double, Person::HealthStatus>> person_state;
        std::vector<std::pair<int, Plan *>> producer_plans;
        std::vector<std::tuple<int, std::string, int>> producer_draft_plans;
        std::vector<std::tuple<int, std::string, int>> producer_dropped_orders;
        std::vector<std::tuple<int, Plan *>> producer_pursued_draft_plans;
};
