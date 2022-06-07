#include "map_renderer.h"

#include <cassert>
#include <execution>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
void renderer::MapRenderer::SetSettings(const MapRenderSettings &settings)
{
    this->settings = settings;
}

const renderer::MapRenderSettings &renderer::MapRenderer::GetSettings()
{
    return settings;
}

void renderer::MapRenderer::AddStopPoint(const std::string_view title, const svg::Point &position)
{
    using svg::Text;
    using svg::Circle;

    auto point = Circle()
            .SetCenter(position)
            .SetRadius(settings.stop_radius)
            .SetFillColor("white");

    auto text = Text()
            .SetData(std::string(title))
            .SetPosition(position)
            .SetOffset(settings.stop_label_offset)
            .SetFontSize(settings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetFillColor("black");
    auto text_bottom = Text(text)
            .SetData(std::string(title))
            .SetPosition(position)
            .SetOffset(settings.stop_label_offset)
            .SetFontSize(settings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontSize(settings.stop_label_font_size)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetFillColor(settings.underlayer_color)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    stop_points.push_back(std::move(point));
    stop_titles.push_back(std::move(text_bottom));
    stop_titles.push_back(std::move(text));
}

void renderer::MapRenderer::AddBusLine(const std::string_view title, std::vector<svg::Point> points, bool isLinear)
{
    assert(!points.empty());

    using svg::Text;
    using svg::Color;
    using svg::Polyline;

    Color color = settings.color_palette.at((color_index++) % settings.color_palette.size());
    auto text = Text()
            .SetData(std::string(title))
            .SetFillColor(color)
            .SetPosition(points.front())
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold");
    auto text_bottom = Text()
            .SetData(std::string(title))
            .SetFillColor(settings.underlayer_color)
            .SetPosition(points.front())
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetPosition(points.front())
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold");

    bus_titles.push_back(Text(text_bottom));
    bus_titles.push_back(Text(text));

    if ((points.front() != points.back()) && isLinear){
        bus_titles.push_back(Text(text_bottom).SetPosition(points.back()));
        bus_titles.push_back(Text(text).SetPosition(points.back()));
    }

    if (isLinear){
        points.resize(points.size() * 2 - 1);
        std::copy(std::execution::par,
                  points.begin(), points.begin() + points.size() / 2,
                  points.rbegin());
    }

    auto line = Polyline()
            .SetStrokeWidth(settings.line_width)
            .SetStrokeColor(color)
            .SetFillColor("none")
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (auto point : points)
        line.AddPoint(point);

    bus_lines.push_back(std::move(line));
}

void renderer::MapRenderer::Render(std::ostream &stream)
{
    using namespace svg;
    for (Polyline &line : bus_lines){
        doc.Add(std::move(line));
    }
    for (Text &text : bus_titles){
        doc.Add(std::move(text));
    }
    for (Circle &point : stop_points){
        doc.Add(std::move(point));
    }
    for (Text &text : stop_titles){
        doc.Add(std::move(text));
    }

    doc.Render(stream);
}
