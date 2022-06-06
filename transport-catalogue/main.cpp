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
    const json::Dict &requests = node.AsDict();

    TransportCatalogue catalogue;
    MapRenderer renderer;

    RequestHandler handler(catalogue, renderer);

    BaseRequestHandler(requests.at("base_requests"), handler);
    RenderSettingsRequestHandler(requests.at("render_settings"), renderer);
    StatRequestHandler(requests.at("stat_requests"), handler, ostream);
}
