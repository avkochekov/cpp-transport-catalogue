#pragma once

#include <iostream>

#include "json.h"
#include "request_handler.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using catalogue::TransportCatalogue;

void BaseRequestHandler(const json::Node &node, TransportCatalogue &catalogue);
void StatRequestHandler(const json::Node &node, TransportCatalogue &catalogue, MapRenderer& renderer, std::ostream &stream);
void RenderSettingsRequestHandler(const json::Node &node, MapRenderer &render);
