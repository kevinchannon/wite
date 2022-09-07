#pragma once

#if __cpp_concepts >= 201907
#define _WITE_HAS_CONCEPTS 1
#define _WITE_CONCEPT      concept
#else
#define _WITE_HAS_CONCEPTS 0
#define _WITE_CONCEPT      constexpr auto
#endif

#if __cpp_lib_ranges >= 201911
#define _WITE_HAS_RANGES 1
#else
#define _WITE_HAS_RANGES 0
#endif