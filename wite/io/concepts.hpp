#pragma once

#include <wite/env/features.hpp>
#include <wite/common/concepts.hpp>
#include <wite/io/encoding.hpp>

#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename T>
_WITE_CONCEPT is_encoded = std::is_base_of_v<io::encoding, T>;

template <typename T>
_WITE_CONCEPT is_buffer_readable = common::is_pod_like<T> || (is_encoded<T> && common::is_pod_like<typename T::value_type>);

template <typename T>
_WITE_CONCEPT is_buffer_writeable = common::is_pod_like<T> || (is_encoded<T> && common::is_pod_like<typename T::value_type>);

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
