#pragma once

#include <iostream>

#include "json.h"
#include "request_handler.h"
#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using catalogue::TransportCatalogue;
using json::Node;

class JsonReader{
    RequestHandler &handler;

    Node StatRequestBus(const json::Dict &dict);
    Node StatRequestStop(const json::Dict &dict);
    Node StatRequestMap(const json::Dict &dict);
public:
    JsonReader(RequestHandler &handler);

    void ParseRequest(std::istream &input = std::cin, std::ostream &output = std::cout);

    void BaseRequestHandler(const json::Node &node);
    void StatRequestHandler(const json::Node &node, std::ostream &stream);
    void RenderSettingsRequestHandler(const json::Node &node);
};
