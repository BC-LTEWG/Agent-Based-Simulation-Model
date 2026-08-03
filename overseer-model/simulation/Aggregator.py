import logging
logger = logging.getLogger(__name__)
import numpy as np
from copy import deepcopy
from .Collector import Collector
from typing import Tuple
from scipy.linalg import inv, eig
from scipy.stats import norm
from pathlib import Path
import math
import copy
from overseer.tools.dataclasses import Replace, Extend, Append, Update

np.set_printoptions(
    precision=3,      # digits after decimal-ish
    suppress=True,    # avoid scientific notation when possible
    linewidth=200     # avoid wrapping rows too early
)

class Aggregator:
    """ The guy who watches the LTE and keeps track of the data. """
    def __init__(self, bin_path, params):
        self.params = params

        # create a more human readable dictionary of helpful quantities
        self.settings = {
            "n_goods": params.N_g,
            "n_machines": params.N_m,
            "max_num_inputs_per_good": params.N_inputs_max,
            "n_time_steps": params.N_S,
            "n_persons": params.N_h,
            # "prods_per_machine": params.m_r,
            "fixed_seed": params.fixed_seed,
            "seed": params.seed,
            "n_producers": params.N_p,
            "n_distributors": params.N_d,
            "n_firms": params.N_p + params.N_d,
            "init_working_day": params.T,
            "init_working_week": params.W,
            "daily_sick_chance": params.S,
            "person_ability_stddev": params.v_ability,
            "n_abilities": params.N_a,
            "productivity": params.productivity,
            "consump_epsilon": params.consump_epsilon,
            # the rest of these are d if pending_inventory_j else 0ependent variables
            "n_sectors": params.N_g + params.N_m + 1,
            "n_consumer_goods": params.N_g,
            "n_products": 2 * params.N_g + params.N_m,
            "init_prices": params.init_prices,
            "free_goods": params.free_goods,
            "new_free_good_interval": params.new_free_good_interval
        }

        # create and start the collection thread
        args = self._get_args_from_settings()

        cli_cmd = bin_path + " "
        for arg in args:
            cli_cmd += arg
            cli_cmd += " "

        logger.info(f"Running binary with command: \n   {cli_cmd}")

        self.collector = Collector(bin_path, args)
        self.collector.start_sim_and_begin_collection()

        # internal stuff
        self.t = [0]
        self.current_t = 0
        self.current_week = 0
        self.stdout_done = False
        self.stderr_done = False

        self.current_cout = []

        self.fic = 0.0
        self.average_consumer_goods_value = 0.0
        self.average_public_sector_consumer_goods_value = 0.0
        self.public_fund = 0.0
        self.public_expenditure = 0.0
        self.public_revenue = 0.0

        # HERE IS WHERE YOU WOULD DECLARE ANY QUANTITIES WHICH YOU WANT THE OVERSEER TO KEEP TRACK OF
        self.prices = np.zeros(self.settings["n_products"])
        self.living_labor_values = np.zeros(self.settings["n_products"])
        self.turnover_times = [[] for n in range(self.settings["n_products"])]
        self.current_employment = 0

        self.persons = {i: {
            "account": 0,
            "endowment": np.zeros(self.settings["n_products"]),
            "abilities": np.zeros(self.settings["n_abilities"]),
            "health": "Healthy", # everyone starts in good health
            "recent_busyness": 0.0
        } for i in range(self.settings["n_persons"])}

        self.producers = {i: {
            "employees": 0,
            "inventory": np.zeros(self.settings["n_products"]),
            "pending_inventory": np.zeros(self.settings["n_products"]),
            "demand_signals": np.zeros(self.settings["n_products"]),
            "reorder_thresholds": np.zeros(self.settings["n_products"]),
            "resupply_rates": np.zeros(self.settings["n_products"]),
            "resupply_deficits": np.zeros(self.settings["n_products"]),
            "tracked_inputs": np.zeros(self.settings["n_products"], dtype= bool),
            "catalog": [],
            "recent_busyness": 0,
            "inc_inventory": np.zeros(self.settings["n_products"]),
        } for i in range(self.settings["n_producers"])}

        self.distributors = {i: {
            "employees": 0,
            "inventory": np.zeros(self.settings["n_products"]),
            "pending_inventory": np.zeros(self.settings["n_products"]),
            "demand_signals": np.zeros(self.settings["n_products"]),
            "reorder_thresholds": np.zeros(self.settings["n_products"]),
            "resupply_rates": np.zeros(self.settings["n_products"]),
            "resupply_deficits": np.zeros(self.settings["n_products"]),
            "tracked_inputs": np.zeros(self.settings["n_products"], dtype= bool),
            "catalog": [],
            "recent_busyness": 0,
            "inc_inventory": np.zeros(self.settings["n_products"]),

        } for i in range(self.settings["n_distributors"])}

        self.A = np.zeros((self.settings["n_products"], self.settings["n_products"]))
        self.l = np.zeros(self.settings["n_products"])
        self.b = np.zeros(self.settings["n_products"])
        self.consumption_frequencies = np.zeros(self.settings["n_products"])
        self.consumption_periods = np.zeros(self.settings["n_products"])
        self.order_sizes = [[] for i in range(self.settings["n_products"])]
        self.old_order_size_avgs = np.zeros(self.settings["n_products"])
        self.lead_times = [[] for i in range(self.settings["n_products"])]
        self.old_lead_time_avgs = np.zeros(self.settings["n_products"])
        self.team_sizes = [[] for i in range(self.settings["n_products"])]
        self.old_team_size_avgs = np.zeros(self.settings["n_products"])
        self.transfer_requests_by_sector = np.zeros(self.settings["n_sectors"])
        self.transfer_requests_by_sector_t = np.array([])
        self.active_plans = {i: {"plans": 0, "quantity": 0} for i in range(self.settings["n_products"])}
        self.reorder_attempts = np.zeros(self.settings["n_products"])
        self.reorder_failures = np.zeros(self.settings["n_products"])
        self.overall_busyness = 0
        self.overall_busyness_data = []
        self.overall_weekly_busyness = 0
        self.weekly_working_hours = 5*8
        self.long_run_employment_by_sector = np.zeros(self.settings["n_goods"]+self.settings["n_machines"]+1)
        self.long_run_sector_activity = np.zeros(self.settings["n_sectors"])

        self.work_hours_update_this_step = False
        self.censored_busyness_mean = None
        self.uncensored_busyness_mean = None
        self.censored_busyness_stddev = None
        self.uncensored_busyness_stddev = None
        self.individual_busyness_data = []

        self.stalled_plan_casualties = {
            product_id: set() 
            for product_id in range(self.settings["n_products"])
        }
        self.stalled_plan_causes = {
            product_id: {}
            for product_id in range(self.settings["n_products"])
        }
        self.start_plan_stall_casualties = {
            product_id: set()
            for product_id in range(self.settings["n_products"])
        }
        self.start_plan_stall_causes = {
            product_id: {}
            for product_id in range(self.settings["n_products"])
        }
        self.drafting_failures_casualties = np.zeros(
            self.settings["n_products"]
        )
        self.drafting_failures_causes_resources = np.zeros(
            self.settings["n_products"]
        )
        self.drafting_failures_casualties_from_workers = np.zeros(
            self.settings["n_products"]
        )

    def _process_dic(self, dic):
        """ 
        Looks at the contents of a json logged dictionary and updates the relevant quantities accordingly
        """
        id = dic["id"]
        client = dic.get("client", "")
        label = dic.get("label", "")
        values = dic.get("values", [])

        if label == "text_log":
            self.log_text(dic)
            return

        match client:
            case "Simulation":
                if label == "random_seed":
                    self.seed = dic["value"]

            case "Society":
                if label == "A":
                    coords_str = dic["coords"]
                    coords = tuple(int(num) for num in coords_str[1:len(coords_str)-1].split(","))
                    i, j = coords
                    a_ij = dic["value"]
                    self.A[i][j] = a_ij

                if label == "l":
                    i = dic["prod_id"]
                    l_i = dic["value"]

                    self.l[i] = l_i

                if label == "b":
                    i = dic["prod_id"]
                    b_i = dic["value"]
                    self.b[i] = b_i

                if label == "price":
                    id = dic["product_id"]
                    val = dic["price_per_unit"]
                    self.prices[id] = val

                if label == "new_price":
                    prod_id = dic["product_id"]
                    living_labor_per_unit = dic["living_labor_per_unit"]
                    price = dic["price"]
                    self.prices[prod_id] = price
                    self.living_labor_values[prod_id] = living_labor_per_unit

                if label == "mean_consumption_frequency":
                    id = dic["product_id"]
                    val = dic["value"]
                    self.consumption_frequencies[id] = val
                    self.consumption_periods[id] = 1/max(val, 1e-5)

                if label == "employment":
                    self.current_employment = dic["total"]

                if label == "societal_busyness":
                    self.overall_busyness = dic["value"]

                if label == "fic":
                    self.fic = dic["value"]

                if label == "public_sector_distribution_value":
                    self.average_public_sector_consumer_goods_value = dic["value"]

                if label == "all_distribution_value":
                    self.average_consumer_goods_value = dic["value"]

                if label == "public_fund":
                    self.public_fund = dic["value"]

                if label == "public_expenditure":
                    self.public_expenditure = dic["value"]

                if label == "public_revenue":
                    self.public_revenue = dic["value"]

                if label == "work_hours_weekly":
                    self.weekly_working_hours = dic["work_hours_daily"] * dic["work_days_weekly"]
                    self.busy_upper_bd = self.weekly_working_hours / 24 / 7

                if label == "censored_busyness_distribution":
                    self.work_hours_update_this_step = True
                    self.censored_busyness_mean = dic["mean"]
                    self.censored_busyness_stddev = dic["stddev"]
                    logger.info(f"Received censored busyness update: mean = {self.censored_busyness_mean}, stddev = {self.censored_busyness_stddev}")

                if label == "predicted_uncensored_busyness_distribution":
                    self.work_hours_update_this_step = True
                    self.uncensored_busyness_mean = dic["mean"]
                    self.uncensored_busyness_stddev = dic["stddev"]

                if label == "work_hours_weekly":
                    self.work_hours_update_this_step = True
                    work_hours_daily = dic["work_hours_daily"]
                    work_days_weekly = dic["work_days_weekly"]
                    self.busy_upper_bd = (work_hours_daily * work_days_weekly) / (24*7)

                if label == "busyness_data":
                    self.individual_busyness_data.append(dic["value"])

            case "Person":
                if label == "age":
                    self.persons[id]["health"] = values[0]

                if label == "account":
                    self.persons[id]["account"] = dic["value"]

                if label == "health_status":
                    self.persons[id]["health"] = dic["status"]

                if label == "consumption":
                    prod_id = dic["product_id"]
                    amt = dic["quantity"]
                    self.persons[id]["endowment"][prod_id] -= amt

                if label == "ability":
                    ability_id = dic["ability"]
                    val = dic["value"]
                    person_id = dic['id']
                    self.persons[person_id]["abilities"][ability_id] = val

                if label == "inventory":
                    prod_id = dic["product_id"]
                    amt = dic["amount"]
                    person_id = dic['id']
                    self.persons[person_id]["endowment"][prod_id] = amt

                if label == "purchase":
                    prod_id = dic["product_id"]
                    amt = dic["quantity"]
                    self.persons[id]["endowment"][prod_id] += amt
                    cost = self.prices[prod_id]*amt
                    self.persons[id]["account"] -= cost

                if label == "hours_worked":
                    self.persons[id]["account"] += dic["hours"]

            case "Producer":
                if label == "inventory_level":
                    prod_id = dic["product_id"]
                    amt = dic["amount"]
                    producer_id = dic['id']
                    self.producers[producer_id]["inventory"][prod_id] = amt

                if label == "inventory_reduction":
                    prod_id = dic["product_id"]
                    amt = dic["amount"]
                    producer_id = dic["id"]
                    self.producers[producer_id]["inventory"][prod_id] -= amt
                    # distributor_inventories[id-n_producers][prod_id] -= amt

                if label == "catalog_addition":
                    product_id = dic["product_id"]
                    self.producers[id]["catalog"].append(product_id)

                if label == "pursued_plan":
                    producer_id = id
                    customer_id = dic["customer_id"]
                    is_distributor = (customer_id >= self.settings["n_producers"])
                    if is_distributor:
                        customer_id = self._get_dist_key(customer_id)
                    prod_id = dic["product_id"]
                    sector_id = self.get_sector_idx(prod_id)
                    quantity = dic["quantity"]
                    lead_time = dic.get("lead_time", 0)
                    team_size = dic["num_workers"]

                    self.active_plans[prod_id]["plans"] += 1
                    self.active_plans[prod_id]["quantity"] += quantity
                    self.lead_times[prod_id].append(lead_time)
                    self.team_sizes[prod_id].append(team_size)
                    self.order_sizes[prod_id].append(quantity)
                    self.long_run_sector_activity[sector_id] += quantity

                    if is_distributor:
                        self.distributors[customer_id]["inc_inventory"][prod_id] += quantity
                    else:
                        self.producers[customer_id]["inc_inventory"][prod_id] += quantity

                if label == "ended_plan":
                    prod_id = dic["product_id"]
                    amt = dic["quantity"]
                    self.active_plans[prod_id]["plans"] -= 1
                    self.active_plans[prod_id]["quantity"] -= amt

                if label == "shipment_received":
                    prod_id = dic["product_id"]
                    amt = dic["amount"]
                    self.producers[id]["inc_inventory"][prod_id] -= amt

                if label == "current_demand":
                    self.record_demand_signal(dic)

                if label == "resupply_rate":
                    self.record_resupply_rate(dic)

                if label == "drafting_failure":
                    self.record_drafting_failure(dic)

                if label == "stalled_plan":
                    self.record_stalled_plan(dic)

                if label == "unstalled_plan":
                    self.record_stallage_resolved(dic)

                if label == "start_plan_stalled":
                    self.record_start_plan_stalled(dic)

                if label == "start_plan_stallage_resolved":
                    self.record_start_plan_stallage_resolved(dic)

                if label == "reorder_attempt":
                    prod_id = dic["product_id"]
                    self.reorder_attempts[prod_id] += 1

                if label == "reorder_failure":
                    prod_id = dic["product_id"]
                    self.reorder_failures[prod_id] += 1

                if label == "newly_employed":
                    self.producers[id]["employees"] += 1

                if label == "busyness":
                    firm_busyness = dic["firm_busyness"]
                    transfers_available = dic["max_workers_for_transfer"]
                    self.producers[id]["recent_busyness"] = firm_busyness
                    self.overall_busyness_data.append(firm_busyness)

                if label == "accepted_order":
                    prod_id = dic["product_id"]
                    time = dic["offered_turnaround_time"]
                    self.turnover_times[prod_id].append(time)

                if label == "transfer_request":
                    cat = self.producers[id]["catalog"]
                    for i in cat:
                        sector_idx = self.get_sector_idx(i)
                        self.transfer_requests_by_sector[sector_idx] +=  1
                        if len(self.transfer_requests_by_sector_t) == 0 or self.current_t != self.transfer_requests_by_sector_t[-1]:
                            self.transfer_requests_by_sector_t = np.append(self.transfer_requests_by_sector_t, self.current_t)

                if label == "transfer":
                    old_emp = dic["old_workplace_id"]
                    old_emp_is_distributor = (old_emp >= self.settings["n_producers"])
                    if old_emp_is_distributor:
                        old_emp = self._get_dist_key(old_emp)
                        self.distributors[old_emp]["employees"] -= 1
                    else:
                        self.producers[old_emp]["employees"] -= 1

                    new_emp = dic["new_workplace_id"]
                    new_emp_is_distributor = (new_emp >= self.settings["n_producers"])
                    if new_emp_is_distributor:
                        new_emp = self._get_dist_key(new_emp)
                        self.distributors[new_emp]["employees"] += 1
                    else:
                        self.producers[new_emp]["employees"] += 1

                # if label == "draft_plan":
                #     prod_id = dic["product_id"]
                #     quantity = dic["quantity"]
                #     self.order_sizes[prod_id].append(quantity)

            case "Distributor":
                if label == "inventory_level":
                    prod_id = dic["product_id"]
                    amt = dic["amount"]
                    distributor_id = dic['id']
                    dist_key = self._get_dist_key(distributor_id)
                    self.distributors[dist_key]["inventory"][prod_id] = amt

                if label == "inventory_reduction":
                    prod_id = dic["product_id"]
                    amt = dic["amount"]
                    dist_key = self._get_dist_key(id)
                    self.distributors[dist_key]["inventory"][prod_id] -= amt

                if label == "catalog":
                    product_ids_str = dic["product_ids"]
                    product_ids_str_list = product_ids_str.split(",")
                    product_ids = [int(product_id) for product_id in product_ids_str_list]
                    dist_key = self._get_dist_key(id)
                    self.distributors[dist_key]["catalog"] = product_ids

                if label == "accepted_order":
                    prod_id = dic["product_id"]
                    time = dic["offered_turnaround_time"]
                    self.turnover_times[prod_id].append(time)

                if label == "shipment_received":
                    prod_id = dic["product_id"]
                    amt = dic["amount"]
                    dist_id = self._get_dist_key(id)
                    self.distributors[dist_id]["inc_inventory"][prod_id] -= amt

                if label == "catalog_addition":
                    prod_id = dic["product_id"]
                    dist_id = self._get_dist_key(id)
                    self.distributors[dist_id]["catalog"].append(prod_id)

                if label == "current_demand":
                    self.record_demand_signal(dic)

                if label == "resupply_rate":
                    self.record_resupply_rate(dic)

                if label == "drafting_failure":
                    self.record_drafting_failure(dic)

                if label == "reorder_failure":
                    prod_id = dic["product_id"]
                    self.reorder_failures[prod_id] += 1

                if label == "reorder_attempt":
                    prod_id = dic["product_id"]
                    self.reorder_attempts[prod_id] += 1

                if label == "newly_employed":
                    dist_id = self._get_dist_key(id)
                    self.distributors[dist_id]["employees"] += 1

                if label == "busyness":
                    firm_busyness = dic["firm_busyness"]
                    dist_id = self._get_dist_key(id)
                    self.distributors[dist_id]["recent_busyness"] = firm_busyness
                    self.overall_busyness_data.append(firm_busyness)

                if label == "transfer":
                    old_emp = dic["old_workplace_id"]
                    old_emp_is_distributor = (old_emp >= self.settings["n_producers"])
                    if old_emp_is_distributor:
                        old_emp = self._get_dist_key(old_emp)
                        self.distributors[old_emp]["employees"] -= 1
                    else:
                        self.producers[old_emp]["employees"] -= 1

                    new_emp = dic["new_workplace_id"]
                    new_emp_is_distributor = (new_emp >= self.settings["n_producers"])
                    if new_emp_is_distributor:
                        new_emp = self._get_dist_key(new_emp)
                        self.distributors[new_emp]["employees"] += 1
                    else:
                        self.producers[new_emp]["employees"] += 1

                if label == "transfer_request":
                    dist_id = self._get_dist_key(id)
                    cat = self.distributors[dist_id]["catalog"]
                    if len(cat) == 0:
                        return
                    sector_idx = self.get_sector_idx(cat[0])
                    self.transfer_requests_by_sector[sector_idx] +=  1
                    if len(self.transfer_requests_by_sector_t) == 0 or self.current_t != self.transfer_requests_by_sector_t[-1]:
                        self.transfer_requests_by_sector_t = np.append(self.transfer_requests_by_sector_t, self.current_t)

                if label == "pursued_plan":
                    customer_id = dic["customer_id"]
                    is_distributor = (customer_id >= self.settings["n_producers"])
                    if is_distributor:
                        customer_id = self._get_dist_key(customer_id)
                    prod_id = dic["product_id"]
                    sector_id = self.get_sector_idx(prod_id)
                    quantity = dic["quantity"]
                    lead_time = dic.get("lead_time", 0)
                    team_size = dic.get("num_workers")

                    self.active_plans[prod_id]["plans"] += 1
                    self.active_plans[prod_id]["quantity"] += quantity
                    self.order_sizes[prod_id].append(quantity)
                    self.lead_times[prod_id].append(lead_time)
                    self.team_sizes[prod_id].append(team_size)
                    self.long_run_sector_activity[sector_id] += quantity

                    if is_distributor:
                        self.distributors[customer_id]["inc_inventory"][prod_id] += quantity
                    else:
                        self.producers[customer_id]["inc_inventory"][prod_id] += quantity

                if label == "stalled_plan":
                    self.record_stalled_plan(dic)

                if label == "unstalled_plan":
                    self.record_stallage_resolved(dic)

                if label == "start_plan_stalled":
                    self.record_start_plan_stalled(dic)

                if label == "start_plan_stallage_resolved":
                    self.record_start_plan_stallage_resolved(dic)

                if label == "ended_plan":
                    prod_id = dic["product_id"]
                    amt = dic["quantity"]
                    self.active_plans[prod_id]["plans"] -= 1
                    self.active_plans[prod_id]["quantity"] -= amt

    def log_text(self, dic):
        extra = {
            "client": dic.get("client", "Unknown"),
            "time": self.current_t,
        }
        msg = "\n"
        for key, value in dic.items():
            if key in ("id", "client", "level", "t", "label"):
                continue

            msg += f"   {key}: {value}\n"
            
        level = dic.get("level", 20)
        logger.log(level, msg, extra= extra)

    def _update_hourly_stats(self):
        """ 
        Updates the trajectories dictionary.
        """

        good_lo, good_hi = self.get_goods_idxs()
        c_good_lo, c_good_hi = self.get_consumer_goods_idxs()
        m_lo, m_hi = self.get_machines_idxs()

        producer_supply = self._get_producer_supply()
        producer_supply_machines = self._get_producer_supply(machines= True)

        consumer_goods_supply = self._get_distributor_supply()
        distributor_unshelved_supply = self._get_distributor_supply(produced= True)

        self._set_pending_inventories()
        average_demands_producers = self._get_product_property_aggregate(
            self.producers, key="demand_signals"
        )
        average_demands_distributors = self._get_product_property_aggregate(
            self.distributors, key="demand_signals"
        )
        average_reorder_thresholds_producers = self._get_product_property_aggregate(
            self.producers, key="reorder_thresholds"
        )
        average_reorder_thresholds_distributors = self._get_product_property_aggregate(
            self.distributors, key="reorder_thresholds"
        )
        average_pending_inventories_producers = self._get_product_property_aggregate(
            self.producers, key="pending_inventory"
        )
        average_pending_inventories_distributors = self._get_product_property_aggregate(
            self.distributors, key="pending_inventory"
        )
        average_resupply_rates_producers = self._get_product_property_aggregate(
            self.producers, key="resupply_rates"
        )
        average_resupply_rates_distributors = self._get_product_property_aggregate(
            self.distributors, key="resupply_rates"
        )
        average_resupply_deficits_producers = self._get_product_property_aggregate(
            self.producers, key="resupply_deficits"
        )
        average_resupply_deficits_distributors = self._get_product_property_aggregate(
            self.distributors, key="resupply_deficits"
        )

        overall_reorder_thresholds = self._get_product_property_aggregate(
            self.producers, self.distributors,
            key= "reorder_thresholds", 
            operation= "sum"
        )
        overall_pending_inventory = self._get_product_property_aggregate(
            self.producers, self.distributors,
            key= "pending_inventory",
            operation= "sum"
        )

        accounts = [dic["account"] for _, dic in self.persons.items()]
        health_statuses = [0 if dic["health"] == "Healthy" else 1 for _, dic in self.persons.items()]
        n_unhealthy = sum(health_statuses)
        n_healthy = len(self.persons) - n_unhealthy

        average_endowments = self._get_average_endowments(self.persons)
        average_proficiencies = self._get_average_abilities(self.persons)

        plans_in_motion_goods = [self.active_plans[i]["plans"] for i in range(good_lo,good_hi)]
        plans_in_motion_c_goods = [self.active_plans[i]["plans"] for i in range(c_good_lo,c_good_hi)]
        plans_in_motion_machines = [self.active_plans[i]["plans"] for i in range(m_lo,m_hi)]

        quantities_in_prod_goods = [self.active_plans[i]["quantity"] for i in range(good_lo,good_hi)]
        quantities_in_prod_c_goods = [self.active_plans[i]["quantity"] for i in range(c_good_lo,c_good_hi)]
        quantities_in_prod_machines = [self.active_plans[i]["quantity"] for i in range(m_lo,m_hi)]

        sectoral_employment = self._get_available_employment_by_sector()
        sectoral_busyness = self._get_sectoral_busyness()

        drafting_casualty_totals_goods = list(self.drafting_failures_casualties[good_lo:good_hi])
        drafting_casualty_totals_distribution = [np.sum(self.drafting_failures_casualties[c_good_lo:c_good_hi])]
        drafting_casualty_totals_machines = list(self.drafting_failures_casualties[m_lo:m_hi])
        
        sectoral_drafting_casualty_totals = drafting_casualty_totals_goods + \
                                            drafting_casualty_totals_distribution + \
                                            drafting_casualty_totals_machines
        
        goods_drafting_failures_caused_by_workers = self.drafting_failures_casualties_from_workers[good_lo:good_hi]
        machines_drafting_failures_caused_by_workers = self.drafting_failures_casualties_from_workers[m_lo:m_hi]
        c_goods_drafting_failures_caused_by_workers = self.drafting_failures_casualties_from_workers[c_good_lo:c_good_hi]

        drafting_failures_caused_by_goods = self.drafting_failures_causes_resources[good_lo:good_hi]
        drafting_failures_caused_by_machines = self.drafting_failures_causes_resources[m_lo:m_hi]

        stalled_plan_casualties_goods = [len(self.stalled_plan_casualties[i]) for i in range(good_lo,good_hi)]
        stalled_plan_casualties_c_goods = [len(self.stalled_plan_casualties[i]) for i in range(c_good_lo,c_good_hi)]
        stalled_plan_casualties_machines = [len(self.stalled_plan_casualties[i]) for i in range(m_lo,m_hi)]

        stalled_plan_causes_goods = [len(self.stalled_plan_causes[i]) for i in range(good_lo,good_hi)]
        stalled_plan_causes_machines = [len(self.stalled_plan_causes[i]) for i in range(m_lo,m_hi)]

        stalled_plan_causes_total_deficits = self.get_plan_stallage_deficits()
        stalled_plan_causes_total_deficits_goods = stalled_plan_causes_total_deficits[good_lo:good_hi]
        stalled_plan_causes_total_deficits_machines = stalled_plan_causes_total_deficits[m_lo:m_hi]

        start_plan_stall_casualties_goods = [
            len(self.start_plan_stall_casualties[i])
            for i in range(good_lo, good_hi)
        ]

        start_plan_stall_casualties_machines = [
            len(self.start_plan_stall_casualties[i])
            for i in range(m_lo, m_hi)
        ]

        start_plan_stall_causes_goods = [
            len(self.start_plan_stall_causes[i])
            for i in range(good_lo, good_hi)
        ]

        start_plan_stall_causes_machines = [
            len(self.start_plan_stall_causes[i])
            for i in range(m_lo, m_hi)
        ]

        start_plan_stall_deficits = [
            sum(plan_deficits.values())
            for plan_deficits in self.start_plan_stall_causes.values()
        ]

        start_plan_stall_deficits_goods = \
            start_plan_stall_deficits[good_lo:good_hi]

        start_plan_stall_deficits_machines = \
            start_plan_stall_deficits[m_lo:m_hi]

        busyness_data = np.asarray(self.overall_busyness_data)
        if len(self.overall_busyness_data) > 0:
            low, hi = np.quantile(busyness_data, [0.005, 0.995])
            overall_busyness_bins = np.linspace(low, hi, 100)
        else:
            overall_busyness_bins = np.array([0.5])

        self.long_run_employment_by_sector += sectoral_employment

        n_prod_goods = self.settings["n_goods"]
        l_list = list(self.l)

        self.traj = {
            "producer_goods_prices": Append(self.prices[:n_prod_goods]),
            "consumption_goods_prices": Append(self.prices[n_prod_goods:2*n_prod_goods]),
            "machine_prices": Append(self.prices[2*n_prod_goods:]),

            "producer_goods_living_labor_vals": Append(self.living_labor_values[:n_prod_goods]),
            "consumption_goods_living_labor_vals": Append(self.living_labor_values[n_prod_goods:2*n_prod_goods]),
            "machine_prices_living_labor_vals": Append(self.living_labor_values[2*n_prod_goods:]),

            "producer_goods_values": Append(self.values[:n_prod_goods]),
            "consumption_goods_values": Append(self.values[n_prod_goods:2*n_prod_goods]),
            "machine_values": Append(self.values[2*n_prod_goods:]),

            "b": Append(self.b[c_good_lo:c_good_hi]),
            "producer_supply": Append(producer_supply),
            "producer_supply_machines": Append(producer_supply_machines),
            "consumer_goods_supply": Append(consumer_goods_supply),
            "distributor_unshelved_supply": Append(distributor_unshelved_supply),

            "avg_account": Append(np.average(accounts)),
            "min_account": Append(np.min(accounts)),
            "max_account": Append(np.max(accounts)),
            "avg_endowments": Append(average_endowments),

            "plans_in_progress_goods": Append(plans_in_motion_goods),
            "plans_in_progress_c_goods": Append(plans_in_motion_c_goods),
            "plans_in_progress_machines": Append(plans_in_motion_machines),

            "goods_in_production_goods": Append(quantities_in_prod_goods),
            "goods_in_production_c_goods": Append(quantities_in_prod_c_goods),
            "goods_in_production_machines": Append(quantities_in_prod_machines),

            "n_healthy": Append(n_healthy),
            "n_unhealthy": Append(n_unhealthy),
            "average_proficiencies": Append(average_proficiencies),
            "employment": Append(self.current_employment),
            "mean_consumption_frequencies": Append(self.consumption_frequencies[c_good_lo:c_good_hi]),
            "mean_consumption_periods": Append(self.consumption_periods[c_good_lo:c_good_hi]),

            "total_reorder_thresholds_goods": Append(overall_reorder_thresholds[good_lo:good_hi]),
            "total_pending_inventories_goods": Append(overall_pending_inventory[good_lo:good_hi]),

            "average_demand_producers_goods": Append(average_demands_producers[good_lo:good_hi]),
            "average_reorder_thresholds_producers_goods": Append(average_reorder_thresholds_producers[good_lo:good_hi]),
            "average_pending_inventories_producers_goods": Append(average_pending_inventories_producers[good_lo:good_hi]),

            "average_demand_distributors_goods": Append(average_demands_distributors[good_lo:good_hi]),
            "average_reorder_thresholds_distributors_goods": Append(average_reorder_thresholds_distributors[good_lo:good_hi]),
            "average_pending_inventories_distributors_goods": Append(average_pending_inventories_distributors[good_lo:good_hi]),

            "average_demand_distributors_c_goods": Append(average_demands_distributors[c_good_lo:c_good_hi]),
            "average_reorder_thresholds_c_goods": Append(average_reorder_thresholds_distributors[c_good_lo:c_good_hi]),
            "average_pending_inventories_c_goods": Append(average_pending_inventories_distributors[c_good_lo:c_good_hi]),

            "average_machine_demand": Append(average_demands_producers[m_lo:m_hi]),
            "average_reorder_thresholds_machines": Append(average_reorder_thresholds_producers[m_lo:m_hi]),
            "average_machine_pending_inventory": Append(average_pending_inventories_producers[m_lo:m_hi]),

            "resupply_rates_producers_goods": Append(average_resupply_rates_producers[good_lo:good_hi]),
            "resupply_rates_distributors_goods": Append(average_resupply_rates_distributors[good_lo:good_hi]),
            "resupply_rates_machines": Append(average_resupply_rates_producers[m_lo:m_hi]),
            "resupply_rates_c_goods": Append(average_resupply_rates_distributors[c_good_lo:c_good_hi]),

            "resupply_deficits_producers_goods": Append(average_resupply_deficits_producers[good_lo:good_hi]),
            "resupply_deficits_distributors_goods": Append(average_resupply_deficits_distributors[good_lo:good_hi]),
            "resupply_deficits_machines": Append(average_resupply_deficits_producers[m_lo:m_hi]),
            "resupply_deficits_c_goods": Append(average_resupply_deficits_distributors[c_good_lo:c_good_hi]),

            "reorder_attempts_goods": Append(self.reorder_attempts[good_lo:good_hi]),
            "reorder_attempts_c_goods": Append(self.reorder_attempts[c_good_lo:c_good_hi]),
            "reorder_attempts_machines": Append(self.reorder_attempts[m_lo:m_hi]),

            "reorder_failures_goods": Append(self.reorder_failures[good_lo:good_hi]),
            "reorder_failures_c_goods": Append(self.reorder_failures[c_good_lo:c_good_hi]),
            "reorder_failures_machines": Append(self.reorder_failures[m_lo:m_hi]),

            "hrly_sectoral_drafting_casualty_totals": Append(sectoral_drafting_casualty_totals),

            "goods_drafting_failures_caused_by_workers": Append(goods_drafting_failures_caused_by_workers),
            "c_goods_drafting_failures_caused_by_workers": Append(c_goods_drafting_failures_caused_by_workers),
            "machines_drafting_failures_caused_by_workers": Append(machines_drafting_failures_caused_by_workers),

            "drafting_failures_caused_by_goods": Append(drafting_failures_caused_by_goods),
            "drafting_failures_caused_by_machines": Append(drafting_failures_caused_by_machines),

            "available_employment_by_sector": Append(sectoral_employment),

            "sectoral_busyness": Append(sectoral_busyness),
            "overall_busyness": Append(self.overall_busyness),
            "busyness_data": Replace(self.overall_busyness_data),
            "overall_busyness_bins": Replace(overall_busyness_bins),

            "l_goods": Append(l_list[good_lo:good_hi]),
            "l_c_goods": Append(l_list[c_good_lo:c_good_hi]),
            "l_machines": Append(l_list[m_lo:m_hi]),

            "transfer_requests_by_sector": Append(self.transfer_requests_by_sector),
            "long_run_employment_by_sector": Append(self.long_run_employment_by_sector / max(self.current_t, 1)),
            "eqb_employment": Append(self.eqb_employment),

            "min_hrly_output": Append(self.min_hrly_output),
            "long_run_activity": Append(self.long_run_sector_activity / max(self.current_t, 1)),

            "busy_lower_bound": Append(self.busy_lower_bd),
            "busy_upper_bound": Append(self.busy_upper_bd),
            "work_hours_daily": Append(self.weekly_working_hours / self.settings["init_working_week"]),
            "transfer_requests_by_sector_t": Replace(self.transfer_requests_by_sector_t),

            "fic": Append(self.fic),
            "average_consumer_goods_value": Append(self.average_consumer_goods_value),
            "public_fund": Append(self.public_fund),
            "public_revenue": Append(self.public_revenue),
            "public_expenditure": Append(self.public_expenditure),
            "average_public_sector_consumer_goods_value": Append(self.average_public_sector_consumer_goods_value),

            "stalled_plan_casualties_goods": Append(stalled_plan_casualties_goods),
            "stalled_plan_casualties_c_goods": Append(stalled_plan_casualties_c_goods),
            "stalled_plan_casualties_machines": Append(stalled_plan_casualties_machines),

            "stalled_plan_causes_goods": Append(stalled_plan_causes_goods),
            "stalled_plan_causes_machines": Append(stalled_plan_causes_machines),

            "stalled_plan_causes_deficits_goods": Append(stalled_plan_causes_total_deficits_goods),
            "stalled_plan_causes_deficits_machines": Append(stalled_plan_causes_total_deficits_machines),

            "start_plan_stall_casualties_goods": Append(start_plan_stall_casualties_goods),
            "start_plan_stall_casualties_machines": Append(start_plan_stall_casualties_machines),
            "start_plan_stall_causes_goods": Append(start_plan_stall_causes_goods),
            "start_plan_stall_causes_machines": Append(start_plan_stall_causes_machines),
            "start_plan_stall_causes_deficits_goods": Append(start_plan_stall_deficits_goods),
            "start_plan_stall_causes_deficits_machines": Append(start_plan_stall_deficits_machines),

        }
        if self.current_t == 0:
            self.traj["A"] = Replace(self.A)
            if hasattr(self, "seed"):
                # logger.info(f"Adding seed {int(self.seed)} to update entry in traj")
                self.traj["seed"] = Update(
                    details= {"param": "seed", "value": int(self.seed)}
                )
            self.traj["week_counter"] = Append(self.current_week)

        self.transfer_requests_by_sector = np.zeros(self.settings["n_sectors"])
        self.reorder_attempts = np.zeros(self.settings["n_products"])
        self.reorder_failures = np.zeros(self.settings["n_products"])
        self.drafting_failures_casualties_from_workers = np.zeros(self.settings["n_products"])
        self.drafting_failures_casualties = np.zeros(self.settings["n_products"])
        self.drafting_failures_causes_resources = np.zeros(self.settings["n_products"])

    def _update_weekly_stats(self):
        good_lo, good_hi = self.get_goods_idxs()
        c_good_lo, c_good_hi = self.get_consumer_goods_idxs()
        m_lo, m_hi = self.get_machines_idxs()

        order_size_averages = []
        for i, order_size_data in enumerate(self.order_sizes):
            if len(order_size_data) > 0:
                order_size_averages.append(np.average(order_size_data))
            else:
                order_size_averages.append(self.old_order_size_avgs[i])

        self.old_order_size_avgs = order_size_averages

        lead_time_averages = []
        for i, lead_size_data in enumerate(self.lead_times):
            if len(lead_size_data) > 0:
                lead_time_averages.append(np.average(lead_size_data))
            else:
                lead_time_averages.append(self.old_lead_time_avgs[i])

        self.old_lead_time_avgs = lead_time_averages

        team_size_averages = []
        for i, team_size_data in enumerate(self.team_sizes):
            if len(team_size_data) > 0:
                team_size_averages.append(np.average(team_size_data))
            else:
                team_size_averages.append(self.old_team_size_avgs[i])

        self.team_size_averages = team_size_averages

        self.traj["order_sizes_goods"] = Append(order_size_averages[good_lo:good_hi])
        self.traj["order_sizes_c_goods"] = Append(order_size_averages[c_good_lo:c_good_hi])
        self.traj["order_sizes_machines"] = Append(order_size_averages[m_lo:m_hi])

        self.traj["lead_times_goods"] = Append(lead_time_averages[good_lo:good_hi])
        self.traj["lead_times_c_goods"] = Append(lead_time_averages[c_good_lo:c_good_hi])
        self.traj["lead_times_machines"] = Append(lead_time_averages[m_lo:m_hi])

        self.traj["team_sizes_goods"] = Append(team_size_averages[good_lo:good_hi])
        self.traj["team_sizes_c_goods"] = Append(team_size_averages[c_good_lo:c_good_hi])
        self.traj["team_sizes_machines"] = Append(team_size_averages[m_lo:m_hi])
        self.traj["week_counter"] = Append(self.current_t)

        for dataset in (self.order_sizes, self.lead_times, self.team_sizes):
            for ls in dataset:
                ls.clear()

        if not self.work_hours_update_this_step:
            return 

        busyness_data = copy.deepcopy(np.array(self.individual_busyness_data))
        lb = np.min(busyness_data)
        ub = np.max(busyness_data)
        x_sample = np.linspace(lb, ub, 400)
        self.traj["busyness_dist_support"] = Replace(x_sample)
        if self.censored_busyness_mean is not None:
            logger.info(f"{self.censored_busyness_mean=}, {self.censored_busyness_stddev=}")
            dist = norm(
                loc= self.censored_busyness_mean,
                scale= self.censored_busyness_stddev
            )
            self.traj["censored_busyness_dist"] = Replace(dist.pdf(x_sample))
        if self.uncensored_busyness_mean is not None:
            logger.info(f"{self.uncensored_busyness_mean=}, {self.uncensored_busyness_stddev=}")
            dist = norm(
                loc= self.uncensored_busyness_mean,
                scale= self.uncensored_busyness_stddev
            )
            self.traj["uncensored_busyness_dist"] = Replace(dist.pdf(x_sample))

        self.traj["individual_busyness_data"] = Replace(busyness_data)
        self.traj["individual_busyness_data_bins"] = np.histogram_bin_edges(busyness_data, bins= "auto")
        self.individual_busyness_data.clear()
        self.work_hours_update_this_step = False

    def initialize_properties(self):
        N = self.settings["n_persons"]
        net_weekly_demand = N*24*7*self.consumption_frequencies
        gross_weekly_demand = inv(np.eye(self.settings["n_products"]) - self.A)@net_weekly_demand

        sectoral_weekly_labor_req_raw = self.l * gross_weekly_demand
        min_hrly_output = gross_weekly_demand / (24*7)

        goods_lo, goods_hi = self.get_goods_idxs()
        c_goods_lo, c_goods_hi = self.get_consumer_goods_idxs()
        m_lo, m_hi = self.get_machines_idxs()

        prod_goods_sectoral_weekly_labor_req = list(sectoral_weekly_labor_req_raw[goods_lo:goods_hi])
        machine_sectoral_weekly_labor_req = list(sectoral_weekly_labor_req_raw[m_lo:m_hi])
        
        prod_goods_min_hrly_output = list(min_hrly_output[goods_lo:goods_hi])
        machines_min_hrly_output = list(min_hrly_output[m_lo:m_hi])

        overall_sectoral_weekly_labor_req = []
        overall_sectoral_activity_levels = []

        overall_sectoral_weekly_labor_req.extend(prod_goods_sectoral_weekly_labor_req)
        overall_sectoral_weekly_labor_req.append(sum(sectoral_weekly_labor_req_raw[c_goods_lo:c_goods_hi]))
        overall_sectoral_weekly_labor_req.extend(machine_sectoral_weekly_labor_req)
        overall_sectoral_weekly_labor_req = np.asarray(overall_sectoral_weekly_labor_req)

        overall_sectoral_activity_levels.extend(prod_goods_min_hrly_output)
        overall_sectoral_activity_levels.append(sum(min_hrly_output[c_goods_lo:c_goods_hi]))
        overall_sectoral_activity_levels.extend(machines_min_hrly_output)
        overall_sectoral_activity_levels = np.asarray(overall_sectoral_activity_levels)

        init_work_hours = self.settings["init_working_day"]
        init_work_days = self.settings["init_working_week"]

        self.predicted_order_sizes = 0.25 * 1.5 * 24*7 * min_hrly_output
        self.eqb_employment = overall_sectoral_weekly_labor_req / (init_work_hours*init_work_days)
        self.busy_lower_bd = self.settings["consump_epsilon"]*(init_work_hours*init_work_days / (24*7))
        self.busy_upper_bd = (init_work_hours*init_work_days / (24*7))
        self.min_hrly_output = overall_sectoral_activity_levels
        dim = self.A.shape[0]
        self.values = inv(np.eye(dim) - self.A.T)@self.l

        logger.info(f"A = \n{np.array2string(
            self.A,
            formatter={"float_kind": lambda x: f"{x:10.7f}"}
        )}")

        (evals, evecs) = eig(self.A)
        idx = np.argmax(evals.real)
        r_hat = np.real(evals[idx])

        logger.info(f"Spectral radius of A: {r_hat}")


        if self.settings["init_prices"] == "values":
            self.b = self.consumption_frequencies

        if self.settings["init_prices"] == "equilibrium_prices":
            M = self.A + np.linalg.outer(self.b, self.l)
            (evals, evecs) = eig(M.T)
            idx = np.argmax(evals.real)
            r_hat = np.real(evals[idx])
            epr = 1/r_hat - 1

    # The stuff below this point are all just helper functions. 
    # Unless you're making your own or debugging something, you shouldn't ever have to look below here.

    def step(self) -> bool:
        """ 
        The basic step function. Retrieves json dictionaries for processing until the logged time step changes or the simulation finishes.
        """
        while True:
            if self.stdout_done and self.stderr_done:
                return True

            item = self.collector.get_next()

            if item.kind == "error":
                raise RuntimeError(f"{item.stream} reader failed: {item.payload}")

            if item.stream == "meta":
                if item.kind == "wait":
                    return False
                continue

            if item.stream == "stderr":
                if item.kind == "eof":
                    self.stderr_done = True
                    if len(self.current_cout) > 0:
                        self._log_cout_and_clear()
                else:
                    logger.info(f"Standard Error: {item.payload}")

            if item.stream == "stdout":
                if item.kind == "eof":
                    self.stdout_done = True
                    if len(self.current_cout) > 0:
                        self._log_cout_and_clear()

                if item.kind == "json":
                    if len(self.current_cout) > 0:
                        self._log_cout_and_clear()

                    dic = item.payload
                    if self.current_t != dic["t"]:
                        if self.current_t == 0:
                            self.initialize_properties()
                            self._update_hourly_stats()
                        else:
                            self._update_hourly_stats()

                        self.current_t = dic["t"]
                        if self.current_t % (24*7) == 0:
                            self.current_week += 1
                            self._update_weekly_stats()

                        self.traj["t"] = Append(self.current_t)
                        self.t.append(self.current_t)
                        self._process_dic(dic)
                        return False
                    else:
                        self._process_dic(dic)
                else:
                    if item.payload is not None:
                        self.current_cout.append(item.payload)
                continue

            if self.stdout_done and self.stderr_done:
                return True

    def get_data(self):
        if hasattr(self, "traj"):
            return self.traj
        else:
            return {}

    def _log_cout_and_clear(self):
        msg = "Standard Output: \n   "
        for entry in self.current_cout:
            msg += entry
            msg += "\n   "

        logger.info(msg)
        self.current_cout.clear()

    def _get_args_from_settings(self):
        args = [
            "-j",
            "-n", str(self.settings["n_time_steps"]),
            "-p", str(self.settings["n_persons"]),
            "-h", str(self.settings["init_working_day"]),
            "-w", str(self.settings["init_working_week"]),
            "-g", str(self.settings["n_goods"]),
            "-m", str(self.settings["n_machines"]),
            "-r", str(self.settings["n_producers"]),
            "-d", str(self.settings["n_distributors"]),
            "-s", str(self.settings["daily_sick_chance"]),
            "-a", str(self.settings["n_abilities"]),
            "-v", str(self.settings["person_ability_stddev"]),
            "-i", str(self.settings["max_num_inputs_per_good"]),
            "--production_difficulty", str(self.settings["productivity"]),
            "--consumption_demand", str(self.settings["consump_epsilon"]),
            "--init_prices", str(self.settings["init_prices"]),
        ]

        logger.info(f"\n   {self.settings["fixed_seed"]=}, \n   {self.settings["seed"]=}")
        if self.settings["fixed_seed"]:
            args.append("-e")
            args.append(str(self.settings["seed"]))

        if self.settings["free_goods"]:
            args.append("--public_sector_expansion_period")
            args.append(str(self.settings["new_free_good_interval"]))
        else:
            args.append("--public_sector_expansion_period")
            args.append("0")

        return args

    def _get_theoretical_values(self, A, l):
        n = A.shape[0]
        vals = inv(np.eye(n) - A.T)@l

        return vals

    def _get_distributor_supply(self, produced= False):
        n_prod_goods = self.settings["n_goods"]
        if produced:
            idx_low = 0
            idx_high = n_prod_goods
        else:
            idx_low = n_prod_goods
            idx_high = 2*n_prod_goods

        supply = np.zeros(n_prod_goods)
        for properties in self.distributors.values():
            inventory = properties["inventory"]
            supply += inventory[idx_low:idx_high]

        return supply

    def _get_producer_supply(self, machines= False):
        n_prod_goods = self.settings["n_goods"]
        n_machines = self.settings["n_machines"]
        if machines:
            idx_low = 2*n_prod_goods
            idx_high = idx_low + n_machines
        else:
            idx_low = 0
            idx_high = n_prod_goods

        supply = np.zeros(n_prod_goods) if not machines else np.zeros(n_machines)
        for properties in self.producers.values():
            inventory = properties["inventory"]
            supply += inventory[idx_low:idx_high]
        return supply

    def _get_product_property_aggregate(self, *group_dicts, key= "demand_signals", operation= "avg"):
        n_products = self.settings["n_products"]
        aggregates = np.zeros(n_products)
        for idx in range(n_products):
            all_vals = []
            for group_dict in group_dicts:
                all_vals += [
                    member_dict[key][idx]
                    for member_dict in group_dict.values()
                    if member_dict["tracked_inputs"][idx]
                ]
            if operation == "avg":
                aggregates[idx] = np.average(all_vals) if all_vals else 0
            elif operation == "sum":
                aggregates[idx] = np.sum(all_vals) if all_vals else 0

        return aggregates

    def get_goods_idxs(self):
        low = 0
        hi = self.settings["n_goods"]
        return low, hi

    def get_consumer_goods_idxs(self):
        low = self.settings["n_goods"]
        hi = 2*self.settings["n_goods"]
        return low, hi

    def get_machines_idxs(self):
        low = 2*self.settings["n_goods"]
        hi = 2*self.settings["n_goods"] + self.settings["n_machines"]
        return low, hi

    def get_sector_idx(self, prod_id):
        n_goods = self.settings["n_goods"]
        n_machines = self.settings["n_machines"]

        if prod_id < n_goods:
            return prod_id

        if prod_id < 2 * n_goods:
            return n_goods  # distribution sector

        machine_idx = prod_id - 2 * n_goods
        return n_goods + 1 + machine_idx

    def _get_supply(self, distributors, producers= None):
        n_products = self.settings["n_products"]

        supply = np.zeros(n_products)

        for properties in distributors.values():
            inventory = properties["inventory"]
            supply += inventory

        if producers != None:
            for properties in producers.values():
                inventory = properties["inventory"]
                supply += inventory

        return supply

    def _get_average_endowments(self, persons):

        n_prod_goods = self.settings["n_goods"]
        idx_low = n_prod_goods
        idx_high = 2*n_prod_goods
        consumer_goods_idxs = list(range(idx_low, idx_high))

        n_goods = self.settings["n_goods"]

        endowments = [dic["endowment"] for _,dic in persons.items()]
        itemwise_endowments = [[] for i in range(n_prod_goods)]
        for i, idx in enumerate(consumer_goods_idxs):
            for j in range(len(persons)):
                itemwise_endowments[i].append(endowments[j][idx])

        average_endowments = [np.average(itemwise_endowments[i]) for i in range(n_goods)]
        return average_endowments

    def _get_average_abilities(self, persons):
        n_abilities = self.settings["n_abilities"]

        abilities = [dic["abilities"] for _,dic in persons.items()]

        abilitywise_profs = [[] for i in range(n_abilities)]
        for i in range(n_abilities):
            for j in range(len(persons)):
                abilitywise_profs[i].append(abilities[j][i])

        average_proficiencies = [np.average(abilitywise_profs[i]) for i in range(n_abilities)]
        return average_proficiencies

    def _get_available_employment_by_sector(self):
        n_sectors = self.settings["n_sectors"]
        sectoral_employment = np.zeros(n_sectors)
        for properties in self.producers.values():
            cat = properties["catalog"]
            employees = properties["employees"]
            for prod_id in cat:
                sector_id = self.get_sector_idx(prod_id)
                sectoral_employment[sector_id] += employees

        for properties in self.distributors.values():
            sectoral_employment[self.settings["n_goods"]] += properties["employees"]

        return sectoral_employment

    def _get_sectoral_drafting_failures_caused_by_workes(self):
        n_sectors = self.settings["n_sectors"]
        sectoral_drafting_failure_totals = np.zeros(n_sectors)
        
    def _get_activity_levels_by_sector(self):
        n_sectors = self.settings["n_sectors"]
        sectoral_activity_levels = np.zeros(n_sectors)

    def _get_sectoral_busyness(self):
        n_sectors = self.settings["n_goods"]+self.settings["n_machines"]+1

        sectoral_busyness_data = [[] for _ in range(n_sectors)]
        for properties in self.producers.values():
            cat = properties["catalog"]
            busyness = properties["recent_busyness"]
            for prod_id in cat:
                sector_id = self.get_sector_idx(prod_id)
                sectoral_busyness_data[sector_id].append(busyness)

        for properties in self.distributors.values():
            sectoral_busyness_data[self.settings["n_goods"]].append(properties["recent_busyness"])

        sectoral_busyness = np.array([np.average(sector) if sector else 0.0 for sector in sectoral_busyness_data])
        return sectoral_busyness

    def _set_pending_inventories(self):
        for _, producer_dict in self.producers.items():
            producer_dict["pending_inventory"] = producer_dict["inventory"]+producer_dict["inc_inventory"]

        for _, distributor_dict in self.distributors.items():
            distributor_dict["pending_inventory"] = distributor_dict["inventory"]+distributor_dict["inc_inventory"]

    def _get_good_type_and_idx(self, id):
        """ 
        Return what the index of a good would be 
        if it were in an array only containing goods of the same type 
        """
        n_produced_goods = self.settings["n_goods"]
        if id >= 2*n_produced_goods:
            # machine
            return "machine", id-2*n_produced_goods
        elif id >= n_produced_goods:
            # consumer good
            return "consumer_good", id-n_produced_goods
        else:
            return "production_good", id

    def _get_dist_key(self, dist_id):
        return dist_id - self.settings["n_producers"]

    def _get_group_dict_and_id(self, dic):
        client = dic.get("client")
        if client is None or (client != "Producer" and client != "Distributor"):
            return {}, -1

        raw_id = dic["id"]
        group_dict = self.producers if client == "Producer" else self.distributors
        real_id = raw_id if client == "Producer" else self._get_dist_key(raw_id)

        return group_dict, real_id

    def record_demand_signal(self, dic):
        prod_id = dic["product_id"]
        group_dict, real_id = self._get_group_dict_and_id(dic)
        if real_id == -1:
            return

        group_dict[real_id]["tracked_inputs"][prod_id] = True
        group_dict[real_id]["demand_signals"][prod_id] = dic["demand"]
        group_dict[real_id]["reorder_thresholds"][prod_id] = dic["reorder_threshold"]

    def record_resupply_rate(self, dic):
        prod_id = dic["product_id"]
        group_dict, real_id = self._get_group_dict_and_id(dic)
        if real_id == -1:
            return

        prod_id = dic["product_id"]
        group_dict[real_id]["resupply_rates"][prod_id] = dic["resupply_rate"]
        group_dict[real_id]["resupply_deficits"][prod_id] = dic["resupply_deficit"]

    def record_stalled_plan(self, dic):
        plan_id = dic["plan_id"]
        product_id = dic["product_id"]
        missing_resource_id = dic["missing_resource"]
        deficit = dic["deficit"]
        self.stalled_plan_casualties[product_id].add(plan_id)
        self.stalled_plan_causes[missing_resource_id][plan_id] = deficit

    def record_stallage_resolved(self, dic):
        plan_id = dic["plan_id"]
        product_id = dic["product_id"]

        self.stalled_plan_casualties[product_id].discard(plan_id)

        for plan_deficits in self.stalled_plan_causes.values():
            plan_deficits.pop(plan_id, None)

    def get_plan_stallage_deficits(self):
        return [
            sum(plan_deficits.values())
            for plan_deficits in self.stalled_plan_causes.values()
        ]

    def record_start_plan_stalled(self, dic):
        plan_id = dic["plan_id"]
        product_id = dic["product_id"]
        missing_resource_id = dic["missing_resource"]
        deficit = dic["deficit"]

        self.start_plan_stall_casualties[product_id].add(plan_id)
        self.start_plan_stall_causes[missing_resource_id][plan_id] = deficit


    def record_start_plan_stallage_resolved(self, dic):
        plan_id = dic["plan_id"]
        product_id = dic["product_id"]

        self.start_plan_stall_casualties[product_id].discard(plan_id)

        for plan_deficits in self.start_plan_stall_causes.values():
            plan_deficits.pop(plan_id, None)

    def record_drafting_failure(self, dic):
        casualty_id = dic["product_id"]
        reason = dic["reason"]
        if reason == "insufficient_workers":
            self.drafting_failures_casualties_from_workers[casualty_id] += 1
            self.drafting_failures_casualties[casualty_id] += 1
        elif reason == "insufficient_resources":
            self.drafting_failures_casualties[casualty_id] += 1
            missing_products_str_list = dic["missing_products"].split(",")
            missing_product_ids = [
                int(prod) for prod in missing_products_str_list if prod
            ]
            for product_id in missing_product_ids:
                self.drafting_failures_causes_resources[product_id] += 1
