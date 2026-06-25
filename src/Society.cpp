#include <algorithm>
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Constants.h"
#include "ConsumerGood.h"
#include "Distributor.h"
#include "Firm.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "Person.h"
#include "PriceController.h"
#include "Product.h"
#include "Producer.h"
#include "Sim.h"
#include "Society.h"

namespace {
    class MultidimensionalPerronEigenspace : public std::runtime_error {
    public:
        MultidimensionalPerronEigenspace()
            : std::runtime_error("Multidimensional Perron eigenspace detected.") {}
    };
}

static void set_abilities(
        std::vector<Ability *>& abilities,
        std::vector<Ability *>& distribution_abilities
        ) {
    for (unsigned int i = 0; i < Sim::get_num_abilities(); i++) {
        abilities.push_back(new Ability());
    }
    distribution_abilities = abilities;
    std::shuffle(distribution_abilities.begin(), distribution_abilities.end(),
            Sim::get_random_generator());
    std::uniform_int_distribution<> ability_count_dist(1, PRODUCT_ABILITY_COUNT_MAX);
    distribution_abilities.resize(ability_count_dist(Sim::get_random_generator()));
}

static void set_initial_account(
        double& initial_account,
        const std::vector<ConsumerGood *>& consumer_goods
        ) {
    initial_account = 0.0;
    for (ConsumerGood * consumer_good : consumer_goods) {
        initial_account += consumer_good->price_per_unit *
                           consumer_good->mean_consumption_frequency;
    }
    initial_account *= INITIAL_ACCOUNT_DURATION;
}


Society * Society::get_instance() {
    static bool initialized = false;
    static Society * instance = new Society;
    if (!initialized) {
        initialized = true;
        instance->initialize();
    }
    return instance;
}

Society::Society() :
    current_work_hours_daily{Sim::get_initial_work_hours_daily()},
    current_work_days_weekly{Sim::get_initial_work_days_weekly()}
{}

void Society::initialize() {
    set_abilities(abilities, distribution_abilities);
    set_initial_products();
    unsigned int num_producers = Sim::get_num_producers();
    for (unsigned int i = 0; i < num_producers; ++i) {
        Producer * producer = new Producer();
        producers.push_back(producer);
        firms.push_back(producer);
    }
    std::vector<Product *> upstream_products(goods.begin(), goods.end());
    upstream_products.reserve(goods.size() + machines.size());
    upstream_products.insert(upstream_products.end(), machines.begin(), machines.end());
    unsigned int num_upstream_products = upstream_products.size();
    if (num_producers >= num_upstream_products) {
        for (unsigned int i = 0; i < num_producers; ++i) {
            Producer * producer = producers[i];
            Product * product = upstream_products[i % num_upstream_products];
            producer->add_to_catalog(product);
            product_to_suppliers[product].push_back(producer);
        }
    } else {
        for (unsigned int i = 0; i < num_upstream_products; ++i) {
            Producer * producer = producers[i % num_producers];
            Product * product = upstream_products[i];
            producer->add_to_catalog(product);
            product_to_suppliers[product].push_back(producer);
        }
    }
    std::unordered_set<Product *> distributor_catalog(consumer_goods.begin(), consumer_goods.end());
    for (unsigned int i = 0; i < Sim::get_num_distributors(); i++) {
        Distributor * distributor = new Distributor(distributor_catalog);
        distributors.push_back(distributor);
        firms.push_back(distributor);
    }
    set_initial_account(initial_account, consumer_goods);
    for (unsigned int i = 0; i < Sim::get_num_people(); i++) {
        birth_person();
    }
}

unsigned int Society::get_id() {
    if (id) {
        throw std::invalid_argument("Society should be a singleton.");
    }
    return id;
}

