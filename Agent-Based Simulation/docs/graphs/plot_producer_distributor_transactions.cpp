#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <matplot/matplot.h>

namespace fs = std::filesystem;

constexpr int STARTING_NUM_PRODUCERS = 100;
constexpr int STARTING_NUM_DISTRIBUTORS = 100;
constexpr int DIST_ID_MIN = STARTING_NUM_PRODUCERS;
constexpr int DIST_ID_MAX = STARTING_NUM_PRODUCERS + STARTING_NUM_DISTRIBUTORS - 1;

const std::regex RE_REORDER(
    R"(^\[(\d+)\]\s+Firm_(\d+):\s+reorder - Product (\d+)\s+(\d+)\s*$)");
const std::regex RE_PURSUED(
    R"(^\[(\d+)\]\s+Producer_(\d+):\s+pursued_plan - Product (\d+)\s+(\d+)\s*$)");
const std::regex RE_ACCEPTED_ORDER(
    R"(^\[(\d+)\]\s+Firm_(\d+):\s+accepted order - Product (\d+)\s+(\d+)\s*$)");

using TransactionKey = std::tuple<int, int, int>;  // time, product, qty

struct TransactionRecord {
    int time_step{};
    int distributor_id{};
    int producer_id{};
    int product_id{};
    int quantity{};
};

struct ParseResult {
    std::vector<TransactionRecord> records;
    int unmatched_reorders{};
    int unmatched_pursued{};
    std::map<int, long long> producer_produced_qty_by_time;
    std::map<int, long long> producer_production_count_by_time;
    std::map<int, long long> distributor_sold_qty_by_time;
    std::map<int, long long> distributor_sale_count_by_time;
};

ParseResult parse_trace_transactions(const fs::path &trace_path) {
    ParseResult result;
    std::ifstream in(trace_path);
    if (!in.is_open()) {
        throw std::runtime_error("Unable to open trace file: " + trace_path.string());
    }

    std::map<TransactionKey, std::deque<int>> distributor_reorders;
    std::map<TransactionKey, std::deque<int>> producer_pursued;

    std::string line;
    std::smatch match;
    while (std::getline(in, line)) {
        if (std::regex_match(line, match, RE_REORDER)) {
            const int t = std::stoi(match[1].str());
            const int firm_id = std::stoi(match[2].str());
            const int product = std::stoi(match[3].str());
            const int qty = std::stoi(match[4].str());
            if (firm_id >= DIST_ID_MIN && firm_id <= DIST_ID_MAX) {
                distributor_reorders[{t, product, qty}].push_back(firm_id);
            }
            continue;
        }

        if (std::regex_match(line, match, RE_PURSUED)) {
            const int t = std::stoi(match[1].str());
            const int producer = std::stoi(match[2].str());
            const int product = std::stoi(match[3].str());
            const int qty = std::stoi(match[4].str());
            producer_pursued[{t, product, qty}].push_back(producer);
            result.producer_produced_qty_by_time[t] += qty;
            result.producer_production_count_by_time[t] += 1;
            continue;
        }

        if (std::regex_match(line, match, RE_ACCEPTED_ORDER)) {
            const int t = std::stoi(match[1].str());
            const int firm_id = std::stoi(match[2].str());
            const int qty = std::stoi(match[4].str());
            if (firm_id >= DIST_ID_MIN && firm_id <= DIST_ID_MAX) {
                result.distributor_sold_qty_by_time[t] += qty;
                result.distributor_sale_count_by_time[t] += 1;
            }
        }
    }

    std::set<TransactionKey> all_keys;
    for (const auto &[k, _] : distributor_reorders) {
        all_keys.insert(k);
    }
    for (const auto &[k, _] : producer_pursued) {
        all_keys.insert(k);
    }

    for (const auto &key : all_keys) {
        auto rq = distributor_reorders[key];
        auto pq = producer_pursued[key];
        const int matches = static_cast<int>(std::min(rq.size(), pq.size()));
        const int t = std::get<0>(key);
        const int product = std::get<1>(key);
        const int qty = std::get<2>(key);

        for (int i = 0; i < matches; ++i) {
            result.records.push_back(TransactionRecord{
                t, rq.front(), pq.front(), product, qty,
            });
            rq.pop_front();
            pq.pop_front();
        }
        result.unmatched_reorders += static_cast<int>(rq.size());
        result.unmatched_pursued += static_cast<int>(pq.size());
    }

    std::sort(result.records.begin(), result.records.end(),
              [](const TransactionRecord &a, const TransactionRecord &b) {
                  if (a.time_step != b.time_step) return a.time_step < b.time_step;
                  if (a.distributor_id != b.distributor_id) return a.distributor_id < b.distributor_id;
                  if (a.producer_id != b.producer_id) return a.producer_id < b.producer_id;
                  return a.product_id < b.product_id;
              });

    return result;
}

