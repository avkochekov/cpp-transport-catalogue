#include "json_reader.h"

#include <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
svg::Color RenderColor(const json::Node &node)
{
    if (node.IsString()){
        return node.AsString();
    } else if (node.IsArray()){
        const auto &color_array = node.AsArray();
        if (color_array.size() == 3){
            return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
        } else if (color_array.size() == 4){
            return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
        } else {
            throw std::invalid_argument("JsonReader: invalid request type");
        }
    } else {
        throw std::invalid_argument("JsonReader: invalid request type");
    }
}

void RenderMapRequestHandler(const TransportCatalogue &catalogue, MapRenderer &render)
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
                  [&catalogue, &stops_coordinates, &stops_to_coordinates](const std::string_view stop)
    {
        stops_coordinates.push_back(*catalogue.GetStopCoordinates(stop));
        stops_to_coordinates[stop] = &stops_coordinates.back();
    });

    SphereProjector projector(stops_coordinates.begin(),
                              stops_coordinates.end(),
                              render.width,
                              render.height,
                              render.padding);


    Document svg;
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
        std::transform(std::execution::par,
                       stops.begin(), stops.end(),
                       stops_points.begin(),
                       [&stops_to_points](auto &stop_name){ return stops_to_points[stop_name]; });

        render.AddBusLine(bus, stops_points, catalogue.GetBusType(bus) == Linear);
    }

    for (const auto &stop : stops){
        render.AddStopPoint(stop, stops_to_points.at(stop));
    }
}

void BaseRequestHandler(const json::Node &node, catalogue::TransportCatalogue &catalogue)
{
    using namespace json;
    std::deque<Node> bus_nodes;
    std::deque<Node> stop_nodes;

    for (const Node &node : node.AsArray()){
        auto &dict = node.AsMap();
        if (dict.at("type").AsString() == "Bus"){
            bus_nodes.push_back(dict);
        } else if (dict.at("type").AsString() == "Stop"){
            stop_nodes.push_back(dict);
        } else {
            throw std::invalid_argument("JsonReader: invalid request type");
        }
    }

    for (const Node &stop_node : stop_nodes){
        const auto &stop = stop_node.AsMap();
        catalogue.AddStop(stop.at("name").AsString(), {
                              stop.at("latitude").AsDouble(),
                              stop.at("longitude").AsDouble()});
    }

    for (const Node &stop_node : stop_nodes){
        const auto &stop = stop_node.AsMap();
        const auto &from_stop = stop.at("name").AsString();
        for (const auto &[to_stop, distance_node] : stop.at("road_distances").AsMap()){
            catalogue.AddDistance(from_stop, to_stop, distance_node.AsDouble());
        }
    }

    for (const Node &bus_node : bus_nodes){
        const auto &bus = bus_node.AsMap();

        const auto &stops_node = bus.at("stops").AsArray();
        std::vector<std::string> stops(stops_node.size());

        std::transform(stops_node.begin(), stops_node.end(),
                       stops.begin(),
                       [](const Node &stop_node){ return stop_node.AsString(); });

        catalogue.AddBus(bus.at("name").AsString(),
                         bus.at("is_roundtrip").AsBool() ? RouteType::Circle : RouteType::Linear,
                         stops);
    }
}

void StatRequestHandler(const json::Node &node, catalogue::TransportCatalogue &catalogue, MapRenderer &renderer, std::ostream& stream)
{
    using namespace json;

    const auto &nodes = node.AsArray();

    Array results;
    results.reserve(nodes.size());

    for (const Node &node : nodes){
        auto &dict = node.AsMap();

        Dict result;
        result["request_id"] = dict.at("id").AsInt();

        if (dict.at("type").AsString() == "Bus"){
            const auto &info = catalogue.GetBusInfo(dict.at("name").AsString());
            if (info == std::nullopt){
                result["error_message"] = "not found";
            } else{
                result["curvature"] = info->curvature;
                result["route_length"] = info->route_length;
                result["stop_count"] = static_cast<int>(info->stops_on_route);
                result["unique_stop_count"] = static_cast<int>(info->unique_stops);
            }
        } else if (dict.at("type").AsString() == "Stop"){
            const auto &info = catalogue.GetStopInfo(dict.at("name").AsString());
            if (info == std::nullopt){
                result["error_message"] = "not found";
            } else{
                const auto &buses = info->buses;
                Array buses_array(buses.size());
                std::transform(buses.begin(), buses.end(),
                               buses_array.begin(),
                               [](const auto &value){ return Node{value}; });
                result["buses"] = buses_array;
            }
        } else if (dict.at("type").AsString() == "Map"){
            RenderMapRequestHandler(catalogue, renderer);
            std::ostringstream ostream;
            renderer.Render(ostream);
            result["map"] = ostream.str();
        } else {
            throw std::invalid_argument("JsonReader: invalid request type");
        }

        results.push_back(result);
    }
    if (!results.empty())
        Print(Document(results), stream);
}

void RenderSettingsRequestHandler(const json::Node &node, renderer::MapRenderer &render)
{
    const auto &nodes = node.AsMap();

    render.width = nodes.at("width").AsDouble();
    render.height = nodes.at("height").AsDouble();
    render.padding = nodes.at("padding").AsDouble();
    render.line_width = nodes.at("line_width").AsDouble();
    render.stop_radius = nodes.at("stop_radius").AsDouble();
    render.bus_label_font_size = nodes.at("bus_label_font_size").AsInt();
    render.stop_label_font_size = nodes.at("stop_label_font_size").AsInt();
    render.underlayer_width = nodes.at("underlayer_width").AsDouble();
    render.underlayer_color = RenderColor(nodes.at("underlayer_color"));

    const auto &bus_label_offset = nodes.at("bus_label_offset").AsArray();
    render.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};

    const auto &stop_label_offset = nodes.at("stop_label_offset").AsArray();
    render.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    const auto &color_palette = nodes.at("color_palette").AsArray();
    render.color_palette.reserve(color_palette.size());
    for(const auto &color_node : color_palette){
        render.color_palette.push_back(RenderColor(color_node));
    }
}
