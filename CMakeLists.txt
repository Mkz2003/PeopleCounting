cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME PeopleCounting)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_CURRENT_LIST_DIR}/USER/MyCode.c
    ${CMAKE_CURRENT_LIST_DIR}/USER/lcd_spi_200.c
    ${CMAKE_CURRENT_LIST_DIR}/USER/dcmi_ov5640.c
    ${CMAKE_CURRENT_LIST_DIR}/USER/sccb.c

    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/apriltag.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/array.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/binary.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/blob.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/clahe.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/collections.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/draw_dewarp.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/draw.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/edge.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/filter.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/fmath.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/haar.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/hough.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/imlib.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/integral_mw.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/integral.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/jdata_conf.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/lab_tab.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/line.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/matd.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/mathop.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/mve_binary.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/mve_draw.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/mve_filter.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/mve_imlib.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/mve_resize.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/point.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/pool.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/rectangle.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/sincos_tab.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stats.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_binary.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_blob.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_convert.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_dewarp.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_drawing.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_edge.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_equalization.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_filtering.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_find_pixels.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_geometry.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_hough.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_image_io_jpg_sw.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_image_io.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_integral.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_masking.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_math_op.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_mem_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_morph_op.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_object_det.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_point.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_rect.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_resize.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_rotation.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_stats.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_template.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl_warping.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/stm32ipl.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/template.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/umm_malloc.c
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Src/xyz_tab.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_LIST_DIR}/USER/
    ${CMAKE_CURRENT_LIST_DIR}/stm32-mw-ipl/Inc/
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
    # Add user defined symbols
    STM32IPL
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)



# 如果需要，可以定义 SDRAM 起始地址供代码使用
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    SDRAM_BASE=0xC0000000
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -u _printf_float)

