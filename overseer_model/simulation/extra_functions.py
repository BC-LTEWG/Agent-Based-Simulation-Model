import logging

from numpy import long
logger = logging.getLogger(__name__)
from copy import deepcopy

def set_labels(plot_dict, label_prefix, n_produced, n_machines):
    labels = [f"{label_prefix} Sector P{i+1}" for i in range(n_produced)]
    labels += [f"{label_prefix} Distribution"]
    labels += [f"{label_prefix} Sector M{i+1}" for i in range(n_machines)]
    plot_dict["labels"] = labels

def format_plot_config(params, plotting_data):
    data = deepcopy(plotting_data)

    n_produced = params.N_g
    n_machines = params.N_m

    to_change = {
        "wealth": {
            "sectoral_average_account": "Average Account in"
        },
        "employment": {
            "sectoral_employment": "Workers Employed in",
            "long_run_sectoral_employment": "Long-Run Employment in",
            "equilibrium_employment": "Minimum Necessary Employment in",
            "transfer_requests": "Transfer Requests from",
            "surplus_labor_ratios": "Ratio of Minimal to Actual Employment in"
        },
        "busyness": {
            "sectoral_busyness": "Busyness in"
        },
        "plans": {
            "drafting_failures": "Hourly Drafting Failures from"
        },
        "activity": {
            "sectoral_activity_levels": "Long Run Activity Level of",
            "equilibrium_activity_levels": "Min Hourly Output in"
        }
    }
    
    for cat_key in to_change:
        if cat_key not in data:
            continue
        plots_dict = data[cat_key]["plots"]
        for plot_to_change, prefix in to_change[cat_key].items():
            actual_plot_dict = plots_dict[plot_to_change]
            if "label_template" in actual_plot_dict:
                del actual_plot_dict["label_template"]
            set_labels(actual_plot_dict, prefix, n_produced, n_machines)

    return data
