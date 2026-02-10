cmake_minimum_required(VERSION 3.16)
project(dt_psi CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Performance flags (adjust if needed)
add_compile_options(
    -O3
    -march=native
    -fno-omit-frame-pointer
)

add_subdirectory(vendor/boringssl)

add_library(maincrypto
    src/crypto/crypto_primitives.cc
    src/protocol/participant.cc
    src/protocol/keyholder.cc
)

target_include_directories(maincrypto
    PUBLIC
        src
        vendor/boringssl/include
)

target_link_libraries(maincrypto
    PRIVATE
        crypto
        ssl
)

add_executable(app src/main.cc)
target_link_libraries(app PRIVATE maincrypto)
