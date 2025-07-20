struct Inputs;
struct Plan;

class Accountant {
    public:
        bool approve_plan(Plan * draft_plan, Inputs * amended_inputs);
};
