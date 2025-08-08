#pragma once

#include <matplot/matplot.h>

#include "../extern/robin_hood/robin_hood.h"

struct PlotLine {
        std::string key;
        std::initializer_list<float> color;
        bool hidden = false;
        std::vector<double> y_data;

        PlotLine * hide();
};

class Plot {
    private:
        friend class PlotHandler;
        std::vector<double> x_data;
        robin_hood::unordered_map<std::string, PlotLine> lines;

    public:
        std::string title;
        std::string xlabel;
        std::string ylabel;
        bool hidden = false;

        Plot(std::string title, std::string xlabel, std::string ylabel);
        Plot() = default;

        void add_x(double x_val);
        PlotLine * define_line(std::string key, std::initializer_list<float> color);
        void add_y(std::string key, double y_val);
        Plot * hide();
};

class PlotHandler {
    private:
        int width;
        int height;

        matplot::figure_handle figure;
        robin_hood::unordered_map<std::string, Plot> plots;

    public:
        PlotHandler(int width, int height);

        Plot * create_plot(std::string name, std::string title, std::string xlabel,
            std::string ylabel);
        Plot * get_plot(std::string name);
        void plot();
        void save(std::string path);
        void show();
};