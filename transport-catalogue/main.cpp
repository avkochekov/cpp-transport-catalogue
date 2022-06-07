#include <sstream>
#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using json::Document;

int main() {
    std::istream &istream = std::cin;
    std::ostream &ostream = std::cout;

    TransportCatalogue catalogue;
    MapRenderer renderer;

    RequestHandler handler(catalogue, renderer);
    JsonReader reader(handler);
    reader.ParseRequest(istream, ostream);
}
