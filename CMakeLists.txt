set(supported_languages C CXX)

cmake_minimum_required(VERSION 3.26)
project(sc3020_version3
        LANGUAGES ${supported_languages})

set(CMAKE_C_STANDARD 11)

include_directories("SC3020 DB Project 1")
include_directories(simple_version)

add_executable(sc3020_version3
        "SC3020 DB Project 1/address.c"
        "SC3020 DB Project 1/block.c"
        "SC3020 DB Project 1/block.h"
        "SC3020 DB Project 1/disk.c"
        "SC3020 DB Project 1/LRUCache.c"
        "SC3020 DB Project 1/LRUCache.h"
        "SC3020 DB Project 1/record.c"
        simple_version/bt_node.c
        simple_version/bt_node.h
        simple_version/bt_page.c
        simple_version/bt_page.h
        simple_version/storage.c
        simple_version/storage.h
        simple_version/tree_v2.c)
