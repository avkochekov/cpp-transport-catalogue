#pragma once

#include <iostream>

#include "json.h"
#include "request_handler.h"
#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using json::Node;
using json::Dict;

class JsonReader{
    RequestHandler &handler;

    Node StatRequestBus(const Dict &dict);
    Node StatRequestStop(const Dict &dict);
    Node StatRequestMap(const Dict &dict);
    Node StatRequestRoute(const Dict &dict);
public:
    JsonReader(RequestHandler &handler);

    void ParseRequest(std::istream &input = std::cin, std::ostream &output = std::cout);

    void BaseRequestHandler(const Node &node);
    void StatRequestHandler(const Node &node, std::ostream &stream);
    void RenderSettingsRequestHandler(const Node &node);
    void RoutingSettingsHandler(const Node &node);
};
