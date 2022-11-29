add_executable(collections_examples
	"identifiable_item_collection.cpp"
)

target_include_directories(collections_examples PRIVATE
	${CMAKE_SOURCE_DIR}
)
