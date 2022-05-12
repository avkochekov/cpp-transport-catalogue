#include "stat_reader.h"
#include <cassert>
#include <iomanip>

using namespace::catalogue::reader;

static const std::string BusQueryIdentifier = "Bus";
static const std::string StopQueryIdentifier = "Stop";
static const std::string DistanceSeparator = " to ";

void StatRaeader::ReadQueues(size_t queue_count, std::istream &stream) const
{
    while (queue_count --> 0){
        std::string queue;
        std::getline(stream, queue);
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

void StatRaeader::GetBus(const std::string_view queue) const
{
    const std::string_view name = Simplified(queue);
    const auto &info = this->catalogue.AboutBus(name);
    std::cout << "Bus " << name << ": ";
    if (info == std::nullopt){
        std::cout << "not found";
    } else {
        std::cout << info->stops_on_route << " stops on route, ";
        std::cout << info->unique_stops << " unique stops, ";
        std::cout << std::setprecision(6) << info->route_length << " route length, ";
        std::cout << std::setprecision(6) << info->curvature << " curvature";
    }
    std::cout << std::endl;
}

void StatRaeader::GetStop(const std::string_view queue) const
{
    const std::string_view name = Simplified(queue);
    const auto &info = this->catalogue.AboutStop(name);
    std::cout << "Stop " << name << ": ";
    if (info == std::nullopt){
        std::cout << "not found";
    } else{
        const auto &buses = info->buses;
        if (buses.empty()){
            std::cout << "no buses";
        } else {
            std::cout << "buses";
            for (const auto& bus : buses){
                std::cout << ' ' << bus;
            }
        }
    }
    std::cout << std::endl;
}

std::string_view StatRaeader::Simplified(const std::string_view text) const
{
    assert(!text.empty());

    auto left = text.find_first_not_of(' ');
    auto right = text.find_last_not_of(' ');

    assert(left != std::string::npos);

    return text.substr(left, right - left + 1);
}
