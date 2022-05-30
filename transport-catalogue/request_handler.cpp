#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
RequestHandler::RequestHandler(catalogue::TransportCatalogue &db, renderer::MapRenderer &renderer)
    : catalogue_{db}
    , renderer_{renderer} {
}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view &bus_name) const
{
    (void)bus_name;
    return std::nullopt;
}

const std::unordered_set<std::string> RequestHandler::GetBusesByStop(const std::string_view &stop_name) const
{
    const auto &buses = catalogue_.GetStopInfo(stop_name)->buses;
    return std::unordered_set<std::string>(buses.begin(), buses.end());
}

void RequestHandler::RenderMap(std::ostream &stream) const
{
    renderer_.Render(stream);
}