void Society::on_time_step() {
    busyness = 0.0;
    average_account = 0.0;
    for (Person * person : people) {
        busyness += person->get_busyness();
        average_account += person->get_account();
    }
    busyness /= people.size();
    average_account /= people.size();

    for (Person * person : people) {
        person->on_time_step();
    }
    for (Firm * firm : firms) {
        firm->on_time_step();
    }
    PriceController::get_instance()->on_time_step();
    check_expand_public_sector();
    check_sample_busyness_data();
    check_update_work_hours();
}

void Society::set_initial_products() {
    unsigned int starting_num_goods = Sim::get_num_goods();
    const unsigned int starting_num_machines =
        starting_num_goods / Sim::get_goods_per_machine();
    for (unsigned int i = 0; i < starting_num_goods; ++i) {
        Good * new_good = new Good();
        goods.push_back(new_good);
        products.push_back(new_good);
    }
    for (unsigned int i = 0; i < starting_num_goods; ++i) {
        ConsumerGood * new_consumer_good = new ConsumerGood(goods[i]); 
        consumer_goods.push_back(new_consumer_good);
        products.push_back(new_consumer_good);
    }
    for (unsigned int i = 0; i < starting_num_machines; ++i) {
        Machine * new_machine = new Machine();
        machines.push_back(new_machine);
        products.push_back(new_machine);
    }
    for (Product * product : products) {
        product->set_inputs();
        product->set_machines();
    }
    set_product_prices_production_consumption();
    log_consumption_frequencies();
    for (Product * product : products) {
        Logger::log(
                Logger::SOCIETY,
                id,
                "price",
                LogPair("product_id", product->id),
                LogPair("price_per_unit", product->price_per_unit)
                );
    }
}

void Society::populate_io_matrix_and_labor_vector(
    Eigen::MatrixXd &input_output_matrix,
    Eigen::VectorXd &labor_vector) {
    const unsigned int starting_num_firms =
        Sim::get_num_producers() + Sim::get_num_distributors();
    const unsigned int average_team_size =
        std::max<unsigned int>(Sim::get_num_people() / starting_num_firms, 1);
    for (Product * output_product : products) {
        for (const std::pair<Good * const, double> &input :
             output_product->inputs_per_unit) {
            input_output_matrix(input.first->id, output_product->id) = input.second;
        }
        double machine_use_hours =
            output_product->living_labor_per_unit / average_team_size;
        for (Machine * const machine : output_product->machines_needed) {
            input_output_matrix(machine->id, output_product->id) =
                machine_use_hours / machine->lifetime;
        }
        labor_vector(output_product->id) =
            output_product->living_labor_per_unit;
    }
}

double get_max_eigenvalue(Eigen::MatrixXd &io_matrix) {
    Eigen::EigenSolver<Eigen::MatrixXd> eigen_solver(io_matrix, false);
    Eigen::VectorXcd eigenvalues = eigen_solver.eigenvalues();
    double max_eigenvalue = 0.0;
    double tol = 1e-10;
    for (size_t i = 0; i < static_cast<unsigned long>(eigenvalues.size()); ++i) {
        if (eigenvalues(i).real() > max_eigenvalue &&
            std::abs(eigenvalues(i).imag()) < tol) {
            max_eigenvalue = eigenvalues(i).real();
        }
    }
    return max_eigenvalue;
}

std::vector<Ability *>& Society::get_abilities() {
    return abilities;
}

std::vector<Ability *>& Society::get_distribution_abilities() {
    return distribution_abilities;
}

std::vector<Producer *> &Society::get_producers() {
    return producers;
}

std::vector<Producer *>& Society::get_suppliers(Product * product) {
    return product_to_suppliers[product];
}

double Society::get_busyness() {
    return busyness;
}

double Society::get_average_account() {
    return average_account;
}

double Society::get_total_employment() {
    unsigned int employed = 0;
    for (Person * person : people) {
        employed += (person->get_firm() != nullptr);
    }
    return static_cast<double>(employed) / people.size();
}

void Society::log_total_employment() {
    Logger::log(
            Logger::SOCIETY,
            id,
            "employment",
            LogPair("total", get_total_employment())
            );
}

