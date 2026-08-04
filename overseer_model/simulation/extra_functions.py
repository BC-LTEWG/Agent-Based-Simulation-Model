import logging

from numpy import long
logger = logging.getLogger(__name__)
from copy import deepcopy

def format_plot_config(params, plotting_data):
    data = deepcopy(plotting_data)

    n_produced = params.N_c
    n_machines = params.N_m

    employment_plots = data["employment"]["plots"]
    sectoral_employment = employment_plots["sectoral_employment"]
    long_run_employment = employment_plots["long_run_sectoral_employment"]
    minimal_employment = employment_plots["equilibrium_employment"]

    to_change = [sectoral_employment, long_run_employment, minimal_employment]
    for plot_dict in to_change:
        del plot_dict["label_template"]
    
    sectoral_employment["labels"] = [f"Workers Employed in Sector P{i}" for i in range(n_produced)]
    sectoral_employment["labels"] += [f"Workers Employed in Distribution"]
    sectoral_employment["labels"] += [f"Workers Employed in Sector M{i}" for i in range(n_machines)]

    long_run_employment["labels"] = [f"Long-Run Employment in Sector P{i}" for i in range(n_produced)]
    long_run_employment["labels"] += [f"Long-Run Employment in Distribution"]
    long_run_employment["labels"] += [f"Long-Run Employment in Sector M{i}" for i in range(n_machines)]

    minimal_employment["labels"] = [f"Minimum Necessary Employment in Sector P{i}" for i in range(n_produced)]
    minimal_employment["labels"] += [f"Minimum Necessary Employment in Distribution"]
    minimal_employment["labels"] += [f"Minimum Necessary Employment in Sector M{i}" for i in range(n_machines)]
   
    busyness_plots = data["busyness"]["plots"]
    sectoral_busyness = busyness_plots["sectoral_busyness"]
    if sectoral_busyness.get("label_template") is not None:
        del sectoral_busyness["label_template"]
    sectoral_busyness["labels"] = [f"Busyness in Sector P{i}" for i in range(n_produced)]
    sectoral_busyness["labels"] += [f"Busyness in Distribution Sector"]
    sectoral_busyness["labels"] += [f"Busyness in Sector M{i}" for i in range(n_machines)]

    return data
