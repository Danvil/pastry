CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_LIBRARY(pastry_deferred SHARED
	src/Camera.cpp
	src/DeferredRenderer.cpp
	src/GameObject.cpp
	src/GBuffer.cpp
	src/Geometry.cpp
	src/Light.cpp
	src/Scene.cpp
	src/SkyBox.cpp
)

INCLUDE_DIRECTORIES(
	include
	../core/include
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	TARGET_LINK_LIBRARIES(pastry_deferred
		pastry_core
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	TARGET_LINK_LIBRARIES(pastry_deferred
		pastry_core
		opengl32
		glu32
		gdi32
		glfw3dll
		glew32
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
