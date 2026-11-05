cmake_minimum_required(VERSION 3.20)
project(premises_control)

set(CMAKE_CXX_STANDARD 20)

find_package(SQLite3 REQUIRED)

add_executable(premises_control
        main.cpp
        models/User.h
        models/User.cpp
        models/UserRepo.h
        models/UserRepo.cpp
        models/uuid.h
        models/uuid.cpp
        models/Card.h
        models/Card.cpp
        models/CardRepo.h
        models/CardRepo.cpp
        models/AccessPolicy.h
        models/AccessPolicy.cpp
        models/AccessPolicyRepo.h
        models/AccessPolicyRepo.cpp)

target_link_libraries(premises_control PRIVATE SQLite3::SQLite3)