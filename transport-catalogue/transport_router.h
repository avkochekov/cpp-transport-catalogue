#pragma once

#include "transport_catalogue.h"

#include "graph.h"
#include "router.h"

#include <deque>
#include <map>

namespace router {
struct RouteInfo{
    struct StopInfo{
        std::string_view name;
        double time;
    };
    struct BusInfo{
        std::string_view name;
        size_t span_count;
        double time;
    };

    std::deque<StopInfo> stops;
    std::deque<BusInfo> buses;
    double total_time;
};

class TransportRouter{
    struct RouteParams {
        std::string_view from_stop;
        std::string_view to_stop;
        std::string_view bus;
        size_t span_count;
        double time;
    };

public:
    TransportRouter& SetBusWaitTime(double);
    TransportRouter& SetBusVelocity(double);
    void RouteCatalogue(catalogue::TransportCatalogue& catalogue);
    std::optional<RouteInfo> MakeRoute(const std::string& from_stop, const std::string& to_stop);

    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    graph::DirectedWeightedGraph<double> &GetGraph();

    const RouteParams &GetRouteParams(const graph::EdgeId) const;
    const size_t GetRouteParamsCount() const;
    void SetRouteParams(const graph::EdgeId edge_id, const RouteParams& params);

    void SetStops(const std::vector<std::string>& stops);
    void SetBuses(const std::vector<std::string>& buses);

    const std::vector<std::string>& GetStops() const;
    const std::vector<std::string>& GetBuses() const;

    double GetBusWaitTime() const;
    double GetBusVelocity() const;


private:
    struct {
        double wait_time = 0;
        double velocity = 0;
    } bus_params_;

    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> graph_router_;

    std::vector<std::string> stops_;
    std::vector<std::string> buses_;

    std::map<graph::EdgeId, RouteParams> route_info_;
};
} // namespace router
