# from parameters import Params
import numpy as np
from scipy.integrate import solve_ivp
from random_events import maybe_trigger_event

class CapitalistEconomy:
    """Basic capitalist economy"""
    def __init__(self, params, debug: bool = False):
        self.params = params 
        self.debug = debug
        self.n = self.params.A.shape[0]
        self.current_t = 0
        self.t = [0.0]
        # Simulation solves the equation dy/dt = f(t,y), where y is a vector of every relevant independent quantity dumped into one place
        # note what y consists of - all other numbers can be considered dependent variables and can be determined from just these
        self.y = np.concatenate([params.q0, params.p0, params.s0, params.l, np.array([params.m_w0]), np.array([params.L])])
        self.exo_supply_deduction = np.zeros(self.n) # carries a to-be-applied exogenous shock to supply

        # Time series record of the simulation 
        self.history = {
            "p": np.array([params.p0]),
            "q": np.array([params.q0]), 
            "s": np.array([params.s0]),
            "m_w": np.array([params.m_w0]), 
            "L": np.array([params.L]), 
            "w": np.array([params.w0]),
            "r": np.array([params.r0]),
            "l": np.array([params.l]),
            "event_snapshots": []  # list of active events per step
        }

        # dependent variables
        self.history["total_labor_employed"] = np.array([self._get_employment(params.q0, params.l)])
        b, c = self._get_consumption(params.m_w0, params.p0)
        self.history["b"], self.history["c"] = np.array([b]), np.array([c])
        values = self._get_values(self.params.A, self.params.l)
        self.history["values"] = np.array([values])
        self.history["wage_values"] = np.array([params.w0 * values])
        self.history["reserve_army_size"] = np.array([self.params.L - self.history["total_labor_employed"]])
        self.history["m_c"] = np.array([1-params.m_w0])
        val_ms, val_cc, surplus_val, e, value_rop = self._get_value_split(values, b, self.params.q0, self.params.A)
        self.history["values_ms"], self.history["cc_vals"], self.history["surplus_vals"], self.history["e"], self.history["value_rops"] = np.array([val_ms]), np.array([val_cc]), np.array([surplus_val]), np.array([e]), np.array([value_rop])
        epr, eq_p = self._get_equilibrium_info(params.p0, params.w0, params.A, params.l)
        self.history["epr"], self.history["epr_prices"] = np.array([epr]), np.array([eq_p])
        self.history["compos_of_capital"] = np.array([val_cc / val_ms])
        profit_rates = self._get_profit_rates(params.A, params.p0, params.w0, params.l)
        self.history["profit_rates"] = np.array([profit_rates])

        # calculate initial f(t,y) 
        self.dydt = self._get_dydt(self.params)
        
        # RANDOM EVENT SYSTEM 
        self.events_catalog = []          # to be loaded later by simulation
        self.active_events = []           # list of event objects currently active
        self.allow_random_events = False  # a toggle that enables random events during a limited post-equilibrium window
        self.event_steps_remaining = 0    # will be set to 40 once equilibrium is reached, counts down 40 steps after that. 
        self.detected_equilibrium = False # a one-time-latch that remains true once equilibrium is reached


    def step(self):
        if self.debug:
            print(f"[STEP DEBUG] t={self.current_t:.0f} | "
                f"allow_random_events={self.allow_random_events} | "
                f"event_steps_remaining={self.event_steps_remaining}")
        
        # EVENT MANAGEMENT (executed once per discrete time step) 
        if self.allow_random_events and self.event_steps_remaining > 0:
            maybe_trigger_event(self)  # may or may not trigger events
            self.event_steps_remaining -= 1

        # decrement durations 
        ended = []
        for ev in self.active_events:
            if ev.duration > 0:
                ev.remaining_steps -= 1
                if ev.remaining_steps <= 0:
                    ended.append(ev)

        # remove finished events
        for ev in ended:
            self.active_events.remove(ev)

        # record events for this timestep
        snapshot = [
            (ev.name, ev.remaining_steps if ev.duration > 0 else "permanent")
            for ev in self.active_events
        ]
        self.history["event_snapshots"].append(snapshot)
        
        # BUILD EVENT EFFECT MULTIPLIERS FOR THIS STEP
        # Reset multipliers to 1.0 each step
        self.event_effect_multipliers = {
            "p": 1.0,
            "q": 1.0,
            "l": 1.0,
            "L": 1.0,
            "A": 1.0,
            "kappa": 1.0
        }

        # Multiply in the effects of all active events
        for ev in self.active_events:
            for k, v in ev.effects.items():
                if k in self.event_effect_multipliers:
                    self.event_effect_multipliers[k] *= v
        
        t_eval = np.linspace(self.current_t, self.current_t+1, self.params.res+1)[1:]
        sol = solve_ivp(self.dydt, (float(self.current_t), float(self.current_t+1)), self.y,
                        method= "BDF", rtol= 1e-6, atol=1e-9,
                        t_eval=t_eval, max_step=1.0)
        if not sol.success or not np.all(np.isfinite(sol.y)):
            # Do something besides this mb iono
            print("Simulation failed.")
        self.exo_supply_deduction = np.zeros(self.n) # if there was an exogenous supply shock, it will have been applied by now, so set it back to zero

        # record state history for each solver sample within this step
        for i in range(self.params.res):
            self.y = sol.y[:,i]
            self._step_traj(self.y)
            self.current_t = t_eval[i]
            self.t.append(float(self.current_t))

        # recalculate f(t,y)
        self.dydt = self._get_dydt(self.params)
    
    def force_end_events(self):
        """Immediately end ALL active events."""
        self.active_events = []
        self.event_effect_multipliers = {
            "p": 1.0,
            "q": 1.0,
            "l": 1.0,
            "L": 1.0,
            "A": 1.0,
            "kappa": 1.0
        }

    def change_param(self, param_name, new_val):
        setattr(self.params, param_name, new_val)

    def _step_traj(self, y):
        q, p, s, l, m_w, L = self._split_state(y)
        self.history["q"] = np.append(self.history["q"], [q], axis=0)
        self.history["p"] = np.append(self.history["p"], [p], axis=0)
        self.history["s"] = np.append(self.history["s"], [s], axis=0)
        self.history["m_w"] = np.append(self.history["m_w"], m_w)
        self.history["L"] = np.append(self.history["L"], L)
        self.history["l"] = np.append(self.history["l"], l)

        w = self._get_hourly_wage(l, q, L)
        self.history["w"] = np.append(self.history["w"], w)
        r = self._get_interest_rate(m_w)
        self.history["r"] = np.append(self.history["r"], r)
        employment = self._get_employment(q, l)
        self.history["total_labor_employed"] = np.append(self.history["total_labor_employed"], employment)
        b, c = self._get_consumption(m_w, p)
        self.history["b"] = np.append(self.history["b"], [b], axis=0)
        self.history["c"] = np.append(self.history["c"], [c], axis=0)

        values = self._get_values(self.params.A, l)
        self.history["values"] = np.append(self.history["values"], [values], axis=0)
        self.history["wage_values"] = np.append(self.history["wage_values"], [w * values], axis=0)
        self.history["reserve_army_size"] = np.append(self.history["reserve_army_size"], L - employment)
        self.history["m_c"] = np.append(self.history["m_c"], 1-self.params.m_w0)

        val_ms, val_cc, surplus_val, e, value_rop = self._get_value_split(values, b, q, self.params.A)
        self.history["values_ms"] = np.append(self.history["values_ms"], val_ms)
        self.history["cc_vals"] = np.append(self.history["cc_vals"], val_cc)
        self.history["surplus_vals"] = np.append(self.history["surplus_vals"], surplus_val)
        self.history["e"] = np.append(self.history["e"], e)
        self.history["value_rops"] = np.append(self.history["value_rops"], value_rop)

        epr, eq_p = self._get_equilibrium_info(p, w, self.params.A, l)
        self.history["epr"] = np.append(self.history["epr"], epr)
        self.history["epr_prices"] = np.append(self.history["epr_prices"], [eq_p], axis=0)

        self.history["compos_of_capital"] = np.append(self.history["compos_of_capital"], val_cc / val_ms)
        profit_rates = self._get_profit_rates(self.params.A, p, w, l)
        self.history["profit_rates"] = np.append(self.history["profit_rates"], [profit_rates], axis=0)

    def check_supply(self):
        s = self.y[2*self.n:3*self.n]
        return s

    def receive_offer(self):
        # fill in with whatever seems reasonable
        pass

    def make_offer(self):
        # fill in with whatever seems reasonable
        pass

    def exo_supply_shock(self, deduction):
        q, p, s, l, m_w, L = self._split_state(self.y)
        s -= deduction
        self.y = np.concatenate([q, p, s, l, np.array([m_w]), np.array([L])])
        self.exo_supply_deduction = deduction

    def _trade(self):
        # fill in with whatever seems reasonable
        pass

    def _split_state(self, y):
        n = self.n
        q = y[0:n]
        p = y[n:2*n]
        s = y[2*n:3*n]
        l = y[3*n:4*n]
        m_w = y[4*n]
        L  = y[4*n+1]
        return q, p, s, l, m_w, L

    def _get_employment(self, q, l):
        return q@l
    
    def _get_dydt(self, params):
        A = params.A
        b_bar = params.b_bar
        c_bar = params.c_bar
        kappa = params.kappa
        eta = params.eta
        alpha_w = params.alpha_w
        alpha_c = params.alpha_c
        alpha_L = params.alpha_L
        alpha_l = params.alpha_l

        # Creates the right hand side of the equation dy/dt = f(t,y)
        def rhs(t: float, y: np.ndarray) -> np.ndarray:
            n = self.n
            q = y[0:n]
            p = y[n:2*n]
            s = y[2*n:3*n]
            l = y[3*n:4*n]
            m_w = float(y[4*n])
            L = float(y[4*n+1])
            
            # Apply event effect multipliers 
            eff = getattr(self, "event_effect_multipliers", None)

            if eff is not None:
                q_eff = q * eff["q"]
                p_eff = p * eff["p"]
                l_eff = l * eff["l"]
                L_eff = L * eff["L"]
                A_eff = A * eff["A"]
                kappa_eff = kappa * eff["kappa"]
            else:
                q_eff = q
                p_eff = p
                l_eff = l
                L_eff = L
                A_eff = A
                kappa_eff = kappa
            
            delta_L = alpha_L * L_eff
            delta_l = -1 * alpha_l * l_eff

            w = self._get_hourly_wage(l_eff, q_eff, L_eff)
            r = self._get_interest_rate(m_w)

            # computing total demand
            p_dot_b = max(p_eff.dot(b_bar), 1e-12)
            p_dot_c = max(p_eff.dot(c_bar), 1e-12)

            total_labor = l_eff.dot(q_eff)
            delta_m_w = total_labor * w - alpha_w * m_w

            b = (b_bar * alpha_w * m_w) / p_dot_b
            c = (c_bar * alpha_c * (1.0 - m_w)) / p_dot_c
            total_demand = A_eff @ q_eff + b + c

            # from total demand, we obtain change in supply
            delta_s = q_eff - total_demand - self.exo_supply_deduction

            # from change in supply, we obtain change in prices
            s_safe = np.maximum(s, params.s_floor)
            delta_p = -eta * delta_s * (p_eff / s_safe)

            # compute profit
            unit_cost = A_eff.T @ p_eff + w * l_eff
            revenue = p_eff * total_demand
            total_cost = unit_cost * (1.0 + r) * q_eff
            profit = revenue - total_cost

            # from profit, we obtain change in output
            denom = np.maximum(unit_cost * (1.0 + r), 1e-12)
            delta_q = kappa_eff * (profit / denom)

            return np.concatenate(
                [delta_q, delta_p, delta_s, delta_l,
                np.array([delta_m_w]), np.array([delta_L])]
            )

        return rhs

    # helper methods of various kinds
    def _get_hourly_wage(self, l: np.ndarray, q: np.ndarray, L: float) -> float:
        """Returns the current hourly wage given the current level of employment and size of reserve army"""
        initial_employment = float(self.params.l.dot(self.params.q0))
        employment = float(l.dot(q))
        denom = max(L - employment, self.params.eps_u)
        num = max(self.params.L - initial_employment, self.params.eps_u)
        return self.params.w0 * (num / denom) ** self.params.eta_w

    def _get_interest_rate(self, m_w: float) -> float:
        """Returns the current interest rate given the current capitalist savings"""
        denom = max(1.0 - float(m_w), self.params.eps_m)
        num = max(1.0 - self.params.m_w0, self.params.eps_m)
        return self.params.r0 * (num / denom) ** self.params.eta_r

    def _get_consumption(self, m_w, p):
        b = (self.params.alpha_w * m_w)/(p.dot(self.params.b_bar))*self.params.b_bar
        c = (self.params.alpha_c * (1-m_w))/(p.dot(self.params.c_bar))*self.params.c_bar
        return b, c

    def _get_values(self, A, l):
        return np.linalg.inv(np.eye(self.n) - A.T)@l

    def _get_value_split(self, values, b, q, A):
        total_value = q.dot(values)
        val_ms = b.dot(values)
        val_cc = values.dot(A@q)
        surplus_val = total_value - val_ms - val_cc
        e = surplus_val / val_ms
        value_rop = surplus_val / (val_ms + val_cc)
        return val_ms, val_cc, surplus_val, e, value_rop

    def _get_equilibrium_info(self, p, w, A, l):
        hourly_b = w / (p.dot(self.params.b_bar)) * self.params.b_bar
        r_hat, eq_p = self._get_pf_info(A, l, hourly_b)
        r_hat = np.real(r_hat)
        epr = 1/r_hat - 1
        scalar = np.linalg.norm(p) / np.linalg.norm(eq_p)
        eq_p = scalar * eq_p
        return epr, eq_p

    def _get_pf_info(self, A, l, b):
        M = A+np.linalg.outer(b,l)
        evals, evecs = np.linalg.eig(M.T)
        index = np.argmax(evals.real)
        r_hat = evals[index]
        p = evecs[:,index].real
        if p[0] < 0:  p *= -1
        return r_hat, p

    def _get_profit_rates(self, A, p, w, l):
        unit_costs = A.T@p + w*l
        unit_profit_rates = p - unit_costs
        for i in range(self.n):
            unit_profit_rates[i] /= unit_costs[i]
        return unit_profit_rates
