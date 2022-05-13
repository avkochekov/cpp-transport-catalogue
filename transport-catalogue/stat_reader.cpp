#include "stat_reader.h"
#include <cassert>
#include <iomanip>

using namespace::catalogue::reader;

static const std::string BusQueryIdentifier = "Bus";
static const std::string StopQueryIdentifier = "Stop";
static const std::string DistanceSeparator = " to ";

void StatReader::ReadQueues() const
{
    std::string queue;
    std::getline(istream, queue);
    size_t queue_count = std::stod(queue);

    while (queue_count --> 0){
        std::getline(istream, queue);
        if (queue.empty()){
            ++queue_count;
            continue;
        }

        if (auto iter = queue.find(BusQueryIdentifier); iter != std::string::npos){
            GetBus(queue.substr(iter + BusQueryIdentifier.size()));
        } else if (auto iter = queue.find(StopQueryIdentifier); iter != std::string::npos){
            GetStop(queue.substr(iter + StopQueryIdentifier.size()));
        }

    }
}

void StatReader::GetBus(const std::string_view queue) const
{
    const std::string_view name = Simplified(queue);
    const auto &info = this->catalogue.GetBusInfo(name);
    ostream << "Bus " << name << ": ";
    if (info == std::nullopt){
        ostream << "not found";
    } else {
        ostream << info->stops_on_route << " stops on route, ";
        ostream << info->unique_stops << " unique stops, ";
        ostream << std::setprecision(6) << info->route_length << " route length, ";
        ostream << std::setprecision(6) << info->curvature << " curvature";
    }
    ostream << std::endl;
}

void StatReader::GetStop(const std::string_view queue) const
{
    const std::string_view name = Simplified(queue);
    const auto &info = this->catalogue.GetStopInfo(name);
    ostream << "Stop " << name << ": ";
    if (info == std::nullopt){
        ostream << "not found";
    } else{
        const auto &buses = info->buses;
        if (buses.empty()){
            ostream << "no buses";
        } else {
            ostream << "buses";
            for (const auto& bus : buses){
                ostream << ' ' << bus;
            }
        }
    }
    ostream << std::endl;
}

std::string_view StatReader::Simplified(const std::string_view text) const
{
    assert(!text.empty());

    auto left = text.find_first_not_of(' ');
    auto right = text.find_last_not_of(' ');

    assert(left != std::string::npos);

    return text.substr(left, right - left + 1);
}
