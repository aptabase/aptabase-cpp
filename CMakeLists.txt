cmake_minimum_required (VERSION 3.8)

project ("Aptabase-Cpp")

add_library(AptabaseCpp 
	"./sources/model.cpp"
	"./sources/analytics.cpp"
	"./sources/provider/provider.cpp"
	"./sources/provider/manual_provider.cpp"
	"./sources/provider/auto_provider.cpp"
	"./sources/provider/tick_provider.cpp"
	"./sources/provider/worker_provider.cpp"
	"./sources/net/client.cpp"
)

if(CMAKE_APTABASE_USE_HTTPLIB)
	target_sources(AptabaseCpp PRIVATE "./sources/net/httplib.cpp")

	target_link_libraries(AptabaseCpp PUBLIC httplib)
	target_compile_definitions(AptabaseCpp PUBLIC APTABASE_USE_HTTPLIB=1)
endif()

if(CMAKE_APTABASE_USE_BOOST)
	target_sources(AptabaseCpp PRIVATE "./sources/net/boost.cpp")

	find_package(Boost REQUIRED)
	find_package(OpenSSL REQUIRED)

	target_link_directories(AptabaseCpp PUBLIC ${Boost_LIBRARY_DIR_RELEASE})
	target_include_directories(AptabaseCpp PUBLIC ${OPENSSL_INCLUDE_DIR} PUBLIC ${Boost_INCLUDE_DIR})
	target_link_libraries(AptabaseCpp PUBLIC ${OPENSSL_LIBRARIES} PUBLIC ${Boost_LIBRARIES})

	target_compile_definitions(AptabaseCpp PUBLIC APTABASE_USE_BOOST=1)
endif()

target_include_directories(AptabaseCpp 
	PUBLIC "./include"
)
target_precompile_headers(AptabaseCpp 
	PUBLIC "./include/aptabase/model.hpp"
	PUBLIC "./include/aptabase/analytics.hpp"
)
target_compile_features(AptabaseCpp PRIVATE cxx_std_17)