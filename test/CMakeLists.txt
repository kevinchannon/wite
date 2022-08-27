Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.1.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(wite_test
	"string/splitTests.cpp"
	"string/joinTests.cpp"
	"string/trimTests.cpp"
	"string/stripTests.cpp"
	"collections/stackVectorTests.cpp"
	"io/bytesBufferTests.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET wite_test PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(wite_test PRIVATE
	${CMAKE_SOURCE_DIR}
)

target_link_libraries(wite_test PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

catch_discover_tests(wite_test)
