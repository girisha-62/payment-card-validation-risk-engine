cmake_minimum_required(VERSION 3.20)

project(PaymentCardValidationRiskEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# Windows + MSYS2 UCRT64:
# Install with:
#   pacman -S mingw-w64-ucrt-x86_64-postgresql
#
# If PostgreSQL is installed elsewhere, pass:
#   -DPOSTGRESQL_INCLUDE_DIR=...
#   -DPOSTGRESQL_LIBRARY=...
set(POSTGRESQL_INCLUDE_DIR
    "C:/msys64/ucrt64/include"
    CACHE PATH "PostgreSQL client include directory")

set(POSTGRESQL_LIBRARY
    "C:/msys64/ucrt64/lib/libpq.dll.a"
    CACHE FILEPATH "PostgreSQL client import library")

if(NOT EXISTS "${POSTGRESQL_INCLUDE_DIR}/libpq-fe.h")
    message(FATAL_ERROR
        "PostgreSQL header not found: ${POSTGRESQL_INCLUDE_DIR}/libpq-fe.h\n"
        "Install MSYS2 UCRT64 PostgreSQL client with:\n"
        "  pacman -S mingw-w64-ucrt-x86_64-postgresql")
endif()

if(NOT EXISTS "${POSTGRESQL_LIBRARY}")
    message(FATAL_ERROR
        "PostgreSQL library not found: ${POSTGRESQL_LIBRARY}")
endif()

add_library(payment_core
    src/card/card_validator.cpp
    src/risk/risk_engine.cpp
    src/store/transaction_store.cpp
    src/thread/thread_pool.cpp
    src/transaction/transaction_processor.cpp
)

target_include_directories(payment_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${POSTGRESQL_INCLUDE_DIR}"
)

target_link_libraries(payment_core PUBLIC
    Threads::Threads
    "${POSTGRESQL_LIBRARY}"
)

add_executable(payment-risk-engine
    src/main.cpp
)

target_include_directories(payment-risk-engine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${POSTGRESQL_INCLUDE_DIR}"
)

target_link_libraries(payment-risk-engine PRIVATE payment_core)

if(WIN32)
    add_custom_command(TARGET payment-risk-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Program Files/PostgreSQL/18/bin/libpq.dll"
        "$<TARGET_FILE_DIR:payment-risk-engine>/libpq.dll"
        VERBATIM)
endif()

enable_testing()
