#include <fstream>
#include <vector>

#include "serialization.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <svg.pb.h>

svg_serialize::Color SerializeColor(const svg::Color& other){
    svg_serialize::Color color;
    switch (other.index()) {
    case 1:
        color.set_name(std::get<std::string>(other));
        break;
    case 2: {
        const svg::Rgb& r_color = std::get<svg::Rgb>(other);
        color.mutable_rgb()->set_red(r_color.red);
        color.mutable_rgb()->set_green(r_color.green);
        color.mutable_rgb()->set_blue(r_color.blue);
        break;
    }
    case 3:{
        const svg::Rgba& r_color = std::get<svg::Rgba>(other);
        color.mutable_rgba()->set_red(r_color.red);
        color.mutable_rgba()->set_green(r_color.green);
        color.mutable_rgba()->set_blue(r_color.blue);
        color.mutable_rgba()->set_alpha(r_color.opacity);
        break;
    }
    default:
        break;
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

void serialize::Serialize(const std::string &path, const catalogue::TransportCatalogue& t_catalogue, const renderer::MapRenderer& t_renderer, const router::TransportRouter& t_router)
{
    std::ofstream outstream(path, std::ios_base::binary);
    if (!outstream)
        return;

    const auto& t_stops = t_catalogue.GetStops();

    transport_catalogue_serialize::Catalogue catalogue;
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

    // RENDERER

    auto settings = catalogue.mutable_renderer();

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
    settings->SerializePartialToOstream(&outstream);

    // ROUTER

    auto router = catalogue.mutable_router();

    router->set_wait_time(t_router.GetBusWaitTime());
    router->set_velocity(t_router.GetBusVelocity());

    auto graph = router->mutable_graph();
    auto t_graph = t_router.GetGraph();

    for(size_t i = 0; i < t_graph.GetEdgeCount(); ++i){
        auto edge = graph->add_edges();
        const auto& t_edge = t_graph.GetEdge(i);
        edge->set_index(i);
        edge->set_from(t_edge.from);
        edge->set_to(t_edge.to);
        edge->set_weight(t_edge.weight);

        const auto& t_buses = t_router.GetBuses();
        const auto& t_route_param = t_router.GetRouteParams(i);
        auto distance = std::distance(t_buses.begin(), std::find(t_buses.begin(), t_buses.end(), t_route_param.bus));
        edge->set_bus(distance);
        edge->set_span_count(t_route_param.span_count);
        edge->set_time(t_route_param.time);
    }

    catalogue.SerializeToOstream(&outstream);
}

void serialize::Deserialize(const std::string &path, catalogue::TransportCatalogue& t_catalogue, renderer::MapRenderer& t_renderer, router::TransportRouter& t_router)
{
    std::ifstream instream(path, std::ios_base::binary);
    if (!instream)
        return;

    transport_catalogue_serialize::Catalogue catalogue;
    catalogue.ParseFromIstream(&instream);

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

    // RENDERER
    {
        auto settings = catalogue.renderer();

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

    // ROUTER
    {
        auto settings = catalogue.router();

        t_router.SetBusWaitTime(settings.wait_time());
        t_router.SetBusVelocity(settings.velocity());

        t_router.SetStops(t_catalogue.GetStops());
        t_router.SetBuses(t_catalogue.GetBuses());

        auto t_graph = graph::DirectedWeightedGraph<double>(t_router.GetStops().size());
        for (size_t i = 0; i < settings.graph().edges_size(); ++i){
            auto edge = settings.graph().edges(i);
            t_graph.AddEdge({edge.from(),
                             edge.to(),
                             edge.weight()});
            t_router.SetRouteParams(i, {t_router.GetStops().at(edge.from()),
                                        t_router.GetStops().at(edge.to()),
                                        t_router.GetBuses().at(edge.bus()),
                                        edge.span_count(),
                                        edge.time()});
        }

        t_router.GetGraph() = t_graph;
    }
}

