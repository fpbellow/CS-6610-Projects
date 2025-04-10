# CMakeList.txt : CMake project for CS6610-Project1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CS6610-Project1")

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (CS6610-Project1 "CS6610-Project1.cpp" "src/Application.cpp" "src/ShaderCollection.cpp" "src/D3D11Application.cpp")

target_link_libraries(CS6610-Project1 PRIVATE glfw imgui::imgui)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CS6610-Project1 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
