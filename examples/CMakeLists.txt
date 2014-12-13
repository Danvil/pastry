CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FUNCTION(CreateExample NAME)
	SET(SAMPLE_NAME pastry-example-${NAME})
	INCLUDE_DIRECTORIES(
		include
		../modules/core/include
	)
	ADD_EXECUTABLE(${SAMPLE_NAME} ${NAME}.cpp)
	TARGET_LINK_LIBRARIES(${SAMPLE_NAME} pastry_core)
ENDFUNCTION(CreateExample)

CreateExample(hello)
CreateExample(quad)
CreateExample(particles)
CreateExample(texture)
CreateExample(mesh)
CreateExample(kitten)
CreateExample(sprites)
CreateExample(text)
CreateExample(input)
CreateExample(pong)
CreateExample(post)
CreateExample(gravity)
CreateExample(crowd)
CreateExample(mosaik)

CreateExample(deferred)
INCLUDE_DIRECTORIES(
	../modules/deferred/include
)
TARGET_LINK_LIBRARIES(pastry-example-deferred pastry_deferred)