void write_transactions_csv(const std::vector<TransactionRecord> &records, const fs::path &out_csv) {
    fs::create_directories(out_csv.parent_path());
    std::ofstream out(out_csv);
    if (!out.is_open()) {
        throw std::runtime_error("Unable to write CSV: " + out_csv.string());
    }

    out << "time_step,distributor_id,producer_id,product_id,quantity\n";
    for (const auto &r : records) {
        out << r.time_step << ","
            << r.distributor_id << ","
            << r.producer_id << ","
            << r.product_id << ","
            << r.quantity << "\n";
    }
}

void plot_time_series(const ParseResult &parsed, const fs::path &out_png) {
    if (parsed.producer_produced_qty_by_time.empty() &&
        parsed.distributor_sold_qty_by_time.empty()) {
        using namespace matplot;
        auto fig = figure(true);
        fig->size(1200, 700);
        plot({0.0}, {0.0});
        title("No producer/distributor accepted-order events found in trace");
        xlabel("Time step");
        ylabel("Quantity traded");
        grid(on);
        save(out_png.string());
        return;
    }

    std::set<int> all_times_set;
    for (const auto &[t, _] : parsed.producer_produced_qty_by_time) all_times_set.insert(t);
    for (const auto &[t, _] : parsed.distributor_sold_qty_by_time) all_times_set.insert(t);

    std::vector<double> times;
    std::vector<double> producer_produced_quantities;
    std::vector<double> distributor_sold_quantities;
    std::vector<double> producer_production_counts;
    std::vector<double> distributor_sale_counts;
    times.reserve(all_times_set.size());
    producer_produced_quantities.reserve(all_times_set.size());
    distributor_sold_quantities.reserve(all_times_set.size());
    producer_production_counts.reserve(all_times_set.size());
    distributor_sale_counts.reserve(all_times_set.size());

    for (const int t : all_times_set) {
        const auto produced_qty_it = parsed.producer_produced_qty_by_time.find(t);
        const auto sold_qty_it = parsed.distributor_sold_qty_by_time.find(t);
        const auto produced_count_it = parsed.producer_production_count_by_time.find(t);
        const auto sold_count_it = parsed.distributor_sale_count_by_time.find(t);
        times.push_back(static_cast<double>(t));
        producer_produced_quantities.push_back(
            static_cast<double>(produced_qty_it != parsed.producer_produced_qty_by_time.end()
                                    ? produced_qty_it->second
                                    : 0));
        distributor_sold_quantities.push_back(
            static_cast<double>(sold_qty_it != parsed.distributor_sold_qty_by_time.end()
                                    ? sold_qty_it->second
                                    : 0));
        producer_production_counts.push_back(
            static_cast<double>(produced_count_it != parsed.producer_production_count_by_time.end()
                                    ? produced_count_it->second
                                    : 0));
        distributor_sale_counts.push_back(
            static_cast<double>(sold_count_it != parsed.distributor_sale_count_by_time.end()
                                    ? sold_count_it->second
                                    : 0));
    }

    using namespace matplot;
    auto fig = figure(true);
    fig->size(1200, 700);

    subplot(2, 1, 0);
    auto p1 = plot(times, producer_produced_quantities);
    p1->line_width(1.8);
    p1->display_name("Producer produced quantity");
    hold(on);
    auto p2 = plot(times, distributor_sold_quantities);
    p2->line_width(1.8);
    p2->display_name("Distributor accepted order quantity");
    hold(off);
    title("Producer Produced vs Distributor Accepted Order Quantity");
    xlabel("Time step");
    ylabel("Quantity traded");
    grid(on);
    legend();

    subplot(2, 1, 1);
    auto p3 = plot(times, producer_production_counts);
    p3->line_width(1.6);
    p3->display_name("Producer production events");
    hold(on);
    auto p4 = plot(times, distributor_sale_counts);
    p4->line_width(1.6);
    p4->display_name("Distributor accepted order events");
    hold(off);
    title("Producer Production Events vs Distributor Accepted Order Events");
    xlabel("Time step");
    ylabel("Number of transactions");
    grid(on);
    legend();

    save(out_png.string());
}

std::vector<int> top_ids_by_total(const std::map<int, long long> &totals, std::size_t n) {
    std::vector<std::pair<int, long long>> items(totals.begin(), totals.end());
    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    if (items.size() > n) {
        items.resize(n);
    }
    std::vector<int> ids;
    ids.reserve(items.size());
    for (const auto &[id, _] : items) {
        ids.push_back(id);
    }
    return ids;
}

