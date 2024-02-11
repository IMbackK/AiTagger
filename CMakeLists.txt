cmake_minimum_required(VERSION 3.23)

project(aimusictagger LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 20)

find_package(Llama REQUIRED)
find_package(tagparser REQUIRED)


set(SRC_FILES
	main.cpp
	utils.cpp
	bar.cpp
	llmai.cpp
	tag.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC NETWORK_DIR="${CMAKE_CURRENT_SOURCE_DIR}/net" PROG_NAME="${PROJECT_NAME}")
target_link_libraries(${PROJECT_NAME} ${llama_LIBRARY} ${TAG_PARSER_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ${LLAMA_INCLUDE_DIRS} ${TAG_PARSER_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
