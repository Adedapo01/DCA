################################################################################
# Author: Urs R. Haehner (haehneru@itp.phys.ethz.ch)
#
# Checks for external libraries and creates a global lists of them and the corresponding include
# paths.
# We explicitely search for static libraries because compute nodes cannot load shared libraries from
# e.g. the project directory.

set(DCA_EXTERNAL_LIBS "" CACHE INTERNAL "")
set(DCA_EXTERNAL_INCLUDE_DIRS "" CACHE INTERNAL "")

################################################################################
# Lapack
if (NOT DCA_HAVE_LAPACK)
  find_package(LAPACK REQUIRED)
endif()

mark_as_advanced(LAPACK_LIBRARIES)
message("LAPACK_LIBRARIES: ${LAPACK_FOUND} ${LAPACK_LINKER_FLAGS} ${LAPACK_LIBRARIES} ${LAPACK95_LIBRARIES}")
list(APPEND DCA_EXTERNAL_LIBS ${LAPACK_LIBRARIES})

################################################################################
# Blas
if (NOT DCA_HAVE_BLAS)
  find_package(BLAS REQUIRED)
endif()

mark_as_advanced(LAPACK_LIBRARIES)
message("LAPACK_LIBRARIES: ${LAPACK_FOUND} ${LAPACK_LINKER_FLAGS} ${LAPACK_LIBRARIES} ${LAPACK95_LIBRARIES}")
list(APPEND DCA_EXTERNAL_LIBS ${LAPACK_LIBRARIES})

################################################################################
# HDF5
# Find HDF5 by looking for a CMake configuration file (hdf5-1.10.x).
find_package(HDF5 COMPONENTS C CXX NO_MODULE QUIET)
if (NOT HDF5_FOUND)
  # Fall back to a search for a FindHDF5.cmake file and execute it.
  find_package(HDF5 REQUIRED COMPONENTS C CXX)
endif()

list(APPEND DCA_EXTERNAL_LIBS ${HDF5_LIBRARIES})
list(APPEND DCA_EXTERNAL_INCLUDE_DIRS ${HDF5_INCLUDE_DIRS})

################################################################################
# ADIOS2
if (DCA_WITH_ADIOS2)
set(DCA_HAVE_ADIOS2 FALSE CACHE INTERNAL "")
find_package(ADIOS2)
if (ADIOS2_FOUND)
  list(APPEND DCA_EXTERNAL_LIBS ${ADIOS2_LIBRARIES})
  list(APPEND DCA_EXTERNAL_INCLUDE_DIRS ${ADIOS2_INCLUDE_DIRS})
  set(DCA_HAVE_ADIOS2 TRUE CACHE INTERNAL "")
  #message("ADIOS2: libraries ${ADIOS2_LIBRARIES}")
endif()
endif()

################################################################################
################################################################################
# FFTW
set(FFTW_INCLUDE_DIR "" CACHE PATH "Path to fftw3.h.")
set(FFTW_LIBRARY "" CACHE FILEPATH "The FFTW3(-compatible) library.")

list(APPEND DCA_EXTERNAL_LIBS ${FFTW_LIBRARY})
list(APPEND DCA_EXTERNAL_INCLUDE_DIRS ${FFTW_INCLUDE_DIR})

################################################################################
# Simplex GM Rule
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/simplex_gm_rule)
# list(APPEND DCA_EXTERNAL_LIBS ${SIMPLEX_GM_RULE_LIBRARY})
# list(APPEND DCA_EXTERNAL_INCLUDE_DIRS ${SIMPLEX_GM_RULE_INCLUDE_DIR})

################################################################################
# Gnuplot
list(APPEND DCA_EXTERNAL_LIBS ${GNUPLOT_INTERFACE_LIBRARY})
list(APPEND DCA_EXTERNAL_INCLUDE_DIRS ${GNUPLOT_INTERFACE_INCLUDE_DIR})

# message("DCA_EXTERNAL_LIBS = ${DCA_EXTERNAL_LIBS}")
# message("DCA_EXTERNAL_INCLUDE_DIRS = ${DCA_EXTERNAL_INCLUDE_DIRS}")
