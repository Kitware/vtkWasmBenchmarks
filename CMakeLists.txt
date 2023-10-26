cmake_minimum_required(VERSION 3.13)
project(ConesViewer)

set(VUE_COMPONENTS_DIR "${PROJECT_SOURCE_DIR}/src/components")
set(DEBUGINFO "NONE" CACHE STRING "Type of debug info")
set_property(CACHE DEBUGINFO PROPERTY
  STRINGS
    NONE              # -g0
    READABLE_JS       # -g1
    PROFILE           # -g2
    DEBUG_NATIVE      # -g3
)
set(OPTIMIZE "BEST" CACHE STRING "Emscripten optimization")
set_property(CACHE OPTIMIZE PROPERTY
  STRINGS
    NO_OPTIMIZATION       # -O0
    LITTLE                # -O1
    MORE                  # -O2
    BEST                  # -O3
    SMALL                 # -Os
    SMALLEST              # -Oz
    SMALLEST_WITH_CLOSURE # -Oz --closure 1
)

add_subdirectory("cpp/ConesViewer")
#add_subdirectory("cpp/GeometryViewer")