#include <fstream>
#include <vector>

#include "serialization.h"

#include <map_renderer.pb.h>
#include <svg.pb.h>

svg_serialize::Color SerializeColor(const svg::Color& other){
    svg_serialize::Color color;
    if (std::holds_alternative<std::string>(other)){
        color.set_name(std::get<std::string>(other));
    } else if (std::holds_alternative<svg::Rgba>(other)){
        const svg::Rgba& r_color = std::get<svg::Rgba>(other);
        color.mutable_rgba()->set_red(r_color.red);
        color.mutable_rgba()->set_green(r_color.green);
        color.mutable_rgba()->set_blue(r_color.blue);
        color.mutable_rgba()->set_alpha(r_color.opacity);
    } else if (std::holds_alternative<svg::Rgb>(other)){
        const svg::Rgb& r_color = std::get<svg::Rgb>(other);
        color.mutable_rgb()->set_red(r_color.red);
        color.mutable_rgb()->set_green(r_color.green);
        color.mutable_rgb()->set_blue(r_color.blue);
    }
    return color;
}

svg::Color DeserializeColor(const svg_serialize::Color& other){
    svg::Color color;

    switch (other.value_case()) {
    case svg_serialize::Color::ValueCase::kName:
        color = other.name();
        break;
    case svg_serialize::Color::ValueCase::kRgb:
        color = svg::Rgb{
                static_cast<uint8_t>(other.rgb().red()),
                static_cast<uint8_t>(other.rgb().green()),
                static_cast<uint8_t>(other.rgb().blue())};
        break;
    case svg_serialize::Color::ValueCase::kRgba:
        color = svg::Rgba{
                static_cast<uint8_t>(other.rgba().red()),
                static_cast<uint8_t>(other.rgba().green()),
                static_cast<uint8_t>(other.rgba().blue()),
                other.rgba().alpha()};
        break;
    default:
        break;
    }
    return color;
}

void serialize::SerializeCatalogue(transport_catalogue_serialize::Catalogue& catalogue, const catalogue::TransportCatalogue &t_catalogue)
{
    const auto& t_stops = t_catalogue.GetStops();

    for (const std::string& t_stop : t_stops){
        auto stop = catalogue.add_stops();
        stop->set_name(t_stop);

        // Координаты остановки
        const auto& t_stop_coordinates = t_catalogue.GetStopCoordinates(t_stop);
        if (t_stop_coordinates){
            stop->mutable_coordinates()->set_latitude(t_stop_coordinates->lat);
            stop->mutable_coordinates()->set_longitude(t_stop_coordinates->lng);
        }

        // Дистанция между остановками
        for (const std::string& t_stop_to : t_stops){
            auto t_distance = t_catalogue.GetDistanceBetweenStops(t_stop, t_stop_to);
            if (t_distance == 0 || t_distance > catalogue::TransportCatalogue::maxRouteDistance)
                continue;

            auto distance = stop->add_distance();
            distance->set_length(t_distance);
            distance->set_stop(t_stop_to);
        }
    }

    const auto& t_buses = t_catalogue.GetBuses();
    for (const std::string& t_bus : t_buses){
        auto bus = catalogue.add_buses();
        bus->set_name(t_bus);
        bus->set_is_roundtrip(t_catalogue.GetBusType(t_bus) == RouteType::Roundtrip);
        auto t_bus_stops = t_catalogue.GetBusStops(t_bus);
        if (t_bus_stops){
            for(const std::string& t_bus_stop : *t_bus_stops){
                bus->add_stops(t_bus_stop);
            }
        }
    }
}

void serialize::SerializeRenderer(transport_catalogue_serialize::Catalogue& tc, const renderer::MapRenderer &t_renderer)
{
    auto settings = tc.mutable_renderer();

    const auto& r_settings = t_renderer.GetSettings();

    settings->set_width(r_settings.width);
    settings->set_height(r_settings.height);
    settings->set_padding(r_settings.padding);
    settings->set_line_width(r_settings.line_width);
    settings->set_stop_radius(r_settings.stop_radius);
    settings->set_underlayer_width(r_settings.underlayer_width);

    settings->set_bus_label_font_size(r_settings.bus_label_font_size);
    settings->set_stop_label_font_size(r_settings.stop_label_font_size);

    settings->mutable_bus_label_offset()->set_x(r_settings.bus_label_offset.x);
    settings->mutable_bus_label_offset()->set_y(r_settings.bus_label_offset.y);

    settings->mutable_stop_label_offset()->set_x(r_settings.stop_label_offset.x);
    settings->mutable_stop_label_offset()->set_y(r_settings.stop_label_offset.y);

    *settings->mutable_underlayer_color() = SerializeColor(r_settings.underlayer_color);

    for (const svg::Color& r_color : r_settings.color_palette){
        *settings->add_color_palette() = SerializeColor(r_color);
    }
}

