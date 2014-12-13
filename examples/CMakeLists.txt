CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE_DIRECTORIES(
	include
	../modules/core/include
	../modules/sprites/include
	../modules/postfx/include
	../modules/deferred/include
)

FUNCTION(CreateExample NAME)
	SET(SAMPLE_NAME pastry-example-${NAME})
	ADD_EXECUTABLE(${SAMPLE_NAME} ${NAME}.cpp)
	TARGET_LINK_LIBRARIES(${SAMPLE_NAME} pastry_core)
ENDFUNCTION(CreateExample)

CreateExample(hello)

CreateExample(quad)

CreateExample(particles)

CreateExample(texture)

CreateExample(mesh)

CreateExample(kitten)
TARGET_LINK_LIBRARIES(pastry-example-kitten pastry_sprites)

CreateExample(sprites)
TARGET_LINK_LIBRARIES(pastry-example-sprites pastry_sprites)

CreateExample(text)

CreateExample(input)

CreateExample(pong)

CreateExample(postfx)
TARGET_LINK_LIBRARIES(pastry-example-postfx pastry_sprites pastry_postfx)

CreateExample(gravity)

CreateExample(crowd)

CreateExample(mosaik)
TARGET_LINK_LIBRARIES(pastry-example-mosaik pastry_sprites)

CreateExample(deferred)
TARGET_LINK_LIBRARIES(pastry-example-deferred pastry_deferred)