void Society::adjust_io_matrix(Eigen::MatrixXd& io_matrix) {

    const size_t dim = io_matrix.rows();
    const size_t new_dim = dim - consumer_goods.size();

    std::vector<size_t> kept_indices;
    kept_indices.reserve(new_dim);

    for (size_t i = 0; i < products.size(); ++i) {
        if (products[i]->product_type != Product::TYPE_CONSUMER_GOOD) {
            kept_indices.push_back(products[i]->id);
        }
    }

    Eigen::MatrixXd productive_matrix(new_dim, new_dim);

    for (size_t i = 0; i < new_dim; ++i) {
        for (size_t j = 0; j < new_dim; ++j) {
            productive_matrix(i, j) =
                io_matrix(kept_indices[i], kept_indices[j]);
        }
    }

    double divisor = get_max_eigenvalue(productive_matrix) / Sim::get_difficulty_of_production();

    for (size_t i = 0; i < new_dim; ++i) {
        for (size_t j = 0; j < new_dim; ++j) {
            io_matrix(kept_indices[i], kept_indices[j]) /= divisor;
        }
    }

    for (std::size_t j = 0; j < dim; ++j) {
        for (std::pair<Good * const, double>& input : products[j]->inputs_per_unit) {
            input.second = io_matrix(input.first->id, j);
        }
    }
}

Eigen::MatrixXd get_leontief_inverse(
    Eigen::MatrixXd io_matrix) {
    const std::size_t dim = io_matrix.rows();
    Eigen::MatrixXd identity_matrix = Eigen::MatrixXd::Identity(dim, dim);
    Eigen::MatrixXd leontief_matrix = identity_matrix - io_matrix;
    return leontief_matrix.inverse();
}

static void normalize_consumption_frequencies(
        std::vector<ConsumerGood *>& consumer_goods
        ) {
    double value_consumed_per_hour = 0.0;
    for (ConsumerGood * consumer_good : consumer_goods) {
        value_consumed_per_hour += consumer_good->labor_value *
            consumer_good->mean_consumption_frequency;
    }
    double worked_proportion_of_week =
        static_cast<double>(Society::get_instance()->get_current_work_hours_daily()) * Society::get_instance()->get_current_work_days_weekly() / WEEK;
    double consumption_scalar = Sim::get_product_consumption_mult()
        * worked_proportion_of_week
        / value_consumed_per_hour;
    for (ConsumerGood * consumer_good : consumer_goods) {
        consumer_good->mean_consumption_frequency *= consumption_scalar;
    }
}

void Society::set_product_prices_production_consumption() {
    const size_t dim = products.size();
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(dim, dim);
    Eigen::VectorXd l = Eigen::VectorXd::Zero(dim);
    populate_io_matrix_and_labor_vector(A, l);
    adjust_io_matrix(A);
    log_io_matrix(A, dim);
    log_vector(l, "l", dim);
    Eigen::MatrixXd leontief_inverse = get_leontief_inverse(A);
    Eigen::VectorXd values = leontief_inverse.transpose() * l;
    for (std::size_t i = 0; i < dim; ++i) {
        if (values(i) <= 0.0) {
            std::stringstream message;
            message << "Value of item " << i << " <= 0.";
            throw std::domain_error(message.str());
        }
        products[i]->labor_value = values(i);
    }
    normalize_consumption_frequencies(consumer_goods);
    set_initial_prices(A, l, consumer_goods, dim);
    Eigen::VectorXd demands = Eigen::VectorXd::Zero(dim);
    for (ConsumerGood * consumer_good : consumer_goods) {
        demands[consumer_good->id] = consumer_good->mean_consumption_frequency;
    }
    Eigen::VectorXd production = leontief_inverse * demands;
    for (std::size_t i = 0; i < dim; ++i) {
        initial_production[products[i]] = production(i);
    }
}

