#pragma once

#include <matplot/matplot.h>

#include "../extern/robin_hood/robin_hood.h"

struct PlotItem {
        std::string key;
        std::initializer_list<float> color;

        bool hidden;

        std::vector<double> data;

        PlotItem * hide();
};

class PlotHandler {
    private:
        int width;
        int height;

        std::string title;
        std::string xlabel;
        std::string ylabel;

        matplot::figure_handle figure;
        std::vector<double> x;
        robin_hood::unordered_map<std::string, PlotItem> y;

    public:
        PlotHandler(int width, int height, std::string title, std::string xlabel,
            std::string ylabel);

        void add_x(double x);
        PlotItem * define(std::string key, std::initializer_list<float> color);
        void add(std::string key, double y);
        void plot();
        void save(std::string path);
        void show();
};