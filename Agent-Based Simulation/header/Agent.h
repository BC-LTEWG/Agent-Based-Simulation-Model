#pragma once

class Agent {
    virtual void on_time_step() = 0;
    virtual unsigned int get_id() = 0;
};
