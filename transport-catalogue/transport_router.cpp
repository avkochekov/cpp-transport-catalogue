#include "transport_router.h"
#include "router.h"

#include <algorithm>

namespace router {

static constexpr double SPEED_TRANSFORM_KOEFFICIENT = 1000.0 / 60.0;

TransportRouter &TransportRouter::SetBusWaitTime(double value)
{
    bus_params_.wait_time = value;
    return *this;
}

TransportRouter &TransportRouter::SetBusVelocity(double value)
{
    bus_params_.velocity = value;
    return *this;
}

void TransportRouter::RouteCatalogue(catalogue::TransportCatalogue &catalogue)
{
    using namespace graph;

    SetStops(catalogue.GetStops());
    SetBuses(catalogue.GetBuses());

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

                const double koeff = 1 / (bus_params_.velocity * SPEED_TRANSFORM_KOEFFICIENT);

                forward_time += catalogue.GetDistanceBetweenStops(prev_stop, bus_stops.at(right_index)->name) * koeff;
                backward_time += catalogue.GetDistanceBetweenStops(bus_stops.at(right_index)->name, prev_stop) * koeff;

                auto forward_route = graph_.AddEdge({left_stop_index,
                                                     right_stop_index,
                                                     forward_time + bus_params_.wait_time});
                route_info_[forward_route] = {
                    stops_.at(left_stop_index),
                    stops_.at(right_stop_index),
                    bus,
                    span_count,
                    forward_time};

                if (bus_info->type == Linear){
                    auto backward_route = graph_.AddEdge({right_stop_index,
                                                          left_stop_index,
                                                          backward_time + bus_params_.wait_time});
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
                                                bus_params_.wait_time});

    }
    return route_info;
}

const graph::DirectedWeightedGraph<double> &TransportRouter::GetGraph() const
{
    return graph_;
}

graph::DirectedWeightedGraph<double> &TransportRouter::GetGraph()
{
    return graph_;
}

const TransportRouter::RouteParams& TransportRouter::GetRouteParams(const graph::EdgeId edge) const
{
    return route_info_.at(edge);
}

void TransportRouter::SetRouteParams(const graph::EdgeId edge_id, const RouteParams &params)
{
    route_info_[edge_id] = std::move(params);
}

void TransportRouter::SetStops(const std::vector<std::string> &stops)
{
    stops_ = std::move(stops);
    std::sort(stops_.begin(), stops_.end());
}

void TransportRouter::SetBuses(const std::vector<std::string> &buses)
{
    buses_ = std::move(buses);
    std::sort(buses_.begin(), buses_.end());
}

const std::vector<std::string> &TransportRouter::GetStops() const
{
    return stops_;
}

const std::vector<std::string> &TransportRouter::GetBuses() const
{
    return buses_;
}

double TransportRouter::GetBusWaitTime() const
{
    return bus_params_.wait_time;
}

double TransportRouter::GetBusVelocity() const
{
    return bus_params_.velocity;
}

} // namespace router
