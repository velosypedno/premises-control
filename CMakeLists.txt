cmake_minimum_required(VERSION 3.20)
project(premises_control)

set(CMAKE_CXX_STANDARD 20)

find_package(SQLite3 REQUIRED)

add_executable(premises_control
        main.cpp
        User.h
        User.cpp
        UserRepo.h
        UserRepo.cpp
        uuid.h
        uuid.cpp
        Card.h
        Card.cpp
        CardRepo.h
        CardRepo.cpp)

target_link_libraries(premises_control PRIVATE SQLite3::SQLite3)