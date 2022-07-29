#include <fstream>
#include <vector>

#include "serialization.h"

#include <transport_catalogue.pb.h>

void serialize::Serialize(const std::string &path, const catalogue::TransportCatalogue &t_catalogue)
{
    std::ofstream outstream(path);
    if (!outstream)
        return;

    const auto& t_stops = t_catalogue.GetStops();

    transport_catalogue_serialize::Catalogue catalogue;
    for (const std::string& t_stop : t_stops){
        auto stop = catalogue.add_stops();
        stop->set_name(t_stop);

        // Координаты остановки
        const auto& t_stop_coordinates = t_catalogue.GetStopCoordinates(t_stop);
        if (t_stop_coordinates){
            stop->mutable_coordinates()->set_latitude(t_stop_coordinates->lat);
            stop->mutable_coordinates()->set_longitude(t_stop_coordinates->lng);

        }

        // Дистанция между остановками
        for (const std::string& t_stop_to : t_stops){
            auto t_distance = t_catalogue.GetDistanceBetweenStops(t_stop, t_stop_to);
            if (t_distance == 0)
                continue;

            auto distance = stop->add_distance();
            distance->set_length(t_distance);
            distance->set_stop(std::distance(t_stops.begin(), std::find(t_stops.begin(), t_stops.end(), t_stop_to)));
        }
    }

    const auto& t_buses = t_catalogue.GetBuses();
    for (const std::string& t_bus : t_buses){
        auto bus = catalogue.add_buses();
        bus->set_name(t_bus);
        bus->set_is_roundtrip(t_catalogue.GetBusType(t_bus) == RouteType::Roundtrip);
        auto t_bus_stops = t_catalogue.GetBusStops(t_bus);
        if (t_bus_stops){
            for(const std::string& t_bus_stop : *t_bus_stops){
                bus->add_stops(std::distance(t_stops.begin(), std::find(t_stops.begin(), t_stops.end(), t_bus_stop)));
            }
        }
    }

    catalogue.SerializePartialToOstream(&outstream);
}

void serialize::Deserialize(const std::string &path, catalogue::TransportCatalogue &t_catalogue)
{
    std::ifstream instream(path);
    if (!instream)
        return;

    transport_catalogue_serialize::Catalogue catalogue;
    catalogue.ParseFromIstream(&instream);

    size_t buses_size = catalogue.buses_size();
    size_t stops_size = catalogue.stops_size();

    std::vector<std::string_view> buses(buses_size);
    std::vector<std::string_view> stops(stops_size);

    for (size_t i = 0; i < stops_size; ++i){
        const auto& stop = catalogue.stops(i);
        stops[i] = stop.name();
        t_catalogue.AddStop(std::string(stop.name()),
                            {stop.coordinates().latitude(),
                             stop.coordinates().longitude()});
    }

    for (size_t i = 0; i < stops_size; ++i){
        const auto& stop = catalogue.stops(i);
        for (size_t d = 0; d < stop.distance_size(); ++d){
            const auto& distance = stop.distance(d);
            if (!distance.IsInitialized())
                continue;
            t_catalogue.AddDistance(stop.name(),
                                    std::string(stops[distance.stop()]),
                                    distance.length());
        }
    }

    for (size_t i = 0; i < buses_size; ++i){
        const auto& bus = catalogue.buses(i);
        buses[i] = bus.name();

        size_t bus_stops_size = bus.stops_size();
        std::vector<std::string> bus_stops(bus_stops_size);
        for (size_t i = 0; i < bus_stops_size; ++i){
            bus_stops[i] = std::string(stops[bus.stops(i)]);
        }

        t_catalogue.AddBus(bus.name(),
                           bus.is_roundtrip() ? RouteType::Roundtrip : RouteType::Linear,
                           bus_stops);
    }
}

