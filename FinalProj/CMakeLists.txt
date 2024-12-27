cmake_minimum_required(VERSION 3.0)
project(lab2)

find_package(OpenGL REQUIRED)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


add_subdirectory(external)

include_directories(
		external/glfw-3.1.2/include/
		external/glm-0.9.7.1/
		external/glad-opengl-3.3/include/
		external/tinygltf-2.9.3/
		external/
		FinalProj/
)

# Target for lab2_building
add_executable(lab2_building
		FinalProj/building/building.cpp
		FinalProj/render/shader.cpp
		FinalProj/road/road.cpp
		FinalProj/windmill/windmill.cpp
		FinalProj/windmill/blades.cpp
)

target_link_libraries(lab2_building
		${OPENGL_LIBRARY}
		glfw
		glad
)

# Target for main
add_executable(main
		FinalProj/main.cpp
		FinalProj/render/shader.cpp
		FinalProj/skybox/skybox.h
		FinalProj/skybox/skybox.cpp
		FinalProj/building/building.h
		FinalProj/building/building.cpp
		FinalProj/road/road.h
		FinalProj/road/road.cpp
		FinalProj/windmill/windmill.h
		FinalProj/windmill/windmill.cpp
		FinalProj/windmill/blades.h
		FinalProj/windmill/blades.cpp
		FinalProj/model/model.h
		FinalProj/model/model.cpp
)

target_link_libraries(main
		${OPENGL_LIBRARY}
		glfw
		glad
)