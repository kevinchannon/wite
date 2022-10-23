add_executable(string_examples
	"fragment_string.cpp"
)

target_include_directories(string_examples PRIVATE
	${CMAKE_SOURCE_DIR}
)
