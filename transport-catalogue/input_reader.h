// Чтение запросов на заполнение базы
#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <list>
#include "transport_catalogue.h"

namespace catalogue{
    namespace reader {
        class InputReader {
        public:
            InputReader(TransportCatalogue &catalogue, std::istream &istream = std::cin, std::ostream &ostream = std::cout)
                : catalogue{catalogue}, istream{istream}, ostream{ostream} {};
            void ReadQueues();

        private:
            void ReadStops(const std::vector<std::string>& queue);
            void ReadBuses(const std::vector<std::string>& queue);

            std::vector<std::string> ReadStops(const std::string_view queue, const char separator);

            std::string_view Simplified(const std::string_view);
            std::deque<std::string_view> Split(const std::string_view text, const char separator);
            std::deque<std::string_view> Split(const std::string_view text, const std::string_view separator);

            TransportCatalogue &catalogue;
            std::istream& istream;
            std::ostream& ostream;
        };
    }
}
