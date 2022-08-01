#pragma once

#include <stdlib.h>
#include <string_view>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

namespace serialize{

void Serialize(transport_catalogue_serialize::Catalogue &Catalogue, const catalogue::TransportCatalogue& t_catalogue);
void Serialize(transport_catalogue_serialize::Catalogue &Catalogue, const renderer::MapRenderer& t_renderer);
void Serialize(transport_catalogue_serialize::Catalogue &Catalogue, const router::TransportRouter& t_router);

void Deserialize(const transport_catalogue_serialize::Catalogue &Catalogue, catalogue::TransportCatalogue& t_catalogue);
void Deserialize(const transport_catalogue_serialize::Catalogue &Catalogue, renderer::MapRenderer& t_renderer);
void Deserialize(const transport_catalogue_serialize::Catalogue &Catalogue, router::TransportRouter& t_router);

}   // namespace serialize
