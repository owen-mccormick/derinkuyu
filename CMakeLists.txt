cmake_minimum_required(VERSION 3.10)
project(derinkuyu VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(derinkuyu src/Main.cpp src/Actor.cpp src/Map.cpp src/Worker.cpp src/AStar.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(libtcod CONFIG REQUIRED)

target_link_libraries(derinkuyu PRIVATE libtcod::libtcod)