void serialize::SerializeRouter(transport_catalogue_serialize::Catalogue& tc, const router::TransportRouter &t_router)
{
    auto settings = tc.mutable_router();

    settings->set_wait_time(t_router.GetBusWaitTime());
    settings->set_velocity(t_router.GetBusVelocity());

    const auto& t_buses = t_router.GetBuses();
    const auto& t_stops = t_router.GetStops();

    for(size_t i = 0; i < t_router.GetRouteParamsCount(); ++i){
        auto route = settings->add_routes();

        const auto& t_route_param = t_router.GetRouteParams(i);

        route->set_index(i);
        route->set_from(std::distance(t_stops.begin(), std::find(t_stops.begin(), t_stops.end(), t_route_param.from_stop)));
        route->set_to(std::distance(t_stops.begin(), std::find(t_stops.begin(), t_stops.end(), t_route_param.to_stop)));
        route->set_bus(std::distance(t_buses.begin(), std::find(t_buses.begin(), t_buses.end(), t_route_param.bus)));
        route->set_span_count(t_route_param.span_count);
        route->set_time(t_route_param.time);
    }

    auto graph = tc.mutable_graph();
    auto t_graph = t_router.GetGraph();
    for(size_t i = 0; i < t_graph.GetEdgeCount(); ++i){
        auto edge = graph->add_edges();
        const auto& t_edge = t_graph.GetEdge(i);
        edge->set_index(i);
        edge->set_from(t_edge.from);
        edge->set_to(t_edge.to);
        edge->set_weight(t_edge.weight);
    }
}

void serialize::DeserializeCatalogue(const transport_catalogue_serialize::Catalogue& catalogue, catalogue::TransportCatalogue &t_catalogue)
{
    size_t buses_size = catalogue.buses_size();
    size_t stops_size = catalogue.stops_size();

    std::vector<std::string_view> buses(buses_size);
    std::vector<std::string_view> stops(stops_size);

    for (size_t i = 0; i < stops_size; ++i){
        const auto& stop = catalogue.stops(i);
        stops[i] = stop.name();
        t_catalogue.AddStop(std::string(stop.name()),
                            {stop.coordinates().latitude(),
                             stop.coordinates().longitude()});
    }

    for (size_t i = 0; i < stops_size; ++i){
        const auto& stop = catalogue.stops(i);
        for (size_t d = 0; d < stop.distance_size(); ++d){
            const auto& distance = stop.distance(d);
            if (!distance.IsInitialized())
                continue;
            t_catalogue.AddDistance(stop.name(),
                                    distance.stop(),
                                    distance.length());
        }
    }

    for (size_t i = 0; i < buses_size; ++i){
        const auto& bus = catalogue.buses(i);
        buses[i] = bus.name();

        size_t bus_stops_size = bus.stops_size();
        std::vector<std::string> bus_stops(bus_stops_size);
        for (size_t i = 0; i < bus_stops_size; ++i){
            bus_stops[i] = bus.stops(i);
        }

        t_catalogue.AddBus(bus.name(),
                           bus.is_roundtrip() ? RouteType::Roundtrip : RouteType::Linear,
                           bus_stops);
    }
}

void serialize::DeserializeRenderer(const transport_catalogue_serialize::Catalogue& tc, renderer::MapRenderer &t_renderer)
{
    auto settings = tc.renderer();

    renderer::MapRenderSettings r_settings;

    r_settings.width = settings.width();
    r_settings.height = settings.height();
    r_settings.padding = settings.padding();
    r_settings.line_width = settings.line_width();
    r_settings.stop_radius = settings.stop_radius();
    r_settings.underlayer_width = settings.underlayer_width();

    r_settings.bus_label_font_size = settings.bus_label_font_size();
    r_settings.stop_label_font_size = settings.stop_label_font_size();

    r_settings.bus_label_offset = svg::Point{
            settings.bus_label_offset().x(),
            settings.bus_label_offset().y()};

    r_settings.stop_label_offset = svg::Point{
            settings.stop_label_offset().x(),
            settings.stop_label_offset().y()};

    r_settings.underlayer_color = DeserializeColor(settings.underlayer_color());

    size_t color_palette_size = settings.color_palette_size();
    r_settings.color_palette.resize(color_palette_size);
    for (size_t i = 0; i < color_palette_size; ++i){
        r_settings.color_palette[i] = DeserializeColor(settings.color_palette(i));
    }

    t_renderer.SetSettings(r_settings);
}

void serialize::DeserializeRouter(const transport_catalogue_serialize::Catalogue& tc, router::TransportRouter &t_router)
{
    auto settings = tc.router();

    size_t buses_size = tc.buses_size();
    size_t stops_size = tc.stops_size();

    std::vector<std::string> buses(buses_size);
    std::vector<std::string> stops(stops_size);

    for (size_t i = 0; i < buses_size; ++i){
        buses[i] = tc.buses(i).name();
    }

    for (size_t i = 0; i < stops_size; ++i){
        stops[i] = tc.stops(i).name();
    }

    t_router.SetBuses(std::move(buses));
    t_router.SetStops(std::move(stops));

    t_router.SetBusWaitTime(settings.wait_time());
    t_router.SetBusVelocity(settings.velocity());

    size_t routes_size = settings.routes_size();
    for (size_t i = 0; i < routes_size; ++i){
        auto route = settings.routes(i);
        t_router.SetRouteParams(route.index(),
                                {t_router.GetStops().at(route.from()),
                                 t_router.GetStops().at(route.to()),
                                 t_router.GetBuses().at(route.bus()),
                                 route.span_count(),
                                 route.time()});
    }

    auto graph = tc.graph();
    auto t_graph = graph::DirectedWeightedGraph<double>(t_router.GetStops().size());
    for (size_t i = 0; i < graph.edges_size(); ++i){
        auto edge = graph.edges(i);
        t_graph.AddEdge({edge.from(),
                         edge.to(),
                         edge.weight()});
    }

    t_router.GetGraph() = t_graph;
}
