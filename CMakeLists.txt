cmake_minimum_required (VERSION 3.8)

project ("Aptabase-Cpp")

add_library(AptabaseCpp 
	"./sources/model.cpp"
)
target_link_libraries(AptabaseCpp 
	PUBLIC nlohmann_json 
)
target_include_directories(AptabaseCpp 
	PUBLIC "./include"
)
target_precompile_headers(AptabaseCpp 
	PUBLIC "./include/aptabase/model.hpp"
)