cmake_minimum_required(VERSION 3.20)
project(premises_control)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

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
        models/AccessPolicyRepo.cpp
        infra/Database.h
        infra/Database.cpp
        infra/Config.h
        infra/Config.cpp
        models/AccessLogRecord.h
        models/AccessLogRecord.cpp
        models/AccessLogRecordRepo.h
        models/AccessLogRecordRepo.cpp
        services/CardCheck.h
        services/CardCheck.cpp
        services/AdminCheck.h
        services/AdminCheck.cpp
        services/CryptoService.h
        services/CryptoService.cpp
        ui/ConsoleUI.h
        ui/ConsoleUI.cpp
        ui/AdminView.h
        ui/AdminView.cpp
        ui/UserView.h
        ui/UserView.cpp
        ui/AdminDashboard.h
        ui/AdminDashboard.cpp
        )

target_link_libraries(premises_control PRIVATE
        SQLite3::SQLite3
        OpenSSL::SSL
        OpenSSL::Crypto
        ftxui::screen
        ftxui::dom
        ftxui::component
)
