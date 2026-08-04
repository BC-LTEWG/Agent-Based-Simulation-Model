import logging
logger = logging.getLogger(__name__)
import os
from .parameters import Params
from .Aggregator import Aggregator
from typing import Tuple
import numpy as np
import subprocess
import json
from scipy.linalg import inv

def get_trajectories(params: Params, event_queue):
    logger.info(params.exe_path)
    aggregator = Aggregator(params.exe_path, params)
    try:
        sim_finished = False
        while not sim_finished:
            sim_finished = aggregator.step()
            traj = aggregator.get_data()
            yield traj
    finally:
        aggregator.collector.stop()