static Eigen::VectorXd get_epr_prices(const Eigen::MatrixXd& M, double spectral_radius) {
    Eigen::EigenSolver<Eigen::MatrixXd> eigen_solver(
        M.transpose()
    );

    double tol = 1e-8;
    Eigen::VectorXcd eigenvalues = eigen_solver.eigenvalues();
    Eigen::MatrixXcd eigenvectors = eigen_solver.eigenvectors();

    std::vector<Eigen::VectorXd> likely_perron_evecs;

    for (Eigen::Index i = 0; i < eigenvalues.size(); ++i) {
        if (std::abs(eigenvalues(i).real() - spectral_radius) < tol) {
            likely_perron_evecs.push_back(eigenvectors.col(i).real());
        }
    }

    Eigen::MatrixXd V(M.rows(), likely_perron_evecs.size());

    for (std::size_t j = 0; j < likely_perron_evecs.size(); ++j) {
        V.col(j) = likely_perron_evecs[j];
    }

    Eigen::Index n = V.rows();
    Eigen::Index k = V.cols();

    if (k == 1) {
        Eigen::VectorXd p = V.col(0);

        if (p.minCoeff() < -tol) {
            p *= -1.0;
        }
        if (p.minCoeff() < -tol) {
            throw std::runtime_error("Perron root is not non-negative!");
        }

        for (Eigen::Index i = 0; i < n; ++i) {
            if (p(i) < 0.0) {
                p(i) = 0.0;
            }
        }

        p /= p.norm();
        return p;
    } 
    throw MultidimensionalPerronEigenspace();
}

static Eigen::VectorXd retry_with_perturbation(
    const Eigen::MatrixXd& A,
    const Eigen::VectorXd& l,
    Eigen::VectorXd& b) {
    std::vector<int> positive_indices;
    for (int i = 0; i < b.size(); ++i) {
        if (b(i) > 0.0) {
            positive_indices.push_back(i);
        }
    }
    if (positive_indices.empty()) {
        throw std::domain_error("Can't perturb b: no positive entries. Giving up.");
    }

    std::uniform_int_distribution<> index_dist(
        0,
        static_cast<int>(positive_indices.size()) - 1
    );

    int chosen_position = index_dist(Sim::get_random_generator());
    int i = positive_indices[chosen_position];

    b(i) *= 1.0 + PRICE_B_PERTURBATION_EPSILON;
    Eigen::MatrixXd new_augmented_matrix = A+l*b.transpose();
    double new_spectral_radius = get_max_eigenvalue(new_augmented_matrix);
    if (new_spectral_radius >= 1.0) {
        throw std::domain_error("Perturbation destroyed productivity of augmented matrix. Giving up.");
    }
    Eigen::VectorXd price_candidate = get_epr_prices(
        new_augmented_matrix,
        new_spectral_radius
    );
    return price_candidate;
}

