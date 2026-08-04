cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/src"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/discus"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/discus_ambilight"
)

set(COMPONENTS
    src
    discus
    discus_ambilight
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(discus_application_ambilight)
