#include <wite/collections/identifiable_item_collection.hpp>
#include <wite/core/id.hpp>
#include <test/utils.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <compare>
#include <vector>
#include <stdexcept>

namespace {

  struct TestItem {
    using id_type = wite::id<TestItem, size_t>;

    explicit TestItem(id_type id) : _id{std::move(id)} {}

    id_type id() const { return _id; }

    auto operator<=>(const TestItem&) const = default;

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

  SECTION("inserted item can be retreived by ID") {
    auto items = identifiable_item_collection<TestItem>{};
    const auto item = TestItem{TestItem::id_type{1}};
    items.insert(item);

    const auto& retrieved_item = items.at(TestItem::id_type{1});
    REQUIRE(retrieved_item == item);
  }

  SECTION("multiple items can be inserted and retreived") {
    auto items      = identifiable_item_collection<TestItem>{};
    const auto item_1 = TestItem{TestItem::id_type{1}};
    const auto item_2 = TestItem{TestItem::id_type{2}};
    const auto item_3 = TestItem{TestItem::id_type{3}};

    items.insert(std::vector{item_1, item_2, item_3});

    {
      const auto& retrieved_item = items.at(TestItem::id_type{1});
      REQUIRE(retrieved_item == item_1);
    }

    {
      const auto& retrieved_item = items.at(TestItem::id_type{3});
      REQUIRE(retrieved_item == item_3);
    }

    {
      const auto& retrieved_item = items.at(TestItem::id_type{2});
      REQUIRE(retrieved_item == item_2);
    }
  }

  SECTION("std::out_of_range is thrown if an item with the required ID is not found") {
    WITE_REQ_THROWS(identifiable_item_collection<TestItem>{}.at(TestItem::id_type{1}), std::out_of_range, "identifiable_item_collection failed to retreive item");
  }
}
