#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <unordered_set>
#include <vector>

#define private public
#define protected public
#include "Producer.h"
#include "Distributor.h"
#include "Product.h"
#include "Society.h"
#undef protected
#undef private
#include "doctest.h"

TEST_SUITE_BEGIN("Producer");

namespace {

Product * make_output_product(Product * input = nullptr) {
    Product * output = new Product("Test output");
    output->required_abilities = {Person::ABILITY_1};
    output->inputs_per_unit.clear();
    output->machines_needed.clear();
    output->price_per_unit = 10.0;
    output->living_labor_per_unit = 1.0;
    output->order_size = 1;
    if (input) {
        output->inputs_per_unit[input] = 1.0;
    }
    return output;
}

Plan make_plan(Producer * producer, Product * product, Firm * customer, int quantity = 1) {
    Plan plan{};
    plan.order = new Order(product, quantity, customer, 5);
    plan.firm = producer;
    plan.training_time = 0;
    plan.training_time_remaining = 0;
    plan.predicted_turnaround_time = 1;
    plan.machinery_cost = 0.0;
    plan.labor_hours = quantity;
    plan.raw_materials = 0.0;
    plan.total_hours = plan.labor_hours + plan.raw_materials;
    plan.prd = 0.0;
    plan.labor_hours_remaining = plan.labor_hours;
    plan.raw_materials_remaining = plan.raw_materials;
    plan.total_hours_remaining = plan.total_hours;
    plan.outgoing_units_consumed = 0;
    return plan;
}

} 

TEST_CASE("Producer constructor initializes reorderable inventory") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_goods().size() > 1);

    Product * input = society->get_goods()[0];
    Product * output = make_output_product(input);
    Producer producer(society, {output});

    std::unordered_set<Product *> products_to_reorder = producer.get_products_to_reorder();
    REQUIRE(products_to_reorder.count(input));
    CHECK(producer.inventory[input] == doctest::Approx(
            producer.get_reorder_threshold(input) * FIRM_INITIAL_INVENTORY_MULTIPLIER));
}

TEST_CASE("Producer can_produce checks catalog membership") {
    Society * society = Society::get_instance();
    std::vector<Product *>& products = society->get_products();
    REQUIRE(products.size() > 1);

    std::unordered_set<Product *> catalog = {products[0]};
    Producer producer(society, catalog);

    CHECK(producer.can_produce(products[0]));
    CHECK_FALSE(producer.can_produce(products[1]));
}

TEST_CASE("Producer handles on_time_step call") {
    Society * society = Society::get_instance();
    std::vector<Product *>& products = society->get_products();
    REQUIRE(products.size() > 0);

    Producer producer(society, {products[0]});

    CHECK_NOTHROW(producer.on_time_step());
}

TEST_CASE("Producer draft_plan rejects order without required inventory") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_goods().size() > 1);
    REQUIRE(society->get_distributors().size() > 0);

    Product * input = society->get_goods()[0];
    Product * output = make_output_product(input);
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Order * order = new Order(output, 10, customer, 5);

    producer.inventory[input] = 0;
    CHECK(producer.draft_plan(order) == DRAFT_ORDER_REJECTED);
}

TEST_CASE("Producer draft_plan creates a draft plan when inventory is enough") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_goods().size() > 1);
    REQUIRE(society->get_distributors().size() > 0);

    Product * input = society->get_goods()[0];
    Product * output = make_output_product(input);
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Order * order = new Order(output, 2, customer, 5);

    producer.inventory[input] = 100;
    int turnaround = producer.draft_plan(order);
    CHECK(turnaround != DRAFT_ORDER_REJECTED);
    CHECK(producer.order_to_draft_plan.count(order));
    CHECK(producer.order_to_draft_plan[order] != nullptr);
}

TEST_CASE("Producer drop_order removes tracked draft plan") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_goods().size() > 1);
    REQUIRE(society->get_distributors().size() > 0);

    Product * input = society->get_goods()[0];
    Product * output = make_output_product(input);
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Order * order = new Order(output, 2, customer, 5);

    producer.inventory[input] = 100;
    REQUIRE(producer.draft_plan(order) != DRAFT_ORDER_REJECTED);
    REQUIRE(producer.order_to_draft_plan.count(order));
    producer.drop_order(order);
    CHECK_FALSE(producer.order_to_draft_plan.count(order));
}

TEST_CASE("Producer pursue_order moves a draft plan to in-progress") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_goods().size() > 1);
    REQUIRE(society->get_distributors().size() > 0);

    Product * input = society->get_goods()[0];
    Product * output = make_output_product(input);
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Order * order = new Order(output, 2, customer, 5);

    producer.inventory[input] = 100;
    REQUIRE(producer.draft_plan(order) != DRAFT_ORDER_REJECTED);
    CHECK(producer.pursue_order(order));
    CHECK(producer.order_to_draft_plan.count(order));
    CHECK(producer.order_to_draft_plan[order] == nullptr);
    CHECK(producer.plans_in_progress.size() == 1);
}

