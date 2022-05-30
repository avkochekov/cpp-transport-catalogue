#pragma once

#include <iostream>

#include "json.h"
#include "request_handler.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using catalogue::TransportCatalogue;

svg::Color Render_Color(const json::Node& node);
void Base_Request(const json::Node &node, TransportCatalogue &catalogue);
void Stat_Request(const json::Node &node, TransportCatalogue &catalogue, MapRenderer& renderer, std::ostream &stream);
void Render_Settings_Request(const json::Node &node, MapRenderer &render);
void Render_Map_Request(const TransportCatalogue &catalogue, MapRenderer &render);
//    FillCatalogue()
