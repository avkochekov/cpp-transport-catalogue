#pragma once

#include <vector>
#include <deque>

#include "svg.h"
#include "transport_catalogue.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {
    struct MapRenderSettings{
        double width;                           ///< ширина изображения в пикселях. Вещественное число в диапазоне от 0 до 100000.
        double height;                          ///< высота изображения в пикселях. Вещественное число в диапазоне от 0 до 100000.
        double padding;                         ///< отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.
        double line_width;                      ///< толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.
        double stop_radius;                     ///< радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
        double underlayer_width;                ///< толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>. Вещественное число в диапазоне от 0 до 100000.
        unsigned bus_label_font_size;           ///< размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
        unsigned stop_label_font_size;          ///< размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
        svg::Point bus_label_offset;            ///< смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа double.
                                                ///< Задаёт значения свойств dx и dy SVG-элемента <text>. Элементы массива — числа в диапазоне от –100000 до 100000.
        svg::Point stop_label_offset;           ///< смещение названия остановки относительно её координат на карте.
                                                ///< Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Числа в диапазоне от –100000 до 100000.
        svg::Color underlayer_color;            ///< цвет подложки под названиями остановок и маршрутов. Формат хранения цвета будет ниже.
        std::vector<svg::Color> color_palette;  ///< цветовая палитра. Непустой массив.
    };

    class MapRenderer{
        svg::Document doc;
        std::deque<svg::Text> stop_titles;
        std::deque<svg::Circle> stop_points;

        std::deque<svg::Text> bus_titles;
        std::deque<svg::Polyline> bus_lines;

        size_t color_index = 0;
        MapRenderSettings settings;
    public:
        void SetSettings(const MapRenderSettings &settings);
        const MapRenderSettings &GetSettings();

        void RenderCatalogue(const catalogue::TransportCatalogue& catalogue, std::ostream &out);

        void AddStopPoint(const std::string_view title, const svg::Point &position);
        void AddBusLine(const std::string_view title, std::vector<svg::Point> points, bool isLinear);
        void Render(std::ostream&);
    };
}
