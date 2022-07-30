#include <sstream>
#include <iostream>
#include <fstream>

#include "json_reader.h"
#include "request_handler.h"

#include "test_queries.h"

using namespace std::literals;
using json::Document;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {

    std::string file_name = "s14_2_opentest_1";

    {
        const std::string_view mode("make_base");
        std::ifstream stream("/home/anton/documents/YandexPracticum/Tests/" + file_name +"_make_base.json");

        std::istream &istream = stream;
        std::ostream &ostream = std::cout;

        TransportCatalogue catalogue;
        MapRenderer renderer;
        TransportRouter router;

        RequestHandler handler(catalogue, renderer, router);
        JsonReader reader(handler);
        auto requests = reader.ParseRequest(istream);

        if (mode == "make_base"sv) {
            if (requests.count("base_requests"))
                reader.BaseRequestHandler(requests.at("base_requests"));
            if (requests.count("render_settings"))
                reader.RenderSettingsRequestHandler(requests.at("render_settings"));
            if (requests.count("routing_settings"))
                reader.RoutingSettingsHandler(requests.at("routing_settings"));
            handler.Serialize(reader.SerializationSettings(requests.at("serialization_settings")));
        } else if (mode == "process_requests"sv) {
            handler.Deserialize(reader.SerializationSettings(requests.at("serialization_settings")));
            reader.StatRequestHandler(requests.at("stat_requests"), ostream);
        } else {
            PrintUsage();
            return 1;
        }
    }

    {
        const std::string_view mode("process_requests");
        std::ifstream in_stream("/home/anton/documents/YandexPracticum/Tests/" + file_name +"_process_requests.json");
        std::ofstream out_stream("/home/anton/documents/YandexPracticum/Tests/" + file_name +"_result.json");
        std::ifstream ans_stream("/home/anton/documents/YandexPracticum/Tests/" + file_name +"_answer.json");

        TransportCatalogue catalogue;
        MapRenderer renderer;
        TransportRouter router;

        RequestHandler handler(catalogue, renderer, router);
        JsonReader reader(handler);
        auto requests = reader.ParseRequest(in_stream);

        if (mode == "make_base"sv) {
            reader.BaseRequestHandler(requests.at("base_requests"));
            reader.RenderSettingsRequestHandler(requests.at("render_settings"));
    //        reader.RoutingSettingsHandler(requests.at("routing_settings"));
            handler.Serialize(reader.SerializationSettings(requests.at("serialization_settings")));
        } else if (mode == "process_requests"sv) {
            handler.Deserialize(reader.SerializationSettings(requests.at("serialization_settings")));
            reader.StatRequestHandler(requests.at("stat_requests"), out_stream);
        } else {
            PrintUsage();
            return 1;
        }
    }
}
