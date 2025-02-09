message(STATUS "→ Building basic example")

# Add basic example executable
add_executable(basic)
target_sources(basic PRIVATE main.cpp)
target_link_libraries(basic PRIVATE mprisctl)
