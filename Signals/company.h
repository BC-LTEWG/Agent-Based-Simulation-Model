#include <string>
#include <vector>

using namespace std;

class Company;
class Worker;

struct Inputs {
    double fixed_capital;
    double raw_materials;
    double labor;
    Inputs();
};

struct Plan {
    string product_name;
    int quantity;
    string unit;
    Company * company;
    Inputs inputs;
    double product;
    Plan();
};

class Company {
    friend class Accountant;
    public:
        Company(string name);
        string name();
        void add_plan(Plan * plan);
        void print_plans();
        void employ(Worker * worker);
    private:
        string company_name;
        vector<Plan *> plans;
        vector<Worker *> workers;
};


/*
f means of production  p
c raw materials        r
l labor                a
prd product = -(f + c + l)
*/
