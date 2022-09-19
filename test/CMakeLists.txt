Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.1.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(wite_test 
	"core/resultTests.cpp"
	"string/splitTests.cpp"
	"string/joinTests.cpp"
	"string/trimTests.cpp"
	"string/stripTests.cpp"
	"collections/stackVectorTests.cpp"
	"io/byteBufferTests.cpp"
    "io/byteStreamTests.cpp"
	"io/convertToBytesTests.cpp"
	"string/hexlifyTests.cpp"
	 "io/byteBufferReadTests.cpp"
	 "io/byteReadBufferViewTests.cpp"
	 "io/byteWriteBufferTests.cpp"
	 "io/byteWriteBufferViewTests.cpp"
)

target_include_directories(wite_test PUBLIC
	${CMAKE_SOURCE_DIR}
)

target_link_libraries(wite_test PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

catch_discover_tests(wite_test)
