###############################################################################
CMAKE_MINIMUM_REQUIRED ( VERSION 2.6 )
PROJECT ( terrainConstraintNode )


###############################################################################
IF (APPLE)
	ADD_DEFINITIONS ( -DMAC_PLUGIN -DREQUIRE_IOSTREAM -include ${MAYA_INCLUDE_DIR}/maya/OpenMayaMac.h )
ELSE (APPLE)
	ADD_DEFINITIONS ( -m64 -DLINUX -DREQUIRE_IOSTREAM -O2 -fomit-frame-pointer )
ENDIF (APPLE)

###############################################################################
INCLUDE_DIRECTORIES ( src )
#INCLUDE_DIRECTORIES ( ${OPENGL_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${MAYA_INCLUDE_DIR} )
SET( EXECUTABLE_OUTPUT_PATH release/$ENV{DRD_PLATFORM}/${MAYA_VERSION} )
SET( LIBRARY_OUTPUT_PATH release/$ENV{DRD_PLATFORM}/${MAYA_VERSION} )

###############################################################################
# The actual Node

ADD_LIBRARY ( terrainConstraintNode SHARED
	src/terrainConstraintNode.cpp
	src/terrainConstraintPlugin.cpp
	src/softwareLookUp.cpp
	src/utils.cpp
	src/timer.cpp
	src/singleton.cpp
)

TARGET_LINK_LIBRARIES ( terrainConstraintNode
	${MAYA_LIBRARIES}
	${GCOV_LIB}
)
