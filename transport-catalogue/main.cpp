#include <sstream>
#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

#include "test_queries.h"

using json::Document;

int main() {
//    std::istream &istream = std::cin;
    std::stringstream stream = Query3();

    std::istream &istream = stream;
    std::ostream &ostream = std::cout;

    TransportCatalogue catalogue;
    MapRenderer renderer;
    TransportRouter router;

    RequestHandler handler(catalogue, renderer, router);
    JsonReader reader(handler);
    reader.ParseRequest(istream, ostream);
}
