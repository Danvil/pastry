CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_LIBRARY(pastry SHARED
	# pastry
	pastry.cpp
	sprites.cpp
	text.cpp
	postfx.cpp
	obj.cpp
	# pastry.deferred
	deferred/DeferredRenderer.cpp
	deferred/GBuffer.cpp
	deferred/Geometry.cpp
	deferred/Light.cpp
	# SOIL
	dep/SOIL2/etc1_utils.c
	dep/SOIL2/image_DXT.c
	dep/SOIL2/image_helper.c
	dep/SOIL2/SOIL2.c
	dep/SOIL2/stb_image.c
	dep/SOIL2/stb_image_write.c
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	TARGET_LINK_LIBRARIES(pastry
		glfw
		GLEW
		GL
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	TARGET_LINK_LIBRARIES(pastry
		opengl32
		glu32
		gdi32
		glfw3dll
		glew32
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
