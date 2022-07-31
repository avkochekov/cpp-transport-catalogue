#pragma once

#include <stdlib.h>
#include <string_view>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace serialize{
void Serialize(const std::string& path, const catalogue::TransportCatalogue& t_catalogue, const renderer::MapRenderer& t_renderer, const router::TransportRouter& t_router);
void Deserialize(const std::string& path, catalogue::TransportCatalogue& t_catalogue, renderer::MapRenderer& t_renderer, router::TransportRouter& t_router);
}   // namespace serialize