void Society::set_initial_prices(
        const Eigen::MatrixXd& A,
        const Eigen::VectorXd& l,
        const std::vector<ConsumerGood *>& consumer_goods,
        std::size_t dim 
        ) {
    std::string price_mode = Sim::get_initial_price_mode();
    bool use_labor_values = false;

    if (price_mode == "equilibrium_prices") {
        Eigen::VectorXd price_candidate;
        try {
            Eigen::VectorXd b = Eigen::VectorXd::Zero(dim);

            for (ConsumerGood * consumer_good : consumer_goods) {
                b(consumer_good->id) = consumer_good->mean_consumption_frequency;
            }

            Eigen::MatrixXd augmented_matrix = A+l*b.transpose();
            double spectral_radius = get_max_eigenvalue(augmented_matrix);

            int reductions = 0;
            constexpr int MAX_CONSUMPTION_REDUCTIONS = 10000;
            constexpr double SPECTRAL_RADIUS_TOL = 1e-10;

            double d = Sim::get_difficulty_of_production();
            double r = std::min(spectral_radius, 1.0);
            double p = std::clamp(DESIRED_INITIAL_PROFITABILITY, 0.0, 1.0);
            double target = (d-r)*p + r;
            while (spectral_radius > target + SPECTRAL_RADIUS_TOL) {
                b *= CONSUMP_REDUCTION_FACTOR;

                augmented_matrix = A+l*b.transpose();
                spectral_radius = get_max_eigenvalue(augmented_matrix);
                
                ++reductions;

                if (reductions > MAX_CONSUMPTION_REDUCTIONS) {
                    std::stringstream message;
                    message << "Unable to derive a suitable capitalist vector of equilibrium prices.";
                    throw std::domain_error(message.str());
                }
            }
            std::size_t max_retries = dim;
            bool success = false;
            try {
                price_candidate = get_epr_prices(augmented_matrix, spectral_radius);
                success = true;
            }
            catch (const MultidimensionalPerronEigenspace& e) {
                for (std::size_t attempt = 0; attempt < max_retries; ++attempt) {
                    try {
                        price_candidate = retry_with_perturbation(A, l, b);
                        success = true;
                        break;
                    }
                    catch (const MultidimensionalPerronEigenspace& e) {}
                }
            }
            if (success == true) {
                for (std::size_t i = 0; i < dim; ++i) {
                    if (price_candidate(i) <= 0.0) {
                        std::stringstream message;
                        message << "Price of item " << i << " <= 0.";
                        throw std::domain_error(message.str());
                    }
                    products[i]->price_per_unit = price_candidate(i);
                }
                log_vector(b, "b", b.size());
                return;
            } else {
                Logger::log(
                    Logger::SOCIETY,
                    id,
                    "initial_price_fallback",
                    LogPairS("requested_mode", price_mode),
                    LogPairS("reason", "Multidimensional eigenspace would not budge.")
                );
                use_labor_values = true;
            }
        }
        catch (const std::exception& e) {
            Logger::log(
                Logger::SOCIETY,
                id,
                "initial_price_fallback",
                LogPairS("requested_mode", price_mode),
                LogPairS("reason", e.what())
            );
            use_labor_values = true;
        }
    }
    else {
        use_labor_values = true;
        if (price_mode != "labor_values") {
            Logger::log(
                Logger::SOCIETY,
                id,
                "initial_price_fallback",
                LogPairS("requested_mode", price_mode),
                LogPairS("reason", "Unknown price mode specified")
            );
        }
    }
    if (use_labor_values) {
        for (std::size_t i = 0; i < dim; ++i) {
            products[i]->price_per_unit = products[i]->labor_value;
        }
    }
}



std::vector<Product *>& Society::get_products() {
    return products;
}

std::vector<Good *>& Society::get_goods() {
    return goods;
}

std::vector<ConsumerGood *>& Society::get_consumer_goods() {
    return consumer_goods;
}

std::vector<Machine *>& Society::get_machines() {
    return machines;
}

double Society::get_underlying_living_labor_per_unit(Product * product) {
    return underlying_living_labor_per_unit[product];
}

void Society::set_underlying_living_labor_per_unit(Product * product, double ll) {
    underlying_living_labor_per_unit[product] = ll;
}

std::vector<Firm *>& Society::get_firms() {
    return firms;
}

std::vector<Distributor *> &Society::get_distributors() {
    return distributors;
}

std::unordered_set<Person *>& Society::get_unemployed_people() {
    return unemployed_people;
}

unsigned int Society::get_current_work_hours_daily() {
    return current_work_hours_daily;
}

unsigned int Society::get_current_work_days_weekly() {
	return current_work_days_weekly;
}

int Society::get_initial_account() {
    return initial_account;
}

std::unordered_map<Product *, double>& Society::get_initial_production() {
    return initial_production;
}

Person * Society::birth_person() {
    Person * person = new Person;
    people.push_back(person);
    unemployed_people.insert(person);
    return person;
}

void Society::retire_person(Person *person) {
    // unimplemented until hiring/reallocation is done
}

void Society::pay_into_public_fund(double amount) {
    public_fund += amount;
    log_public_revenue(amount);
    log_public_fund();
}

void Society::charge_from_public_fund(double amount) {
    public_fund -= amount;
    log_public_expenditure(amount);
    log_public_fund();
}

