#include "request_handler.h"

#include "serialization.h"

#include <fstream>
#include <sstream>
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
RequestHandler::RequestHandler(catalogue::TransportCatalogue &db, renderer::MapRenderer &renderer, router::TransportRouter &router)
    : catalogue_{db}
    , renderer_{renderer}
    , router_{router} {
}

void RequestHandler::AddStop(const std::string &name, const double lat, const double lon)
{
    catalogue_.AddStop(name, {lat, lon});
}

void RequestHandler::AddDistanceBetweenStops(const std::string &from, const std::string &to, const double distance)
{
    catalogue_.AddDistance(from, to, distance);
}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view &bus_name) const
{
    return catalogue_.GetBusInfo(bus_name);
}

const std::optional<StopStat> RequestHandler::GetStopStat(const std::string_view &stop_name) const
{
    return catalogue_.GetStopInfo(stop_name);
}

void RequestHandler::SetRendererSettings(const renderer::MapRenderSettings &settings)
{
    renderer_.SetSettings(settings);
}

void RequestHandler::RenderMap(std::ostream &stream) const
{
    renderer_.RenderCatalogue(catalogue_, stream);
}

void RequestHandler::SetRouterSettings(const int bus_wait_time, const int bus_velocity)
{
    router_.SetBusWaitTime(bus_wait_time).SetBusVelocity(bus_velocity);
    router_.RouteCatalogue(catalogue_);
}

std::optional<RouteInfo> RequestHandler::MakeRoute(const std::string &from_stop, const std::string &to_stop) const
{
    return router_.MakeRoute(from_stop, to_stop);
}

void RequestHandler::Serialize(const std::string& path)
{
    std::ofstream stream(path, std::ofstream::out | std::ofstream::trunc | std::ostream::binary);

    transport_catalogue_serialize::Catalogue data;

    serialize::Serialize(data, catalogue_);
    serialize::Serialize(data, renderer_);
    serialize::Serialize(data, router_);

    data.SerializeToOstream(&stream);

    stream.close();
}

void RequestHandler::Deserialize(const std::string& path)
{
    std::ifstream stream(path, std::ifstream::in | std::ostream::binary);

    transport_catalogue_serialize::Catalogue data;
    data.ParseFromIstream(&stream);

    serialize::Deserialize(data, catalogue_);
    serialize::Deserialize(data, renderer_);
    serialize::Deserialize(data, router_);

    stream.close();
}
