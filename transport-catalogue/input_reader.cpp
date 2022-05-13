#include "input_reader.h"

#include <cassert>
#include <execution>
#include <algorithm>

using namespace::catalogue;
using namespace::catalogue::reader;

static const std::string BusQueryIdentifier = "Bus";
static const std::string StopQueryIdentifier = "Stop";
static const std::string DistanceSeparator = " to ";
static constexpr char QuerySeparator = ':';
static constexpr char ItemsSeparator = ',';

static const std::unordered_map<RouteType, char> route_type_separator{
    {RouteType::Circle, '-'},
    {RouteType::Linear, '>'},
};

void InputReader::ReadQueues()
{
    std::string queue;
    std::getline(istream, queue);
    size_t queue_count = std::stod(queue);

    std::vector<std::string> stop_queues;
    std::vector<std::string> bus_queues;
    bus_queues.reserve(queue_count);
    stop_queues.reserve(queue_count);

    while (queue_count --> 0){
        std::string queue;
        std::getline(istream, queue);
        if (queue.empty()){
            ++queue_count;
            continue;
        }

        if (auto iter = queue.find(BusQueryIdentifier); iter != std::string::npos){
            bus_queues.push_back(queue.substr(iter + BusQueryIdentifier.size()));
        } else if (auto iter = queue.find(StopQueryIdentifier); iter != std::string::npos){
            stop_queues.push_back(queue.substr(iter + StopQueryIdentifier.size()));
        }
    }

    ReadStops(stop_queues);

    ReadBuses(bus_queues);
}

void InputReader::ReadStops(const std::vector<std::string>& queues)
{
    std::list<std::tuple<std::string_view, std::string_view, double>> distances;

    for (const auto &queue : queues){
        auto tmp = Split(queue, QuerySeparator);
        auto stop_name = Simplified(tmp.front());

        auto items = Split(tmp.back(), ItemsSeparator);
        double lat = std::stod(std::string(items.front()));
        items.pop_front();
        double lng = std::stod(std::string(items.front()));
        items.pop_front();

        catalogue.AddStop(stop_name, {lat, lng});

        for (auto item : items){
            auto data = Split(item, DistanceSeparator);
            auto dist_string = Simplified(data.front());
            auto dist = std::stod(std::string(dist_string.substr(0, dist_string.size() - 1)));
            auto stop = Simplified(data.back());

            distances.push_back({stop_name, stop, dist});
        }
    }

    for (auto dist : distances){
        catalogue.AddDistance(std::get<0>(dist), std::get<1>(dist), std::get<2>(dist));
    }
}

void InputReader::ReadBuses(const std::vector<std::string>& queues)
{
    for (const auto &queue : queues){
        auto tmp = Split(queue, QuerySeparator);
        auto bus_name = Simplified(tmp.front());
        tmp.pop_front();

        auto type = tmp.back().find(route_type_separator.at(RouteType::Linear)) != std::string::npos ?
                    RouteType::Linear :
                    RouteType::Circle;

        auto stops = Split(tmp.back(), route_type_separator.at(type));
        std::transform(stops.begin(), stops.end(),
                       stops.begin(),
                       [&](const std::string_view stop){ return Simplified(stop); });

        catalogue.AddBus(bus_name, type, stops);
    }
}

std::vector<std::string> InputReader::ReadStops(std::string_view queue, const char separator)
{
    std::vector<std::string> stops;
    size_t pos = -1;
    do {
        queue = queue.substr(pos + 1);
        pos = queue.find(separator);
        stops.push_back(std::string(Simplified(queue.substr(0, pos))));
    } while (pos != std::string::npos);
    return stops;
}

std::string_view InputReader::Simplified(const std::string_view text)
{
    assert(!text.empty());

    auto left = text.find_first_not_of(' ');
    auto right = text.find_last_not_of(' ');

    assert(left != std::string::npos);

    return text.substr(left, right - left + 1);
}

std::deque<std::string_view> InputReader::Split(const std::string_view text, const char separator)
{
    std::deque<std::string_view> res;
    auto last = 0;
    size_t pos = 0;
    do {
        pos = text.find(separator, last);
        res.push_back(text.substr(last, pos - last));
        last = pos + 1;
    } while (pos != std::string_view::npos);
    return res;
}

std::deque<std::string_view> InputReader::Split(const std::string_view text, const std::string_view separator)
{
    std::deque<std::string_view> res;
    auto last = 0;
    size_t pos = 0;
    do {
        pos = text.find(separator, last);
        res.push_back(text.substr(last, pos));
        last = pos + separator.size();
    } while (pos != std::string_view::npos);
    return res;
}
