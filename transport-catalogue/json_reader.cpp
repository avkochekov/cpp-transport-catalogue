#include "json_reader.h"
#include "json_builder.h"

#include <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

JsonReader::JsonReader(RequestHandler &handler)
    : handler{handler} {
}

void JsonReader::ParseRequest(std::istream &input, std::ostream &output)
{
    using namespace json;
    Document doc = json::Load(input);
    const json::Node &node = doc.GetRoot();
    const json::Dict &requests = node.AsDict();

    BaseRequestHandler(requests.at("base_requests"));
    RenderSettingsRequestHandler(requests.at("render_settings"));
    StatRequestHandler(requests.at("stat_requests"), output);
}


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

void JsonReader::BaseRequestHandler(const json::Node &node)
{
    using namespace json;
    std::deque<Node> bus_nodes;
    std::deque<Node> stop_nodes;

    for (const Node &node : node.AsArray()){
        auto &dict = node.AsDict();
        if (dict.at("type").AsString() == "Bus"){
            bus_nodes.push_back(dict);
        } else if (dict.at("type").AsString() == "Stop"){
            stop_nodes.push_back(dict);
        } else {
            throw std::invalid_argument("JsonReader: invalid request type");
        }
    }

    for (const Node &stop_node : stop_nodes){
        const auto &stop = stop_node.AsDict();
        handler.AddStop(stop.at("name").AsString(),
                        stop.at("latitude").AsDouble(),
                        stop.at("longitude").AsDouble());
    }

    for (const Node &stop_node : stop_nodes){
        const auto &stop = stop_node.AsDict();
        const auto &from_stop = stop.at("name").AsString();
        for (const auto &[to_stop, distance_node] : stop.at("road_distances").AsDict()){
            handler.AddDistanceBetweenStops(from_stop, to_stop, distance_node.AsDouble());
        }
    }

    for (const Node &bus_node : bus_nodes){
        const auto &bus = bus_node.AsDict();

        const auto &stops_node = bus.at("stops").AsArray();
        std::vector<std::string> stops(stops_node.size());

        std::transform(stops_node.begin(), stops_node.end(),
                       stops.begin(),
                       [](const Node &stop_node){ return stop_node.AsString(); });

        handler.AddBus(bus.at("name").AsString(),
                       bus.at("is_roundtrip").AsBool() ? RouteType::Circle : RouteType::Linear,
                       stops);
    }
}

void JsonReader::StatRequestHandler(const json::Node &node, std::ostream &stream)
{
    using namespace json;

    const auto &nodes = node.AsArray();

    Builder builder;
    builder.StartArray();

    for (const Node &node : nodes){
        auto &dict = node.AsDict();

        if (dict.at("type").AsString() == "Bus"){
            builder.Value(StatRequestBus(dict).AsDict());
        } else if (dict.at("type").AsString() == "Stop"){
            builder.Value(StatRequestStop(dict).AsDict());
        } else if (dict.at("type").AsString() == "Map"){
            builder.Value(StatRequestMap(dict).AsDict());
        } else {
            throw std::invalid_argument("JsonReader: invalid request type");
        }
    }
    builder.EndArray();
    Print(Document(builder.Build()), stream);
}

void JsonReader::RenderSettingsRequestHandler(const json::Node &node)
{
    const auto &nodes = node.AsDict();

    MapRenderSettings settings;
    settings.width = nodes.at("width").AsDouble();
    settings.height = nodes.at("height").AsDouble();
    settings.padding = nodes.at("padding").AsDouble();
    settings.line_width = nodes.at("line_width").AsDouble();
    settings.stop_radius = nodes.at("stop_radius").AsDouble();
    settings.bus_label_font_size = nodes.at("bus_label_font_size").AsInt();
    settings.stop_label_font_size = nodes.at("stop_label_font_size").AsInt();
    settings.underlayer_width = nodes.at("underlayer_width").AsDouble();
    settings.underlayer_color = RenderColor(nodes.at("underlayer_color"));

    const auto &bus_label_offset = nodes.at("bus_label_offset").AsArray();
    settings.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};

    const auto &stop_label_offset = nodes.at("stop_label_offset").AsArray();
    settings.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    const auto &color_palette = nodes.at("color_palette").AsArray();
    settings.color_palette.reserve(color_palette.size());
    for(const auto &color_node : color_palette){
        settings.color_palette.push_back(RenderColor(color_node));
    }

    handler.SetRendererSettings(settings);
}

Node JsonReader::StatRequestBus(const json::Dict &dict)
{
    using namespace json;

    Builder builder;
    builder.StartDict();
    builder.Key("request_id").Value(dict.at("id").AsInt());
    const auto &info = handler.GetBusStat(dict.at("name").AsString());
    if (info == std::nullopt){
        builder.Key("error_message").Value("not found");
    } else{
        builder.Key("curvature").Value(info->curvature);
        builder.Key("route_length").Value(info->route_length);
        builder.Key("stop_count").Value(static_cast<int>(info->stops_on_route));
        builder.Key("unique_stop_count").Value(static_cast<int>(info->unique_stops));
    }
    builder.EndDict();
    return builder.Build();
}

Node JsonReader::StatRequestStop(const json::Dict &dict)
{
    using namespace json;

    Builder builder;
    builder.StartDict();
    builder.Key("request_id").Value(dict.at("id").AsInt());
    const auto &info = handler.GetStopStat(dict.at("name").AsString());
    if (info == std::nullopt){
        builder.Key("error_message").Value("not found");
    } else{
        const auto &buses = info->buses;
        Array buses_array(buses.size());
        std::transform(buses.begin(), buses.end(),
                       buses_array.begin(),
                       [](const auto &value){ return Node{value}; });
        builder.Key("buses").Value(buses_array);
    }
    builder.EndDict();
    return builder.Build();
}

Node JsonReader::StatRequestMap(const json::Dict &dict)
{
    using namespace json;

    Builder builder;
    builder.StartDict();
    builder.Key("request_id").Value(dict.at("id").AsInt());
    std::ostringstream ostream;
    handler.RenderMap(ostream);
    builder.Key("map").Value(ostream.str());
    builder.EndDict();
    return builder.Build();
}
