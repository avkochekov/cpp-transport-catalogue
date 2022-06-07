#pragma once

#include <unordered_set>

#include "map_renderer.h"
#include "transport_catalogue.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

using catalogue::TransportCatalogue;
using renderer::MapRenderer;
using renderer::MapRenderSettings;

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(catalogue::TransportCatalogue &db, renderer::MapRenderer &renderer);

    void AddStop(const std::string& name, const double lat, const double lon);
    void AddDistanceBetweenStops(const std::string& from, const std::string& to, const double distance);
    template<typename Container>
    void AddBus(const std::string& name, const bool is_roundtrip, const Container &stops);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через остановку
    const std::optional<StopStat> GetStopStat(const std::string_view& stop_name) const;

    void SetRendererSettings(const MapRenderSettings& settings);
    void RenderMap(std::ostream &stream) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    catalogue::TransportCatalogue& catalogue_;
    renderer::MapRenderer& renderer_;
};

template<typename Container>
inline void RequestHandler::AddBus(const std::string &name, const bool is_roundtrip, const Container &stops)
{
    catalogue_.AddBus(name, is_roundtrip ? RouteType::Circle : RouteType::Linear, stops);
}
