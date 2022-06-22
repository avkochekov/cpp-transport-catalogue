TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

unix: LIBS += -ltbb -lpthread

SOURCES += \
        transport_router.cpp \
        transport_catalogue.cpp \
	domain.cpp \
	geo.cpp \
	json.cpp \
	json_reader.cpp \
        json_builder.cpp \
	main.cpp \
	map_renderer.cpp \
	request_handler.cpp \
	svg.cpp

HEADERS += \
    test_queries.h \
        transport_router.h \
        transport_catalogue.h \
	domain.h \
	geo.h \
	json.h \
	json_reader.h \
        json_builder.h \
	map_renderer.h \
	request_handler.h \
        svg.h \
        ranges.h \
        router.h \
        graph.h
