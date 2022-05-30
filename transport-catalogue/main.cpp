#include <sstream>
#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using json::Document;

int main() {
    std::istream &istream = std::cin;
    std::ostream &ostream = std::cout;

    Document doc = json::Load(istream);
    const json::Node &node = doc.GetRoot();
    const json::Dict &requests = node.AsMap();

    TransportCatalogue catalogue;
    MapRenderer renderer;

    RequestHandler handler(catalogue, renderer);

    Base_Request(requests.at("base_requests"), catalogue);
    Render_Settings_Request(requests.at("render_settings"), renderer);
    Stat_Request(requests.at("stat_requests"), catalogue, renderer, ostream);
}
