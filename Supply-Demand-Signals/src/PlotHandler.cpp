#include "PlotHandler.hpp"

#include <utility>

#include "matplot/freestanding/axes_functions.h"
#include "matplot/util/keywords.h"

PlotItem * PlotItem::hide() {
    hidden = true;
    return this;
}

PlotHandler::PlotHandler(int width, int height, std::string title, std::string xlabel,
    std::string ylabel)
    : width(width), height(height), title(title), xlabel(xlabel), ylabel(ylabel) {}

void PlotHandler::add_x(double x) { this->x.push_back(x); }

PlotItem * PlotHandler::define(std::string key, std::string color) {
    y[key] = PlotItem{
        .key = key,
        .color = color,
        .hidden = false,
        .data = std::vector<double>(),
    };

    return &y[key];
}

void PlotHandler::add(std::string key, double y) {
    if (this->y.count(key) == 0) {
        throw "Key " + key + " not registered. Use define() to create it first.";
    }

    this->y[key].data.push_back(y);
}

void PlotHandler::plot() {
    figure = matplot::figure(true);
    figure->size(width, height);

    matplot::hold(matplot::on);

    for (auto [key, val] : y) {
        if (val.hidden) continue;

        auto plot = matplot::plot(x, val.data);
        plot->display_name(key);
        plot->color(val.color);
        plot->line_width(3);
    }

    matplot::xlabel(xlabel);
    matplot::ylabel(ylabel);
    matplot::title(title);
    matplot::grid(matplot::on);
    matplot::legend(true)->location(matplot::legend::general_alignment::topright);
}

void PlotHandler::save(std::string path) { figure->save(path); }

void PlotHandler::show() { figure->show(); }