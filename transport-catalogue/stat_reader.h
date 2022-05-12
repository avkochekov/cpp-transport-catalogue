// Чтение запросов на вывод и сам вывод
#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <queue>
#include "transport_catalogue.h"

namespace catalogue{
    namespace reader {
        class StatRaeader {
        public:
            StatRaeader(TransportCatalogue& catalogue, std::istream& stream = std::cin) : catalogue{catalogue}, stream{stream} {};

            void ReadQueues(size_t count, std::istream& = std::cin) const;

            void GetBus(const std::string_view queue) const;
            void GetStop(const std::string_view queue) const;

        private:
            std::string_view Simplified(const std::string_view) const;

            TransportCatalogue &catalogue;
            std::istream& stream;
        };
    }
}

