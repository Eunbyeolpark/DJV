set(source GLFWTest.cpp)

add_executable(GLFWTest ${header} ${source})
target_link_libraries(GLFWTest
    GLFW
    GLM
    Threads::Threads)
set_target_properties(
    GLFWTest
    PROPERTIES
    FOLDER tests
    CXX_STANDARD 11)

