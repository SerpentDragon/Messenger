cmake_minimum_required(VERSION 3.10)

project(Server VERSION 1.0.0)

find_package(Boost COMPONENTS system thread program_options REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

set(PROJECT_SOURCES
    main.cpp

    DB_Manager/database.cpp
    DB_Manager/database.h

    ServerUtils/messagequeue.cpp
    ServerUtils/messagequeue.h
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system Boost::thread Boost::program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PQXX_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ../API/)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
