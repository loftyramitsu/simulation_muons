cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(MuonDetector)

# -------------------------------------------------------
#  Geant4
# -------------------------------------------------------
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# -------------------------------------------------------
#  Sources & headers
# -------------------------------------------------------
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

# -------------------------------------------------------
#  Executable
# -------------------------------------------------------
add_executable(sim ${sources} ${headers})
target_link_libraries(sim ${Geant4_LIBRARIES})

# -------------------------------------------------------
#  Copier les macros dans le répertoire de build
# -------------------------------------------------------
foreach(_mac init_vis.mac run.mac)
    configure_file(
        ${PROJECT_SOURCE_DIR}/${_mac}
        ${PROJECT_BINARY_DIR}/${_mac}
        COPYONLY
    )
endforeach()
