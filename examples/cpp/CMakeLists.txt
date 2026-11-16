cmake_minimum_required(VERSION 3.16)
project(msz_download LANGUAGES CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
add_executable(msz-download download.cpp)
target_compile_features(msz-download PRIVATE cxx_std_17)
target_link_libraries(msz-download PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
