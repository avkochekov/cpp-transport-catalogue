// Чтение запросов на заполнение базы
#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <list>
#include "transport_catalogue.h"

namespace catalogue{
    namespace reader {
        class InputRaeader {
        public:
            InputRaeader(TransportCatalogue& catalogue, std::istream& stream = std::cin) : catalogue{catalogue}, stream{stream} {};
            void ReadQueues(size_t count, std::istream& = std::cin);

        private:
            void ReadStops(const std::vector<std::string>& queue);
            void ReadBuses(const std::vector<std::string>& queue);

            std::vector<std::string> ReadStops(const std::string_view queue, const char separator);

            std::string_view Simplified(const std::string_view);
            std::deque<std::string_view> Split(const std::string_view text, const char separator);
            std::deque<std::string_view> Split(const std::string_view text, const std::string_view separator);

            TransportCatalogue &catalogue;
            std::istream& stream;
        };
    }
}
