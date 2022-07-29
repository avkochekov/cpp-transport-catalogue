#pragma once

#include <stdlib.h>
#include <string_view>

#include "transport_catalogue.h"

namespace serialize{
void Serialize(const std::string& path, const catalogue::TransportCatalogue& t_catalogue);
void Deserialize(const std::string& path, catalogue::TransportCatalogue& t_catalogue);
}   // namespace serialize