void plot_pair_heatmap(const std::vector<TransactionRecord> &records, const fs::path &out_png, std::size_t top_n) {
    std::map<std::pair<int, int>, long long> tx_count_by_pair;
    std::map<int, long long> dist_totals;
    std::map<int, long long> prod_totals;

    for (const auto &r : records) {
        tx_count_by_pair[{r.distributor_id, r.producer_id}] += 1;
        dist_totals[r.distributor_id] += 1;
        prod_totals[r.producer_id] += 1;
    }
    if (tx_count_by_pair.empty()) {
        return;
    }

    const auto top_distributors = top_ids_by_total(dist_totals, top_n);
    const auto top_producers = top_ids_by_total(prod_totals, top_n);

    std::map<int, std::size_t> d_index;
    std::map<int, std::size_t> p_index;
    for (std::size_t i = 0; i < top_distributors.size(); ++i) {
        d_index[top_distributors[i]] = i;
    }
    for (std::size_t i = 0; i < top_producers.size(); ++i) {
        p_index[top_producers[i]] = i;
    }

    std::vector<std::vector<double>> matrix(
        top_distributors.size(), std::vector<double>(top_producers.size(), 0.0));
    for (const auto &[pair_key, tx_count] : tx_count_by_pair) {
        const int d = pair_key.first;
        const int p = pair_key.second;
        const auto d_it = d_index.find(d);
        const auto p_it = p_index.find(p);
        if (d_it != d_index.end() && p_it != p_index.end()) {
            matrix[d_it->second][p_it->second] = static_cast<double>(tx_count);
        }
    }

    std::vector<double> x_ticks(top_producers.size());
    std::iota(x_ticks.begin(), x_ticks.end(), 0.0);
    std::vector<double> y_ticks(top_distributors.size());
    std::iota(y_ticks.begin(), y_ticks.end(), 0.0);

    std::vector<std::string> x_labels;
    std::vector<std::string> y_labels;
    x_labels.reserve(top_producers.size());
    y_labels.reserve(top_distributors.size());
    for (const int p : top_producers) x_labels.push_back(std::to_string(p));
    for (const int d : top_distributors) y_labels.push_back(std::to_string(d));

    using namespace matplot;
    auto fig = figure(true);
    fig->size(1200, 850);
    imagesc(matrix);
    colormap(palette::viridis());
    colorbar();
    title("Producer-Distributor Interaction Heatmap (Transaction Count)");
    xlabel("Producer ID (top by interactions)");
    ylabel("Distributor ID (top by interactions)");
    xticks(x_ticks);
    yticks(y_ticks);
    xticklabels(x_labels);
    yticklabels(y_labels);
    xtickangle(45);
    save(out_png.string());
}

void write_summary(const fs::path &summary_file, const ParseResult &result) {
    std::ofstream out(summary_file);
    if (!out.is_open()) {
        throw std::runtime_error("Unable to write summary: " + summary_file.string());
    }
    out << "matched_transactions=" << result.records.size() << "\n";
    out << "unmatched_distributor_reorders=" << result.unmatched_reorders << "\n";
    out << "unmatched_producer_pursued_plans=" << result.unmatched_pursued << "\n";
}

int main(int argc, char *argv[]) {
    try {
        const fs::path trace_file = (argc > 1) ? fs::path(argv[1]) : fs::path("trace.txt");
        const fs::path out_dir = (argc > 2) ? fs::path(argv[2]) : fs::path("docs") / "graphs";
        fs::create_directories(out_dir);

        if (!fs::exists(trace_file)) {
            throw std::runtime_error("Trace file not found: " + trace_file.string());
        }

        const ParseResult parsed = parse_trace_transactions(trace_file);
        write_transactions_csv(parsed.records, out_dir / "producer_distributor_transactions.csv");
        plot_time_series(parsed, out_dir / "producer_distributor_transactions_time_series.png");
        plot_pair_heatmap(parsed.records, out_dir / "producer_distributor_transactions_heatmap.png", 20);
        write_summary(out_dir / "producer_distributor_transactions_summary.txt", parsed);

        std::cout << "Wrote outputs to: " << out_dir << "\n";
        std::cout << "Matched transactions: " << parsed.records.size() << "\n";
        std::cout << "Unmatched distributor reorders: " << parsed.unmatched_reorders << "\n";
        std::cout << "Unmatched producer pursued plans: " << parsed.unmatched_pursued << "\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
