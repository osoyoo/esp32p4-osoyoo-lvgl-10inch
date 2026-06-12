# Top-level project CMake for ESP-IDF 5.5.x
# NOTE: SquareLine's "Export UI Files" drops its own CMakeLists.txt into main/ui/.
# That file is for SquareLine's PC build only — do NOT let it overwrite THIS file.
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(osoyoo_dsi_lvgl)
