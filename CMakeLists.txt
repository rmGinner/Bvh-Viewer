cmake_minimum_required(VERSION 2.8)

project(bvhviewer)
find_package(OpenGL)
find_package(GLUT)
add_executable(${PROJECT_NAME} "main.c")
target_link_libraries(bvhviewer ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
