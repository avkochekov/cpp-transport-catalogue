#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>
#include <sstream>

using namespace std;

int main()
{
    catalogue::TransportCatalogue transport_catalogue;
    catalogue::reader::InputReader input_reader(transport_catalogue);
    catalogue::reader::StatReader stat_reader(transport_catalogue);

    input_reader.ReadQueues();
    stat_reader.ReadQueues();

    return 0;
}
