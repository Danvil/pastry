CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_LIBRARY(pastry_core SHARED
	# pastry core
	src/obj.cpp
	src/pastry.cpp
	src/text.cpp
	# SOIL
	src/dep/SOIL2/etc1_utils.c
	src/dep/SOIL2/image_DXT.c
	src/dep/SOIL2/image_helper.c
	src/dep/SOIL2/SOIL2.c
	src/dep/SOIL2/stb_image.c
	src/dep/SOIL2/stb_image_write.c
)

INCLUDE_DIRECTORIES(include)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	TARGET_LINK_LIBRARIES(pastry_core
		glfw3
		GLEW
		GL
		X11 Xrandr Xi Xxf86vm Xcursor Xinerama
		pthread
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	TARGET_LINK_LIBRARIES(pastry_core
		opengl32
		glu32
		gdi32
		glfw3dll
		glew32
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
