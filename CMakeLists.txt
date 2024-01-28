# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(weather_assistant_zephyr)

file(GLOB SRC_FILES    
    "src/*.c"
)

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)


include(${ZEPHYR_BASE}/cmake/cfb.cmake NO_POLICY_SCOPE)
generate_cfb_font_for_target(app fonts/04B.ttf ${gen_dir}/cfb_mono_04B.h 12 16 -s 12 --first 32 --last 126 --name cfb_custom_1)
generate_cfb_font_for_target(app fonts/COMICBD.ttf ${gen_dir}/cfb_mono_COMICBD.h 13 16 -s 12 --first 32 --last 126 --name cfb_custom_2)

target_sources(app PRIVATE ${SRC_FILES})
