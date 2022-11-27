#include <test/utils.hpp>
#include <wite/collections/identifiable_item_collection.hpp>
#include <wite/core/id.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <compare>
#include <forward_list>
#include <stdexcept>
#include <vector>

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
  SECTION("construction") {
    SECTION("default constructed collection has zero size") {
      REQUIRE(0 == identifiable_item_collection<TestItem>{}.size());
    }

    SECTION("default constructed collection is empty") {
      REQUIRE(identifiable_item_collection<TestItem>{}.empty());
    }
  }

  SECTION("inserting items") {
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

    SECTION("variadic insertion of items") {
      auto items        = identifiable_item_collection<TestItem>{};
      const auto item_0 = TestItem{TestItem::id_type{1}};
      const auto item_1 = TestItem{TestItem::id_type{2}};
      const auto item_2 = TestItem{TestItem::id_type{3}};

      const auto result = items.insert(item_0, item_1, item_2);
      REQUIRE(std::array{true, true, true} == result);

      REQUIRE(3 == items.size());
      REQUIRE(item_0 == items.at(item_0.id()));
      REQUIRE(item_1 == items.at(item_1.id()));
      REQUIRE(item_2 == items.at(item_2.id()));
    }

    SECTION("variadic insertion of items with existing IDs returns false") {
      auto items        = identifiable_item_collection<TestItem>{};
      const auto item_0 = TestItem{TestItem::id_type{1}};
      const auto item_1 = TestItem{TestItem::id_type{2}};
      const auto item_2 = TestItem{TestItem::id_type{3}};

      // clang-format off
      const auto [existing_item, expected_insertion_result] = GENERATE_REF(table<TestItem, std::array<bool, 3>>({
        {item_0, std::array{false, true, true}},
        {item_1, std::array{true, false, true}},
        {item_2, std::array{true, true, false}}
      }));
      // clang-format on

      REQUIRE(items.insert(existing_item));

      const auto result = items.insert(item_0, item_1, item_2);
      REQUIRE(expected_insertion_result == result);

      REQUIRE(3 == items.size());
      REQUIRE(item_0 == items.at(item_0.id()));
      REQUIRE(item_1 == items.at(item_1.id()));
      REQUIRE(item_2 == items.at(item_2.id()));
    }
  }

  SECTION("retreiving items") {
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
}
