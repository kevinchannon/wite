#include <wite/collections/identifiable_item_collection.hpp>
#include <wite/core/id.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace {

  struct TestItem {
    using id_type = wite::id<TestItem, size_t>;

    explicit TestItem(id_type id) : _id{std::move(id)} {}

    id_type id() const { return _id; }

    id_type _id;
  };

}

using namespace wite::collections;

TEST_CASE("Identifiable item collection tests", "[collections]") {
  SECTION("default constructed collection has zero size") {
    REQUIRE(0 == identifiable_item_collection<TestItem>{}.size());
  }

  SECTION("default constructed collection is empty") {
    REQUIRE(identifiable_item_collection<TestItem>{}.empty());
  }

  SECTION("insert an item increases the size") {
    auto items = identifiable_item_collection<TestItem>{};

    items.insert(TestItem{TestItem::id_type{1}});

    REQUIRE(1 == items.size());

    SECTION("and the collection is not empty") {
      REQUIRE_FALSE(items.empty());
    }
  }
}
