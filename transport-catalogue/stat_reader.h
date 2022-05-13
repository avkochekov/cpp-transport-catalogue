// Чтение запросов на вывод и сам вывод
#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <queue>
#include "transport_catalogue.h"

namespace catalogue{
    namespace reader {
        class StatReader {
        public:
            StatReader(TransportCatalogue& catalogue, std::istream &istream = std::cin, std::ostream &ostream = std::cout)
                : catalogue{catalogue}, istream{istream}, ostream{ostream} {};

            void ReadQueues() const;

            void GetBus(const std::string_view queue) const;
            void GetStop(const std::string_view queue) const;

        private:
            std::string_view Simplified(const std::string_view) const;

            TransportCatalogue &catalogue;
            std::istream& istream;
            std::ostream& ostream;
        };
    }
}

