import random
import numpy as np

# RANDOM EVENT SYSTEM 
# This module introduces random macroeconomic and natural
# events into the capitalist economy simulation.

# Model assumptions:
#   - 1 timestep = 1 quarter (3 months)
#   - 4 steps = 1 year
#   - Simulation runs 40 steps (10 years)

# Each event:
# - Has a duration in quarters
# - Has a probability of occurring per quarter
# - Alters key economic parameters (p, q, A, l, L, kappa)

# This system allows the model to simulate global shocks,
# localized events, and positive developments over time.

class RandomEvent:
    def __init__(self, name, category, description, duration, effects, base_chance):
        # Parameters
        # name: str
        #     Name of the event (e.g. "Financial Crisis")
        # category: str
        #     Type of event (Natural, Economic, Social, etc.)
        # description: str
        #     One-line summary of its real-world mechanism.
        # duration: int
        #     Duration in quarters (use -1 for permanent effects).
        # effects: dict
        #     Multipliers applied to economy variables.
        #     Example: {"p": 1.15, "q": 0.8, "L": 0.9}
        # base_chance: float
        #     Probability (0–1) of the event occurring each quarter.
        #     Calculated by 1/4N, since 4 quarters in a year, N is a year 
        #     Rounded to 3 decimal places. 
        
        self.name = name
        self.category = category
        self.description = description
        self.duration = duration
        self.effects = effects
        self.base_chance = base_chance
        self.active = False
        self.remaining_steps = 0

# DEFINE MAJOR EVENTS (Durations in Quarters)
def load_event_table():
    return [
        # NATURAL EVENTS 
        RandomEvent(
            "Drought", "Natural Disaster",
            "Reduces crop yields and raises food/input prices.",
            duration=4,  # 1 year (seasonal cycle)
            effects={"p": 1.15, "q": 0.8},
            base_chance=round(1 / (4 * 5), 3)  # once every 5 years = 0.050
        ),
        RandomEvent(
            "Severe Storm / Tornado", "Natural Disaster",
            "Localized destruction of production capacity.",
            duration=1,  # 1 quarter (short-term)
            effects={"p": 1.25, "q": 0.7, "localized": True},
            base_chance=round(1 / (4 * 1), 3)  # once every year = 0.250
        ),
        RandomEvent(
            "Volcanic Eruption / Earthquake", "Natural Disaster",
            "Damages infrastructure and reduces available labor.",
            duration=3,  # around 9 months duration + recovery
            effects={"L": 0.9},
            base_chance=round(1 / (4 * 12), 3)  # once every 12 years = 0.021
        ),

        # ECONOMIC EVENTS 
        RandomEvent(
            "Financial Crisis", "Economic",
            "Credit freeze, market collapse, and reduced investment.",
            duration=8,  # around 2 years of duration + recovery
            effects={"kappa": 0.7, "p": 1.3, "q": 0.8},
            base_chance=round(1 / (4 * 10), 3)  # once every 10 years = 0.025
        ),
        RandomEvent(
            "Inflation Shock", "Economic",
            "Rapid monetary expansion raises prices sharply.",
            duration=3,  # around 9 months of duration + recovery 
            effects={"p": 1.25},
            base_chance=round(1 / (4 * 5), 3)  # once every 5 years = 0.050
        ),
        RandomEvent(
            "Recession", "Economic",
            "Falling demand and reduced output economy-wide.",
            duration=6,  # around 1.5 years of duration + recovery 
            effects={"p": 0.9, "q": 0.8},
            base_chance=round(1 / (4 * 3.5), 3)  # once every 3.5 years = 0.071
        ),

        # GEOPOLITICAL EVENTS 
        RandomEvent(
            "War / Conflict", "Geopolitical",
            "Severe disruption to labor, logistics, and prices.",
            duration=12,  # active conflict (3 years)
            effects={"L": 0.7, "p": 1.4, "q": 0.7},
            base_chance=round(1 / (4 * 20), 3),  # once every 20 years
        ),

        RandomEvent(
            "Post-War Recovery", "Geopolitical / Recovery",
            "Economic rebuilding and reintegration of labor after war.",
            duration=8,  # 2 years recovery
            effects={"L": 1.1, "p": 0.95, "q": 1.15},
            base_chance=0.0,  # not random - only follows War
        ),

        # SOCIAL EVENTS 
        RandomEvent(
            "Worker Strike", "Social",
            "Labor unavailable in selected sectors for one quarter.",
            duration=1,  # around 3 months of duration 
            effects={"l": 0.3, "localized": True},
            base_chance=round(1 / (4 * 1.5), 3)  # once every 1.5 years = 0.167
        ),
        RandomEvent(
            "Pandemic / Health Crisis", "Health / Social",
            "Labor shortages and logistics disruptions worldwide.",
            duration=10,  # around 2.5 years of duration + recovery 
            effects={"L": 0.8, "p": 1.15, "q": 0.9},
            base_chance=round(1 / (4 * 25), 3)  # once every 25 years = 0.010
        ),

        # POSITIVE EVENTS 
        RandomEvent(
            "Technological Innovation", "Positive Structural",
            "Improves efficiency and productivity permanently.",
            duration=-1,  # permanent structural change
            effects={"A": 0.9, "p": 0.95, "q": 1.1},
            base_chance=round(1 / (4 * 8), 3)  # once every 8 years = 0.031
        ),
        RandomEvent(
            "Foreign Investment Surge", "Positive Economic",
            "Capital inflow boosts capacity and lowers borrowing costs.",
            duration=4,  # around 1 year of duration 
            effects={"kappa": 1.1, "p": 0.95, "q": 1.1},
            base_chance=round(1 / (4 * 4), 3)  # once every 4 years = 0.063
        ),
    ]

# EVENT TRIGGER SYSTEM (Quarter-based)
# Decision on whether to trigger an event on a specific time step 
# Need to add a list in CapitalistEconomy.py to record ongoing events 
# to avoid having multiple of the same events happening on an economy 
# and end the event when the duration is reached. 
# Attempts to trigger new random events each timestep.
# Adds triggered events to economy.active_events.
def maybe_trigger_event(economy):
    for event in economy.events_catalog:

        # Skip if already active
        if event in economy.active_events:
            continue

        # Post-War Recovery only follows War
        if event.name == "Post-War Recovery":
            continue

        # Trigger probability
        if random.random() < event.base_chance:

            # Activate event
            event.active = True
            event.remaining_steps = event.duration if event.duration > 0 else -1
            event.applied_once = False  # reset permanent-event flag
            economy.active_events.append(event)

            # Mark link for War -> Recovery
            if event.name == "War / Conflict":
                for follow in economy.events_catalog:
                    if follow.name == "Post-War Recovery":
                        follow.follow_after = event

    # Handle expired temporary events
    ended = []
    for ev in economy.active_events:
        if ev.duration > 0 and ev.remaining_steps == 0:
            ended.append(ev)

    for ev in ended:
        economy.active_events.remove(ev)

        # War ended -> trigger Post-War Recovery
        for candidate in economy.events_catalog:
            if candidate.name == "Post-War Recovery":
                if getattr(candidate, "follow_after", None) == ev:
                    candidate.active = True
                    candidate.remaining_steps = candidate.duration
                    candidate.applied_once = False
                    economy.active_events.append(candidate)
