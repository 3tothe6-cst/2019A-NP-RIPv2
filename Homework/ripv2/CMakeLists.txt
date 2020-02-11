add_executable(
        ripv2
        main.cpp
        format/common.hpp
        format/ip.hpp
        format/rip.hpp
        common.hpp
        debug.hpp
        environment.hpp
        exchanging.hpp
        forwarding.hpp
        hal.hpp
        table.hpp)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(ripv2 PRIVATE router_hal fmt::fmt spdlog::spdlog)
