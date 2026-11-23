cmake_minimum_required(VERSION 3.20)
project(premises_control)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0169 OLD)
FetchContent_Declare(
  sqlite_amalgamation
  URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
  URL_HASH SHA256=77823cb110929c2bcb0f5d48e4833b5c59a8a6e40cdea3936b99e199dbbe5784
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(sqlite_amalgamation)
if(NOT sqlite_amalgamation_POPULATED)
  FetchContent_Populate(sqlite_amalgamation)
endif()

add_library(sqlite3_amalg STATIC
  ${sqlite_amalgamation_SOURCE_DIR}/sqlite3.c
)
target_include_directories(sqlite3_amalg PUBLIC ${sqlite_amalgamation_SOURCE_DIR})
target_compile_definitions(sqlite3_amalg PUBLIC SQLITE_THREADSAFE=1)

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
        services/Result.h
        services/UserService.h
        services/UserService.cpp
        services/CardService.h
        services/CardService.cpp
        ui/ConsoleUI.h
        ui/ConsoleUI.cpp
        ui/AdminView.h
        ui/AdminView.cpp
        ui/UserView.h
        ui/UserView.cpp
        ui/AdminDashboard.h
        ui/AdminDashboard.cpp
        ui/AdminEditProfile.h
        ui/AdminEditProfile.cpp
        ui/AdminManageUsers.h
        ui/AdminManageUsers.cpp
        ui/AdminManageCards.h
        ui/AdminManageCards.cpp
        ui/AdminManagePolicies.h
        ui/AdminManagePolicies.cpp
        ui/AdminAccessLog.h
        ui/AdminAccessLog.cpp
        ui/UserDashboard.h
        ui/UserDashboard.cpp
        ui/UserEditProfile.h
        ui/UserEditProfile.cpp
        ui/UserChangePin.h
        ui/UserChangePin.cpp
        ui/UserCardsView.h
        ui/UserCardsView.cpp
        ui/UserPolicyView.h
        ui/UserPolicyView.cpp
        )

target_link_libraries(premises_control PRIVATE
        sqlite3_amalg
        ftxui::screen
        ftxui::dom
        ftxui::component
)

add_executable(verify_plan
        verify_plan.cpp
        models/User.cpp
        models/UserRepo.cpp
        models/uuid.cpp
        models/Card.cpp
        models/CardRepo.cpp
        models/AccessPolicy.cpp
        models/AccessPolicyRepo.cpp
        infra/Database.cpp
        infra/Config.cpp
        models/AccessLogRecord.cpp
        models/AccessLogRecordRepo.cpp
        services/CardCheck.cpp
        services/AdminCheck.cpp
        services/CryptoService.cpp
        services/UserService.cpp
        services/CardService.cpp
)
target_link_libraries(verify_plan PRIVATE sqlite3_amalg)
