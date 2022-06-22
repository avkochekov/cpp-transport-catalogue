#include "transport_catalogue.h"
#include <algorithm>
#include <iterator>
#include <execution>
#include <cassert>
#include <optional>
#include <unordered_set>

static constexpr unsigned int maxRouteDistance = 1'000'000;

using namespace::catalogue;

void TransportCatalogue::AddStop(const std::string_view name, const geo::Coordinates &coord)
{
    assert(!name.empty());
    stops.push_back({std::string(name), coord});
    stopname_to_stop[stops.back().name] = &stops.back();
    stop_to_buses[stops.back().name];
}

void TransportCatalogue::AddDistance(const std::string_view from, const std::string_view to, const double l)
{
    assert(stopname_to_stop.count(from));
    assert(stopname_to_stop.count(to));

    if (l < 0 || l > maxRouteDistance){
        throw std::invalid_argument("invalid distance value");
    } else {
        stops_to_distance[{stopname_to_stop.at(from), stopname_to_stop.at(to)}] = l;
    }
}

std::optional<TransportCatalogue::Stop> TransportCatalogue::FindStop(const std::string_view name) const
{
    if (stopname_to_stop.count(name)) {
        return *stopname_to_stop.at(name);
    }
    return std::nullopt;
}

std::optional<TransportCatalogue::Bus> TransportCatalogue::FindBus(const std::string_view name) const
{
    if (busname_to_bus.count(name)) {
        return *busname_to_bus.at(name);
    }
    return std::nullopt;
}

std::optional<StopStat> TransportCatalogue::GetStopInfo(const std::string_view name) const
{
    const auto &stop = FindStop(name);
    if (stop == std::nullopt){
        return std::nullopt;
    } else {
        auto& buses = stop_to_buses.at(std::string(name));
        std::vector<std::string> res(buses.size());
        std::transform(std::execution::par,
                       buses.begin(), buses.end(),
                       res.begin(),
                       [&](const std::string_view var)
        {
            return std::string(var);
        });

        std::sort(res.begin(), res.end());
        res.erase(std::unique(res.begin(), res.end()), res.end());
        return StopStat{std::string(name), res, stopname_to_stop.at(name)->coordinates};
    }
}

std::optional<BusStat> TransportCatalogue::GetBusInfo(const std::string_view name) const
{
    const auto &bus = FindBus(name);
    if (bus == std::nullopt){
        return std::nullopt;
    } else {
        double route_length = 0;
        double geo_length = 0;
        auto l_iter = bus->stops.begin();
        auto r_iter = std::next(l_iter);
        while (r_iter != bus->stops.end()) {
            geo_length += geo::ComputeDistance({(*l_iter)->coordinates.lat, (*l_iter)->coordinates.lng},
                                               {(*r_iter)->coordinates.lat, (*r_iter)->coordinates.lng});
            if (stops_to_distance.count({(*l_iter), (*r_iter)})) {
                route_length += stops_to_distance.at({(*l_iter), (*r_iter)});
            } else if (stops_to_distance.count({(*r_iter), (*l_iter)})) {
                route_length += stops_to_distance.at({(*r_iter), (*l_iter)});
            } else {
                route_length += geo_length;
            }

            l_iter = r_iter;
            r_iter = std::next(l_iter);
        }

        size_t stops_count = bus->stops.size();
        std::unordered_set<Stop*> stops (bus->stops.begin(), bus->stops.end());
        size_t uniq_stops_count = std::unordered_set<Stop*>(bus->stops.begin(), bus->stops.end()).size();



        if (bus->type == Linear){
            stops_count = stops_count * 2 - 1;
            geo_length *= 2;

            auto l_iter = bus->stops.rbegin();
            auto r_iter = std::next(l_iter);
            while (r_iter != bus->stops.rend()) {
                if (stops_to_distance.count({(*l_iter), (*r_iter)})) {
                    route_length += stops_to_distance.at({(*l_iter), (*r_iter)});
                } else if (stops_to_distance.count({(*r_iter), (*l_iter)})) {
                    route_length += stops_to_distance.at({(*r_iter), (*l_iter)});
                } else {
                    route_length += geo_length;
                }

                l_iter = r_iter;
                r_iter = std::next(l_iter);
            }
        }

        const auto &bus_stops_ptr = busname_to_bus.at(name)->stops;
        std::vector<std::string> bus_stops(bus_stops_ptr.size());
        std::transform(bus_stops_ptr.begin(), bus_stops_ptr.end(),
                       bus_stops.begin(),
                       [](const Stop* value){ return value->name; });

        return BusStat{std::string(name), stops_count, uniq_stops_count, route_length, route_length / geo_length, bus_stops};
    }
}

RouteType TransportCatalogue::GetBusType(const std::string_view name) const
{
    return busname_to_bus.at(name)->type;
}

double TransportCatalogue::GetDistanceBetweenStops(const std::string_view from_stop, const std::string_view to_stop) const
{
    const auto from_stop_ptr = stopname_to_stop.at(from_stop);
    const auto to_stop_ptr = stopname_to_stop.at(to_stop);
    if (stops_to_distance.count({from_stop_ptr, to_stop_ptr}))
        return stops_to_distance.at({from_stop_ptr, to_stop_ptr});
    else if (stops_to_distance.count({to_stop_ptr, from_stop_ptr}))
        return stops_to_distance.at({to_stop_ptr, from_stop_ptr});
    else
        return geo::ComputeDistance(from_stop_ptr->coordinates, to_stop_ptr->coordinates);
}

std::vector<std::string> TransportCatalogue::GetStops() const
{
    std::vector<std::string> res(stops.size());
    std::transform(stops.begin(), stops.end(),
                   res.begin(),
                   [](const Stop& val){ return std::string(val.name); });
    return res;
}

std::optional<Coordinates> TransportCatalogue::GetStopCoordinates(const std::string_view name) const
{
    if (stopname_to_stop.count(name))
        return stopname_to_stop.at(name)->coordinates;
    return std::nullopt;
}

std::vector<std::string> TransportCatalogue::GetBuses() const
{
    std::vector<std::string> res(buses.size());
    std::transform(buses.begin(), buses.end(),
                   res.begin(),
                   [](const Bus& val){ return val.name; });
    return res;
}

std::optional<std::vector<std::string>> TransportCatalogue::GetBusStops(const std::string_view name) const
{
    if (busname_to_bus.count(name)) {
        const auto &stops = busname_to_bus.at(name)->stops;
        std::vector<std::string> res(stops.size());
        std::transform(stops.begin(), stops.end(),
                       res.begin(),
                       [](const Stop* val){ return val->name; });
        return res;
    }
    return std::nullopt;
}

bool TransportCatalogue::Stop::operator==(const std::string &name) const {
    return this->name == name;
}

bool TransportCatalogue::Stop::operator==(const Stop& other) const {
    return this->name == other.name;
}

bool TransportCatalogue::Bus::operator==(const std::string &name) const {
    return this->name == name;
}

bool TransportCatalogue::Bus::operator==(const Bus& other) const {
    return this->name == other.name;
}
