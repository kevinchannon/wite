add_executable(io_examples
	"buffer_io.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET io_examples PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(io_examples PRIVATE
	${CMAKE_SOURCE_DIR}
)
