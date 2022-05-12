#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>
#include <sstream>

using namespace std;

int main()
{
    catalogue::TransportCatalogue transport_catalogue;
    catalogue::reader::InputRaeader input_reader(transport_catalogue);
    catalogue::reader::StatRaeader stat_reader(transport_catalogue);

    size_t query_count;
    std::cin >> query_count;
    input_reader.ReadQueues(query_count);

    std::cin >> query_count;
    stat_reader.ReadQueues(query_count);
    return 0;
}
