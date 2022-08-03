#pragma once

#include <stdlib.h>
#include <string_view>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

namespace serialize{

void SerializeCatalogue(transport_catalogue_serialize::Catalogue &Catalogue, const catalogue::TransportCatalogue& t_catalogue);
void SerializeRenderer(transport_catalogue_serialize::Catalogue &Catalogue, const renderer::MapRenderer& t_renderer);
void SerializeRouter(transport_catalogue_serialize::Catalogue &Catalogue, const router::TransportRouter& t_router);

void DeserializeCatalogue(const transport_catalogue_serialize::Catalogue &Catalogue, catalogue::TransportCatalogue& t_catalogue);
void DeserializeRenderer(const transport_catalogue_serialize::Catalogue &Catalogue, renderer::MapRenderer& t_renderer);
void DeserializeRouter(const transport_catalogue_serialize::Catalogue &Catalogue, router::TransportRouter& t_router);

}   // namespace serialize
