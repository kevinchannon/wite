add_executable(core_examples
	"uuid.cpp"
)

target_include_directories(core_examples PRIVATE
	${CMAKE_SOURCE_DIR}
)