void Society::check_expand_public_sector() {
    if (Sim::get_public_sector_expansion_period() <= 0) {
        return;
    }
    if (Sim::get_current_time_step() % Sim::get_public_sector_expansion_period() == 0) {
        int index = Sim::get_current_time_step() / Sim::get_public_sector_expansion_period() - 1;
        if (index < 0 || index >= static_cast<int>(consumer_goods.size())) {
            return;
        }
        ConsumerGood * consumer_good = consumer_goods[index];
        consumer_good->public_sector = true;
        log_public_sector_expansion(consumer_good);
    }
}

void Society::check_sample_busyness_data() {
    if (Sim::get_current_time_step() % BUSYNESS_AVERAGING_WINDOW == 0
        && Sim::get_current_time_step() > 0) {
        for (Person * person : people) {
            busyness_data.push_back(person->get_busyness());
        }
    }
}

static double normal_pdf(
    double z) {
    double exponent = -0.5 * std::pow(z, 2);
    return std::exp(exponent) / std::sqrt(2.0 * M_PI);
}

static double normal_cdf(double z) {
    return 0.5 + 0.5 * std::erf(z / std::sqrt(2.0));
}

static double inv_normal_cdf(double p) {
    if (p <= 0.0 || p >= 1.0) {
        throw std::invalid_argument("Probability must be in the open interval (0, 1).");
    }
    double z_min = -10.0;
    double z_max = 10.0;
    while (z_max - z_min > 1e-6) {
        double z_mid = 0.5 * (z_min + z_max);
        double cdf_mid = normal_cdf(z_mid);
        if (cdf_mid < p) {
            z_min = z_mid;
        } else {
            z_max = z_mid;
        }
    }
    return z_min;
}

static double uncensored_log_likelihood(
    const std::vector<double>& data,
    double upper_bound,
    double delta,
    double gamma) {
    double log_likelihood = 0.0;
    for (double x : data) {
        if (x < upper_bound * MAXIMUM_BUSYNESS_CONSIDERATION_BUFFER) {
            log_likelihood += std::log(gamma) + std::log(normal_pdf(gamma * x - delta));
        } else {
            log_likelihood += std::log(normal_cdf(delta - gamma * upper_bound));
        }
    }
    return log_likelihood;
}

static std::pair<double, double> predict_uncensored_distribution(
    const std::vector<double>& data,
    double upper_bound) {
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double variance = 0.0;
    for (double x : data) {
        variance += (x - mean) * (x - mean);
    }
    variance /= data.size();
    double stddev = std::sqrt(variance);
    std::cout << "original mean: " << mean << ", stddev: " << stddev << std::endl;
    return {mean, stddev};
    double gamma = 1 / stddev;
    double delta = mean * gamma;
    double d_gamma = 0.005;
    double d_delta = 0.005;
    double dd_gamma = 0.0;
    double dd_delta = 0.0;
    double alpha = 0.005;
    do {
        double dd_delta = (uncensored_log_likelihood(data, upper_bound, delta + d_delta, gamma) 
            - uncensored_log_likelihood(data, upper_bound, delta, gamma)) / d_delta;
        double dd_gamma = (uncensored_log_likelihood(data, upper_bound, delta, gamma + d_gamma) 
            - uncensored_log_likelihood(data, upper_bound, delta, gamma)) / d_gamma;
        delta += alpha * dd_delta;
        gamma += alpha * dd_gamma;
        gamma = std::max(gamma, 1e-6);
    } while (std::abs(dd_delta) > 1e-6 || std::abs(dd_gamma) > 1e-6);
    double predicted_mean = delta / gamma;
    double predicted_stddev = 1 / gamma;
    return {predicted_mean, predicted_stddev};
}

