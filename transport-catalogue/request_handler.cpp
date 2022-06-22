#include "request_handler.h"

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
    using namespace svg;
    using svg::Circle;

    std::unordered_set<std::string> unique_stops;
    std::vector<std::string> buses = catalogue_.GetBuses();
    std::unordered_map<std::string, std::vector<std::string>> buses_to_stops;
    for (const auto &bus : buses){
        auto bus_stops = *catalogue_.GetBusStops(bus);
        unique_stops.merge(std::unordered_set<std::string>(bus_stops.begin(), bus_stops.end()));
        buses_to_stops[bus] = *catalogue_.GetBusStops(bus);
    }
    std::sort(buses.begin(), buses.end());
    std::vector<std::string> stops(unique_stops.begin(), unique_stops.end());
    std::sort(stops.begin(), stops.end());

    std::deque<Coordinates> stops_coordinates;
    std::unordered_map<std::string_view, Coordinates*> stops_to_coordinates;
    std::for_each(unique_stops.begin(), unique_stops.end(),
                  [this, &stops_coordinates, &stops_to_coordinates](const std::string_view stop)
    {
        stops_coordinates.push_back(*catalogue_.GetStopCoordinates(stop));
        stops_to_coordinates[stop] = &stops_coordinates.back();
    });

    const auto &render_settinds = renderer_.GetSettings();
    SphereProjector projector(stops_coordinates.begin(),
                              stops_coordinates.end(),
                              render_settinds.width,
                              render_settinds.height,
                              render_settinds.padding);


    Document svg;
    std::unordered_map<std::string, Point> stops_to_points;
    for (const auto &stop : unique_stops){
        stops_to_points[stop] = projector(*stops_to_coordinates.at(stop));
    }

    std::deque<Polyline> lines;
    std::deque<Text> buses_names;

    for (const auto &bus : buses){
        if (buses_to_stops.at(bus).empty())
            continue;

        auto stops = std::move(buses_to_stops.at(bus));

        std::vector<Point> stops_points(stops.size());
        std::transform(std::execution::par,
                       stops.begin(), stops.end(),
                       stops_points.begin(),
                       [&stops_to_points](auto &stop_name){ return stops_to_points[stop_name]; });

        renderer_.AddBusLine(bus, stops_points, catalogue_.GetBusType(bus) == Linear);
    }

    for (const auto &stop : stops){
        renderer_.AddStopPoint(stop, stops_to_points.at(stop));
    }
    renderer_.Render(stream);
}

void RequestHandler::SetRouterSettings(const int bus_wait_time, const int bus_velocity)
{
    router_.SetBusWaitTime(bus_wait_time).SetBusVelocity(bus_velocity);
    router_.RouteCatalog(catalogue_);
}

std::optional<RouteInfo> RequestHandler::MakeRoute(const std::string &from_stop, const std::string &to_stop) const
{
    return router_.MakeRoute(from_stop, to_stop);
}
