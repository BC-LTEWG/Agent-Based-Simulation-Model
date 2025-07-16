#include <string>
#include <vector>

using namespace std;

class Company;
class Worker;

struct Plan {

    string product_name;
    int quantity;
    string unit;
    Company * company;
    double fixed_capital;
    double raw_materials;
    double labor;
    double product;
    Plan();
};

class Company {

    public:

        Company(string name);
        string name();
        void add_plan(Plan plan);
        void print_plans();
        void employ(Worker * worker);

    private:

        string company_name;
        vector<Plan> plans;
        vector<Worker *> workers;
};