void Society::check_update_work_hours() {
    if (Sim::get_current_time_step() % WORK_HOURS_UPDATE_PERIOD == 0
        && Sim::get_current_time_step() > 0) {
        std::pair<double, double> predicted_distribution = predict_uncensored_distribution(
            busyness_data,
            static_cast<double>(current_work_hours_daily) * current_work_days_weekly / WEEK
        );
        std::cout << "z-score: " << inv_normal_cdf(WORK_HOURS_UPDATE_CONFIDENCE) << std::endl;
        double confident_lower_bound = 
            predicted_distribution.first + inv_normal_cdf(WORK_HOURS_UPDATE_CONFIDENCE) * predicted_distribution.second;
        current_work_hours_daily = 
            std::ceil(confident_lower_bound * WEEK / current_work_days_weekly);
        current_work_hours_daily = std::min(current_work_hours_daily, DAY);
        log_busyness_data();
        log_predicted_uncensored_busyness_distribution(predicted_distribution);
        log_work_hours_weekly();
        busyness_data.clear();
    }
}

void Society::log_io_matrix(Eigen::MatrixXd& A, size_t dim) {
    for (size_t i = 0; i < dim; ++i) {
        for (size_t j = 0; j < dim; ++j) {
            if (A(i, j)) {
                Logger::log(
                        Logger::SOCIETY,
                        id,
                        "A",
                        LogPairS(
                            "coords",
                            std::string("(") + std::to_string(i) +"," +
                            std::to_string(j) + ")"
                            ),
                        LogPair(
                            "value",
                            A(i, j)
                            )
                        );
            }
        }
    }
}

void Society::log_vector(Eigen::VectorXd& v, std::string name, size_t dim) {
    for (size_t i = 0; i < dim; ++i) {
        Logger::log(
                Logger::SOCIETY,
                id,
                name,
                LogPair("prod_id", i),
                LogPair("value", v(i))
                );
    }
}

void Society::log_consumption_frequencies() {
    for (const ConsumerGood * consumer_good : consumer_goods) {
        Logger::log(
                Logger::SOCIETY,
                id,
                "mean_consumption_frequency",
                LogPair("product_id", consumer_good->id),
                LogPair("value", consumer_good->mean_consumption_frequency)
                );
    }
}

void Society::log_public_fund() {
    Logger::log(
            Logger::SOCIETY,
            id,
            "public_fund",
            LogPair("value", public_fund)
            );
}

void Society::log_public_revenue(double revenue) {
    Logger::log(
            Logger::SOCIETY,
            id,
            "public_revenue",
            LogPair("value", revenue)
            );
}

void Society::log_public_expenditure(double expenditure) {
    Logger::log(
            Logger::SOCIETY,
            id,
            "public_expenditure",
            LogPair("value", expenditure)
            );
}

void Society::log_public_sector_expansion(ConsumerGood * consumer_good) {
    Logger::log(
            Logger::SOCIETY,
            id,
            "public_sector_expansion",
            LogPair("product_id", consumer_good->id)
            );
}

void Society::log_busyness_data() {
    for (double busyness_value : busyness_data) {
        Logger::log(
                Logger::SOCIETY,
                id,
                "busyness_data",
                LogPair("dataset", Sim::get_current_time_step() / WORK_HOURS_UPDATE_PERIOD),
                LogPair("value", busyness_value)
                );
    }
}

void Society::log_predicted_uncensored_busyness_distribution(std::pair<double, double> predicted_distribution) {
    std::cout << "predicted uncensored busyness distribution: mean = "
              << predicted_distribution.first
              << ", stddev = "
              << predicted_distribution.second
              << std::endl;
    Logger::log(
            Logger::SOCIETY,
            id,
            "predicted_uncensored_busyness_distribution",
            LogPair("mean", predicted_distribution.first),
            LogPair("stddev", predicted_distribution.second)
            );
}

void Society::log_work_hours_weekly() {
    std::cout << "new work hours daily: " << current_work_hours_daily << std::endl;
    Logger::log(
            Logger::SOCIETY,
            id,
            "work_hours_weekly",
            LogPair("work_hours_daily", current_work_hours_daily),
            LogPair("work_days_weekly", current_work_days_weekly)
            );
}
