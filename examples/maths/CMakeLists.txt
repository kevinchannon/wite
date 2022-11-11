add_executable(value_range_examples
	"value_range.cpp"
)

target_include_directories(value_range_examples PRIVATE
	${CMAKE_SOURCE_DIR}
)
