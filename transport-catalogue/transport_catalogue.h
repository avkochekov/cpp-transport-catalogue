// Класс транспортного справочника;
#pragma once

#include <cassert>
#include <execution>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <deque>
#include <tuple>
#include <optional>

#include "geo.h"

namespace catalogue {

    enum RouteType{
        Linear,
        Circle,
    };

    namespace info {
        struct Stop{
            std::string name;
            std::vector<std::string> buses;
        };

        struct Bus{
            std::string name;
            size_t stops_on_route;
            size_t unique_stops;
            double route_length;
            double curvature;
        };
    }

    class TransportCatalogue{
        struct Stop{
            std::string name;
            Coordinates coordinates;

            bool operator==(const std::string& name) const;
            bool operator==(const Stop&) const;
        };

        struct Bus{
            std::string name;
            RouteType type;
            std::vector<Stop *> stops;

            bool operator==(const std::string& name) const;
            bool operator==(const Bus&) const;
        };

        struct StopToStopHasher{
            size_t operator()(const std::pair<Stop*, Stop*> stops) const {
                return std::hash<const void*>{}(stops.first) + std::hash<const void*>{}(stops.second);
            }
        };

    public:
        ///[\brief] Добавление новой остановки
        void AddStop(const std::string_view name, const Coordinates);
        ///[\brief] Добавляет новоый маршрут
        template<typename Container>
        void AddBus(const std::string_view name, const RouteType type, const Container stops);
        ///[\brief] Добавление расстояния между остановками
        void AddDistance(const std::string_view from, const std::string_view to, const double distance);
        std::optional<Stop> FindStop(const std::string_view) const;
        std::optional<Bus> FindBus(const std::string_view) const;
        std::optional<info::Stop> AboutStop(const std::string_view) const;
        std::optional<info::Bus> AboutBus(const std::string_view) const;

    private:
        std::deque<Stop> stops;
        std::deque<Bus> buses;

        std::unordered_map<std::string_view, Stop*> stopname_to_stop;
        std::unordered_map<std::string_view, Bus*> busname_to_bus;

        std::unordered_map<std::string_view, std::deque<std::string_view>> stop_to_buses;

        std::unordered_map<std::pair<Stop*, Stop*>, double, StopToStopHasher> stops_to_distance;
    };

    //======================================================================
    template<typename Container>
    inline void TransportCatalogue::AddBus(const std::string_view name, const RouteType type, const Container stops_)
    {
        assert(!name.empty());
        assert(stops_.size() > 1);

        buses.push_back({std::string(name), type, std::vector<Stop*>()});
        busname_to_bus[buses.back().name] = &buses.back();
        auto stops = &buses.back().stops;
        stops->resize(stops_.size());
        std::transform(std::execution::par,
                       stops_.begin(), stops_.end(),
                       stops->begin(),
                       [&](const auto stop_name)
        {
            return stopname_to_stop.at(stop_name);
        });
        for(const auto& stop : stops_){
            stop_to_buses[stop].push_back(buses.back().name);
        }
    }

}
