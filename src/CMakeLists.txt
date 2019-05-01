cmake_minimum_required (VERSION 3.8)

add_executable (TICAv022 "Controller.cpp" "ControllerImpl.cpp" "ControllerImpl_ConnectionEditor.cpp" "ControllerImpl_StateEditor.cpp" "geomobj_impl.cpp" "geom_impl.cpp" "gl_driver.cpp" "ImageLoader.cpp" "imgui_impl_glfw.cpp" "imgui_impl_opengl3.cpp" "InFactoryQueryBuilder.cpp" "main.cpp" "os_dep.cpp" "PointCloud.cpp" "renderer_impl.cpp" "Texture.cpp" "Util.cpp" "World.cpp" "Common.h" "Controller.h" "ControllerImpl.h" "ControllerImpl_ConnectionEditor.h" "ControllerImpl_StateEditor.h" "Geometry.h" "GeomObjects.h" "imgui_impl_glfw.h" "imgui_impl_opengl3.h" "InFactoryQueryBuilder.h" "PointCloud.h" "Renderer.h" "Texture.h" "Util.h" "World.h")

set ( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} opengl32.lib glu32.lib bcrypt.lib" )