TEST_CASE("Producer start_plan consumes inputs and updates raw materials account") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_goods().size() > 1);
    REQUIRE(society->get_distributors().size() > 0);

    Product * input = society->get_goods()[0];
    Product * output = make_output_product(input);
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Plan plan = make_plan(&producer, output, customer, 3);
    plan.raw_materials = 12.0;
    plan.raw_materials_remaining = 12.0;
    producer.inventory[input] = 100;
    producer.pooled_input_value_account = 0.0;

    producer.start_plan(&plan);
    CHECK(producer.inventory[input] == 97);
    CHECK(producer.pooled_input_value_account == doctest::Approx(12.0));
    CHECK(plan.raw_materials == doctest::Approx(0.0));
}

TEST_CASE("Producer move_plan_forward_one_step progresses labor and total hours") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_distributors().size() > 0);
    REQUIRE(society->get_unemployed_people().size() > 0);

    Product * output = make_output_product();
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Plan plan = make_plan(&producer, output, customer, 5);
    plan.training_time = 1;
    plan.training_time_remaining = 1;
    plan.labor_hours = plan.labor_hours_remaining = 5;
    plan.raw_materials = plan.raw_materials_remaining = 2.0;
    plan.total_hours = plan.total_hours_remaining = 7.0;
    plan.workers.push_back(society->get_unemployed_people()[0]);

    producer.move_plan_forward_one_step(&plan);
    CHECK(plan.training_time_remaining == 0);
    CHECK(plan.labor_hours_remaining == 4);
    CHECK(plan.total_hours_remaining == doctest::Approx(6.0));
}

TEST_CASE("Producer move_plan_forward_one_step consumes pooled input value") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_distributors().size() > 0);
    REQUIRE(society->get_unemployed_people().size() > 0);

    Product * output = make_output_product();
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Plan plan = make_plan(&producer, output, customer, 5);
    plan.training_time = 0;
    plan.training_time_remaining = 0;
    plan.labor_hours = plan.labor_hours_remaining = 5;
    plan.raw_materials = plan.raw_materials_remaining = 10.0;
    plan.total_hours = plan.total_hours_remaining = 15.0;
    plan.workers.push_back(society->get_unemployed_people()[0]);
    producer.pooled_input_value_account = 10.0;

    producer.move_plan_forward_one_step(&plan);
    CHECK(plan.labor_hours_remaining == 4);
    CHECK(plan.raw_materials_remaining == doctest::Approx(8.0));
    CHECK(producer.pooled_input_value_account == doctest::Approx(8.0));
    CHECK(plan.total_hours_remaining == doctest::Approx(12.0));
}

TEST_CASE("Producer reorder_raw_materials can source from another producer") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_distributors().size() > 0);
    REQUIRE(society->get_products().size() > 0);
    REQUIRE(society->get_producers().size() > 0);

    Product * product = society->get_products()[0];
    Product * output = make_output_product();
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Plan plan = make_plan(&producer, output, customer, 1);
    producer.pooled_input_value_account = 10000.0;

    Producer * supplier = society->get_producers()[0];
    supplier->pooled_input_value_account = 0.0;
    int old_supplier_inventory = supplier->get_inventory(product);
    supplier->inventory[product] = 1000;
    int before = supplier->inventory[product];

    producer.reorder_raw_materials(&plan, product);
    CHECK(supplier->inventory[product] <= before);
    supplier->inventory[product] = old_supplier_inventory;
}

TEST_CASE("Producer end_plan marks order complete and notifies customer") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_distributors().size() > 0);

    Product * output = make_output_product();
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Plan plan = make_plan(&producer, output, customer, 2);
    plan.prd = 0.0;

    producer.end_plan(&plan);
    CHECK(plan.order->status == Order::ORDER_FINISHED);
}

TEST_CASE("Producer move_plans_forward_one_step finishes zero-hour plans") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_distributors().size() > 0);

    Product * output = make_output_product();
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Plan * plan = new Plan(make_plan(&producer, output, customer, 1));
    plan->labor_hours = plan->labor_hours_remaining = 0;
    plan->raw_materials = plan->raw_materials_remaining = 0.0;
    plan->total_hours = plan->total_hours_remaining = 0.0;
    producer.plans_in_progress.push_back(plan);

    producer.move_plans_forward_one_step();
    CHECK(producer.plans_in_progress.empty());
    CHECK(plan->order->status == Order::ORDER_FINISHED);
}

TEST_CASE("Producer get_products_to_reorder returns unique input products") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_goods().size() > 2);

    Product * input_a = society->get_goods()[0];
    Product * input_b = society->get_goods()[1];
    Product * output = make_output_product();
    output->inputs_per_unit[input_a] = 1.0;
    output->inputs_per_unit[input_b] = 2.0;

    Producer producer(society, {output});
    std::unordered_set<Product *> products_to_reorder = producer.get_products_to_reorder();
    CHECK(products_to_reorder.count(input_a));
    CHECK(products_to_reorder.count(input_b));
}

TEST_CASE("Producer logging helpers execute") {
    Society * society = Society::get_instance();
    REQUIRE(society->get_distributors().size() > 0);

    Product * output = make_output_product();
    Producer producer(society, {output});
    Firm * customer = society->get_distributors()[0];
    Plan * plan = new Plan(make_plan(&producer, output, customer, 1));
    producer.plans_in_progress.push_back(plan);

    CHECK_NOTHROW(producer.log_plans());
    CHECK_NOTHROW(producer.log_draft_plan(plan));
    CHECK_NOTHROW(producer.log_dropped_order(plan->order));
    CHECK_NOTHROW(producer.log_pursued_plan(plan));
}

TEST_SUITE_END();