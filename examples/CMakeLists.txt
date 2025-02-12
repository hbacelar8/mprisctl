message(STATUS "→ Building example")

# Add example executable
add_executable(example)
target_sources(example PRIVATE main.cpp)
target_link_libraries(example PRIVATE mprisctl)
