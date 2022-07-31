#include <sstream>
#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using namespace std::literals;
using json::Document;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    std::istream &istream = std::cin;
    std::ostream &ostream = std::cout;

    TransportCatalogue catalogue;
    MapRenderer renderer;
    TransportRouter router;

    RequestHandler handler(catalogue, renderer, router);
    JsonReader reader(handler);
    auto requests = reader.ParseRequest(istream);

    if (mode == "make_base"sv) {
        if (requests.count("base_requests"))
            reader.BaseRequestHandler(requests.at("base_requests"));
        if (requests.count("render_settings"))
            reader.RenderSettingsRequestHandler(requests.at("render_settings"));
        if (requests.count("routing_settings"))
            reader.RoutingSettingsHandler(requests.at("routing_settings"));
        handler.Serialize(reader.SerializationSettings(requests.at("serialization_settings")));
    } else if (mode == "process_requests"sv) {
        handler.Deserialize(reader.SerializationSettings(requests.at("serialization_settings")));
        reader.StatRequestHandler(requests.at("stat_requests"), ostream);
    } else {
        PrintUsage();
        return 1;
    }
}
