set(NEWMAT_LIBRARIES newmat)

if(NOT TARGET newmat)
	include("${CMAKE_CURRENT_LIST_DIR}/FindlibNewmat.cmake")
endif()

