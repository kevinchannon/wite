add_executable(io_examples
	"buffer_io.cpp"
)

target_include_directories(io_examples PRIVATE
	${CMAKE_SOURCE_DIR}
)
