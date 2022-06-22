#include "transport_router.h"
#include "router.h"

#include <algorithm>

namespace router {

static constexpr double SPEED_TRANSFORM_KOEFFICIENT = 1000.0 / 60.0;

TransportRouter &TransportRouter::SetBusWaitTime(int value)
{
    bus_wait_time_ = value;
    return *this;
}

TransportRouter &TransportRouter::SetBusVelocity(int value)
{
    bus_velocity_ = value;
    return *this;
}

void TransportRouter::RouteCatalog(catalogue::TransportCatalogue &catalogue)
{
    using namespace graph;

    stops_ = catalogue.GetStops();
    buses_ = catalogue.GetBuses();

    std::sort(stops_.begin(), stops_.end());
    std::sort(buses_.begin(), buses_.end());

    graph_ = DirectedWeightedGraph<double>(stops_.size());

    for (const std::string_view bus : buses_){
        const auto& bus_info = catalogue.FindBus(bus);
        if (!bus_info)
            continue;
        if (bus_info->stops.empty())
            continue;

        const auto& bus_stops = bus_info->stops;

        std::vector<size_t> bus_stops_indexes(bus_stops.size());
        std::transform(bus_stops.begin(), bus_stops.end(),
                       bus_stops_indexes.begin(),
                       [this](const auto stop){
            return std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), stop->name));
        });

        for (size_t left_index = 0; left_index < bus_stops.size(); ++left_index){
            size_t span_count = 1;
            double forward_time = 0;
            double backward_time = 0;
            std::string_view prev_stop = bus_stops.at(left_index)->name;
            for (size_t right_index = left_index + 1; right_index < bus_stops.size(); ++right_index){
                auto left_stop_index = bus_stops_indexes[left_index];
                auto right_stop_index = bus_stops_indexes[right_index];

                const double koeff = 1 / (bus_velocity_ * SPEED_TRANSFORM_KOEFFICIENT);

                forward_time += catalogue.GetDistanceBetweenStops(prev_stop, bus_stops.at(right_index)->name) * koeff;
                backward_time += catalogue.GetDistanceBetweenStops(bus_stops.at(right_index)->name, prev_stop) * koeff;

                auto forward_route = graph_.AddEdge({left_stop_index,
                                                     right_stop_index,
                                                     forward_time + bus_wait_time_});
                route_info_[forward_route] = {
                    stops_.at(left_stop_index),
                    stops_.at(right_stop_index),
                    bus,
                    span_count,
                    forward_time};

                if (bus_info->type == Linear){
                    auto backward_route = graph_.AddEdge({right_stop_index,
                                                          left_stop_index,
                                                          backward_time + bus_wait_time_});
                    route_info_[backward_route] = {
                        stops_.at(right_stop_index),
                        stops_.at(left_stop_index),
                        bus,
                        span_count,
                        backward_time};
                }

                prev_stop = bus_stops.at(right_index)->name;
                ++span_count;
            }
        }
    }
}

std::optional<RouteInfo> TransportRouter::MakeRoute(const std::string &from_stop, const std::string &to_stop)
{
    if (!graph_router_){
        graph_router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    auto from_index = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), from_stop));
    auto to_index = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), to_stop));

    auto route = graph_router_->BuildRoute(from_index, to_index);

    if (route == std::nullopt){
        return std::nullopt;
    }
    router::RouteInfo route_info = router::RouteInfo();
    route_info.total_time = route->weight;
    for (const auto edge : route->edges){
        const RouteParams& edge_info = route_info_.at(edge);

        route_info.buses.push_back(
                    router::RouteInfo::BusInfo{edge_info.bus,
                                               edge_info.span_count,
                                               edge_info.time});

        route_info.stops.push_back(
                    router::RouteInfo::StopInfo{edge_info.from_stop,
                                                bus_wait_time_});

    }
    return route_info;
}

} // namespace router
