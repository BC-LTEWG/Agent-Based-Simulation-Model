#include "../include/PlotHandler.hpp"

#include <matplot/core/legend.h>
#include <matplot/freestanding/axes_functions.h>

PlotLine * PlotLine::hide() {
    this->hidden = true;
    return this;
}

Plot::Plot(std::string title, std::string xlabel, std::string ylabel) {
    this->title = title;
    this->xlabel = xlabel;
    this->ylabel = ylabel;
}

void Plot::add_x(double x_val) { x_data.push_back(x_val); }

PlotLine * Plot::define_line(std::string key, std::initializer_list<float> color) {
    lines[key] = {key, color, false, {}};
    return &lines[key];
}

void Plot::add_y(std::string key, double y_val) {
    if (lines.contains(key)) {
        lines.at(key).y_data.push_back(y_val);
    }
}

Plot * Plot::hide() {
    this->hidden = true;
    return this;
}

PlotHandler::PlotHandler(int width, int height) {
    this->width = width;
    this->height = height;
    this->figure = matplot::figure(true);
}

Plot * PlotHandler::create_plot(std::string name, std::string title, std::string xlabel,
    std::string ylabel) {
    plots[name] = Plot(title, xlabel, ylabel);
    return &plots[name];
}

Plot * PlotHandler::get_plot(std::string name) {
    if (plots.contains(name)) {
        return &plots.at(name);
    }
    return nullptr;
}

void PlotHandler::plot() {
    matplot::figure(this->figure);
    matplot::cla();  // Clear the entire figure
    figure->size(width, height);

    // Count only visible plots
    size_t num_visible_plots = 0;
    for (auto const & [name, plot] : plots) {
        if (!plot.hidden) {
            num_visible_plots++;
        }
    }

    if (num_visible_plots == 0) {
        return;
    }

    size_t grid_dim = static_cast<size_t>(ceil(sqrt(num_visible_plots)));

    size_t current_plot_idx = 0;
    for (auto const & [name, plot] : plots) {
        // Skip hidden plots
        if (plot.hidden) {
            continue;
        }

        matplot::subplot(grid_dim, grid_dim, current_plot_idx);
        matplot::cla();  // Clear this specific subplot
        matplot::hold(matplot::on);

        for (auto const & [key, line] : plot.lines) {
            if (!line.hidden) {
                auto p = matplot::plot(plot.x_data, line.y_data);
                p->color(line.color);
                p->display_name(line.key);
                p->line_width(2);
            }
        }
        matplot::title(plot.title);
        matplot::xlabel(plot.xlabel);
        matplot::ylabel(plot.ylabel);
        auto legend = matplot::legend(matplot::on);
        legend->location(matplot::legend::general_alignment::topleft);
        legend->opaque(false);
        current_plot_idx++;
    }
}

void PlotHandler::save(std::string path) {
    plot();
    matplot::save(path);
}

void PlotHandler::show() {
    plot();
    matplot::show();
}