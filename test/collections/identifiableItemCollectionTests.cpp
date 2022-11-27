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

  explicit TestItem(size_t id, std::string data) : _id{std::move(id)}, data{std::move(data)} {} 

  explicit TestItem(size_t id) : TestItem{id, ""} {}

  id_type id() const { return _id; }

  auto operator<=>(const TestItem&) const = default;

  id_type _id;
  std::string data;
};

}  // namespace

using namespace wite::collections;

TEST_CASE("Identifiable item collection tests", "[collections]") {
  auto items        = identifiable_item_collection<TestItem>{};
  const auto item_0 = TestItem{1};
  const auto item_1 = TestItem{2};
  const auto item_2 = TestItem{3};

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
      REQUIRE(items.insert(item_0));

      REQUIRE(1 == items.size());

      SECTION("and the collection is not empty") {
        REQUIRE_FALSE(items.empty());
      }

      SECTION("and inserting another item with the same ID returns false") {
        REQUIRE_FALSE(items.insert(item_0));

        SECTION("and the size does not change") {
          REQUIRE(1 == items.size());
        }
      }
    }

    SECTION("inserted item can be retreived by ID") {
      items.insert(item_0);

      const auto& retrieved_item = items.at(TestItem::id_type{1});
      REQUIRE(retrieved_item == item_0);
    }

    SECTION("multiple items can be inserted and retreived") {
      const auto items_to_insert = std::forward_list{item_0, item_1, item_2};

      const auto result = items.insert(items_to_insert);
      REQUIRE(std::vector{true, true, true} == result);

      for (const auto& expected : items_to_insert) {
        const auto& retrieved_item = items.at(expected.id());
        REQUIRE(expected == retrieved_item);
      }
    }

    SECTION("result of range insert contains false values where insertions fail") {
      const auto items_to_insert = std::array{item_0, item_1, item_2};

      items.insert(item_1);

      const auto result = items.insert(items_to_insert);
      REQUIRE(std::vector{true, false, true} == result);

      for (const auto& expected : items_to_insert) {
        const auto& retrieved_item = items.at(expected.id());
        REQUIRE(expected == retrieved_item);
      }
    }

    SECTION("multiple items can be inserted from a forward range") {
      const auto items_to_insert = std::forward_list{item_0, item_1, item_2};

      const auto result = items.insert(items_to_insert);
      REQUIRE(std::vector{true, true, true} == result);

      for (const auto& expected : items_to_insert) {
        const auto& retrieved_item = items.at(expected.id());
        REQUIRE(expected == retrieved_item);
      }
    }

    SECTION("std::out_of_range is thrown if an item with the required ID is not found") {
      WITE_REQ_THROWS(items.at(TestItem::id_type{3}),
                      std::out_of_range,
                      "identifiable_item_collection failed to retreive item by ID");
    }

    SECTION("variadic insertion of items") {
      const auto result = items.insert(item_0, item_1, item_2);
      REQUIRE(std::array{true, true, true} == result);

      REQUIRE(3 == items.size());
      REQUIRE(item_0 == items.at(item_0.id()));
      REQUIRE(item_1 == items.at(item_1.id()));
      REQUIRE(item_2 == items.at(item_2.id()));
    }

    SECTION("variadic insertion of items with existing IDs returns false") {
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

  SECTION("emplacing items") {
    SECTION("emplace an item increases the size") {
      const auto& item = items.emplace(5, "the data");
      REQUIRE(TestItem::id_type{5} == item.id());
      REQUIRE("the data" == item.data);

      REQUIRE(1 == items.size());
      REQUIRE(items.contains(TestItem::id_type{5}));
    }
  }

  SECTION("retreiving items") {
    using idx_t = identifiable_item_collection<TestItem>::index_type;
    
    SECTION("items can be retreived by index") {
      items.insert(item_0);

      const auto& retrieved_item = items.at(idx_t{0});
      REQUIRE(retrieved_item == item_0);
    }

    SECTION("std::out_of_range is thrown if an item with the required ID is not found") {
      WITE_REQ_THROWS(identifiable_item_collection<TestItem>{}.at(idx_t{0}),
                      std::out_of_range,
                      "identifiable_item_collection failed to retreive item by index");
    }
  }

  SECTION("removing items") {
    SECTION("clear removes all items") {
      items.insert(item_0, item_1, item_2);

      items.clear();
      REQUIRE(items.empty());
    }

    SECTION("erase item by ID") {
      items.insert(item_0, item_1, item_2);

      REQUIRE(items.erase(item_1.id()));
      REQUIRE(2 == items.size());

      REQUIRE(items.contains(item_0.id()));
      REQUIRE_FALSE(items.contains(item_1.id()));
      REQUIRE(items.contains(item_2.id()));
    }

    SECTION("erasing a non-existent item returns false") {
      items.insert(item_0, item_1, item_2);

      REQUIRE_FALSE(items.erase(TestItem::id_type{4}));
      REQUIRE(3 == items.size());
    }

    SECTION("erase a range of item IDs") {
      const auto item_3 = TestItem{4};
      const auto item_4 = TestItem{5};
      const auto item_5 = TestItem{6};

      items.insert(item_0, item_1, item_2, item_3, item_4, item_5);
      const auto ids_to_erase = std::vector{item_1.id(), item_3.id(), item_4.id()};

      items.erase(ids_to_erase);
      REQUIRE(3 == items.size());

      REQUIRE(items.contains(item_0.id()));
      REQUIRE_FALSE(items.contains(item_1.id()));
      REQUIRE(items.contains(item_2.id()));
      REQUIRE_FALSE(items.contains(item_3.id()));
      REQUIRE_FALSE(items.contains(item_4.id()));
      REQUIRE(items.contains(item_5.id()));
    }

    SECTION("variadic erasure of some item IDs") {
      const auto item_3 = TestItem{4};
      const auto item_4 = TestItem{5};
      const auto item_5 = TestItem{6};

      items.insert(item_0, item_1, item_2, item_3, item_4, item_5);
      
      items.erase(item_1.id(), item_3.id(), item_4.id());
      REQUIRE(3 == items.size());

      REQUIRE(items.contains(item_0.id()));
      REQUIRE_FALSE(items.contains(item_1.id()));
      REQUIRE(items.contains(item_2.id()));
      REQUIRE_FALSE(items.contains(item_3.id()));
      REQUIRE_FALSE(items.contains(item_4.id()));
      REQUIRE(items.contains(item_5.id()));
    }
  }

  SECTION("finding items") {
    items.insert(item_0, item_1, item_2);

    SECTION("contains returns true if there is an item with the target ID in the collection") {
      REQUIRE(items.contains(item_0.id()));
      REQUIRE(items.contains(item_1.id()));
      REQUIRE(items.contains(item_2.id()));
    }

    SECTION("contains returns false if the is NO item with the target ID in the collection") {
      REQUIRE_FALSE(items.contains(TestItem::id_type{4}));
    }
  }
}
