#include "map_renderer.h"

#include <cassert>
#include <unordered_set>

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

void renderer::MapRenderer::RenderCatalogue(const catalogue::TransportCatalogue &catalogue, std::ostream& out = std::cout)
{
    using namespace svg;
    using svg::Circle;

    std::unordered_set<std::string> unique_stops;
    std::vector<std::string> buses = catalogue.GetBuses();
    std::unordered_map<std::string, std::vector<std::string>> buses_to_stops;
    for (const auto &bus : buses){
        auto bus_stops = *catalogue.GetBusStops(bus);
        unique_stops.merge(std::unordered_set<std::string>(bus_stops.begin(), bus_stops.end()));
        buses_to_stops[bus] = *catalogue.GetBusStops(bus);
    }
    std::sort(buses.begin(), buses.end());
    std::vector<std::string> stops(unique_stops.begin(), unique_stops.end());
    std::sort(stops.begin(), stops.end());

    std::deque<Coordinates> stops_coordinates;
    std::unordered_map<std::string_view, Coordinates*> stops_to_coordinates;
    std::for_each(unique_stops.begin(), unique_stops.end(),
                  [&stops_coordinates, &stops_to_coordinates, &catalogue](const std::string_view stop)
    {
        stops_coordinates.push_back(*catalogue.GetStopCoordinates(stop));
        stops_to_coordinates[stop] = &stops_coordinates.back();
    });

    const auto &render_settinds = GetSettings();
    SphereProjector projector(stops_coordinates.begin(),
                              stops_coordinates.end(),
                              render_settinds.width,
                              render_settinds.height,
                              render_settinds.padding);


    std::unordered_map<std::string, Point> stops_to_points;
    for (const auto &stop : unique_stops){
        stops_to_points[stop] = projector(*stops_to_coordinates.at(stop));
    }

    std::deque<Polyline> lines;
    std::deque<Text> buses_names;

    for (const auto &bus : buses){
        if (buses_to_stops.at(bus).empty())
            continue;

        auto stops = std::move(buses_to_stops.at(bus));

        std::vector<Point> stops_points(stops.size());
        std::transform(stops.begin(), stops.end(),
                       stops_points.begin(),
                       [&stops_to_points](auto &stop_name){ return stops_to_points[stop_name]; });

        AddBusLine(bus, stops_points, catalogue.GetBusType(bus) == Linear);
    }

    for (const auto &stop : stops){
        AddStopPoint(stop, stops_to_points.at(stop));
    }
    Render(out);
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
        std::copy(points.begin(), points.begin() + points.size() / 2,
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
    using svg::Polyline;
    using svg::Text;
    using svg::Circle;
    using svg::Text;

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
