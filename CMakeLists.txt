cmake_minimum_required(VERSION 3.18)
project(xAnatomy LANGUAGES CXX)

add_definitions(     
	-DDATA_DIR="${PROJECT_SOURCE_DIR}/data/"	
)

add_subdirectory(src)
