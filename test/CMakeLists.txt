cmake_minimum_required(VERSION 3.15)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.1.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(wite_test 
	"binascii/binasciiTests.cpp"
  "binascii/binasciiTests_noExceptions.cpp"
	"collections/stackVectorTests.cpp"
	"collections/identifiableItemCollectionTests.cpp"
	"collections/makeVectorTests.cpp"
	"collections/staticLookupTests.cpp"
	"core/resultTests.cpp"
	"core/scopeTests.cpp"
	"core/indexTests.cpp"
	"core/idTests.cpp"
	"core/uuidTests.cpp"
	"core/uuidTests_noExceptions.cpp"
	"geometry/PointTests.cpp"
	"geometry/IoTests.cpp"
	"io/byteBufferTests.cpp"
  "io/byteStreamTests.cpp"
	"io/convertToBytesTests.cpp"
	"io/byteBufferReadTests.cpp"
	"io/byteReadBufferViewTests.cpp"
	"io/byteWriteBufferTests.cpp"
	"io/byteWriteBufferViewTests.cpp"
	"io/byteBufferReadTests_noExceptions.cpp"
	"io/byteBufferFileTests.cpp"
	"io/encodingTests.cpp"
	"string/splitTests.cpp"
	"string/joinTests.cpp"
	"string/trimTests.cpp"
	"string/stripTests.cpp"
	"string/hexlifyTests.cpp"
	"string/fragmentStringTest.cpp"
	"maths/valueRangeTests.cpp"
	"maths/ioTests.cpp"
	"maths/numericTests.cpp"
	"maths/boundedValueTests.cpp"
)

target_compile_options(wite_test PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(wite_test PUBLIC
	${CMAKE_SOURCE_DIR}
)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
add_compile_definitions(_WITE_ASSERT_WITH_EXCEPTION)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

target_link_libraries(wite_test PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

catch_discover_tests(wite_test)
