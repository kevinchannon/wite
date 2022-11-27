#include <test/utils.hpp>
#include <wite/collections/identifiable_item_collection.hpp>
#include <wite/core/id.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <compare>
#include <stdexcept>
#include <vector>
#include <forward_list>

namespace {

struct TestItem {
  using id_type = wite::id<TestItem, size_t>;

  explicit TestItem(id_type id) : _id{std::move(id)} {}

  id_type id() const { return _id; }

  auto operator<=>(const TestItem&) const = default;

  id_type _id;
};

}  // namespace

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

    REQUIRE(items.insert(TestItem{TestItem::id_type{1}}));

    REQUIRE(1 == items.size());

    SECTION("and the collection is not empty") {
      REQUIRE_FALSE(items.empty());
    }

    SECTION("and inserting another item with the same ID returns false") {
      REQUIRE_FALSE(items.insert(TestItem{TestItem::id_type{1}}));

      SECTION("and the size does not change") {
        REQUIRE(1 == items.size());
      }
    }
  }

  SECTION("inserted item can be retreived by ID") {
    auto items      = identifiable_item_collection<TestItem>{};
    const auto item = TestItem{TestItem::id_type{1}};
    items.insert(item);

    const auto& retrieved_item = items.at(TestItem::id_type{1});
    REQUIRE(retrieved_item == item);
  }

  SECTION("multiple items can be inserted and retreived") {
    auto items = identifiable_item_collection<TestItem>{};
    const auto items_to_insert =
        std::forward_list{TestItem{TestItem::id_type{1}}, TestItem{TestItem::id_type{2}}, TestItem{TestItem::id_type{3}}};

    const auto result = items.insert(items_to_insert);
    REQUIRE(std::vector{true, true, true} == result);

    for (const auto& expected : items_to_insert) {
      const auto& retrieved_item = items.at(expected.id());
      REQUIRE(expected == retrieved_item);
    }
  }

  SECTION("result of range insert contains false values where insertions fail") {
    auto items = identifiable_item_collection<TestItem>{};
    const auto items_to_insert =
        std::array{TestItem{TestItem::id_type{1}}, TestItem{TestItem::id_type{2}}, TestItem{TestItem::id_type{3}}};

    items.insert(items_to_insert[1]);

    const auto result = items.insert(items_to_insert);
    REQUIRE(std::vector{true, false, true} == result);
    
    for (const auto& expected : items_to_insert) {
      const auto& retrieved_item = items.at(expected.id());
      REQUIRE(expected == retrieved_item);
    }
  }

  SECTION("multiple items can be inserted from a forward range") {
    auto items = identifiable_item_collection<TestItem>{};
    const auto items_to_insert =
        std::array{TestItem{TestItem::id_type{1}}, TestItem{TestItem::id_type{2}}, TestItem{TestItem::id_type{3}}};

    items.insert(items_to_insert[1]);

    const auto result = items.insert(items_to_insert);
    REQUIRE(std::vector{true, false, true} == result);

    for (const auto& expected : items_to_insert) {
      const auto& retrieved_item = items.at(expected.id());
      REQUIRE(expected == retrieved_item);
    }
  }

  SECTION("std::out_of_range is thrown if an item with the required ID is not found") {
    WITE_REQ_THROWS(identifiable_item_collection<TestItem>{}.at(TestItem::id_type{1}),
                    std::out_of_range,
                    "identifiable_item_collection failed to retreive item by ID");
  }

  SECTION("items can be retreived by index") {
    using idx_t = identifiable_item_collection<TestItem>::index_type;

    auto items      = identifiable_item_collection<TestItem>{};
    const auto item = TestItem{TestItem::id_type{1}};
    items.insert(item);

    const auto& retrieved_item = items.at(idx_t{0});
    REQUIRE(retrieved_item == item);
  }

  SECTION("std::out_of_range is thrown if an item with the required ID is not found") {
    using idx_t = identifiable_item_collection<TestItem>::index_type;

    WITE_REQ_THROWS(identifiable_item_collection<TestItem>{}.at(idx_t{0}),
                    std::out_of_range,
                    "identifiable_item_collection failed to retreive item by index");
  }
}
