/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

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
#include <iterator>

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

    SECTION("initializer-list construction") {
      using TestItems = identifiable_item_collection<TestItem>;
      const auto list_constructed = TestItems{TestItem{1}, TestItem{2}, TestItem{3}};
      REQUIRE(3 == list_constructed.size());
      REQUIRE(1 == list_constructed.at(TestItems::index_type{0}).id().value());
      REQUIRE(2 == list_constructed.at(TestItems::index_type{1}).id().value());
      REQUIRE(3 == list_constructed.at(TestItems::index_type{2}).id().value());
    }
  }

  SECTION("inserting items") {
    SECTION("try_insert") {
      SECTION("increases the size if successful") {
        REQUIRE(items.try_insert(item_0));

        REQUIRE(1 == items.size());

        SECTION("and the collection is not empty") {
          REQUIRE_FALSE(items.empty());
        }

        SECTION("and try_insert-ing another item with the same ID returns false") {
          REQUIRE_FALSE(items.try_insert(item_0));

          SECTION("and the size does not change") {
            REQUIRE(1 == items.size());
          }
        }
      }

      SECTION("item can be retreived by ID") {
        items.try_insert(item_0);

        const auto& retrieved_item = items.at(TestItem::id_type{1});
        REQUIRE(retrieved_item == item_0);
      }

      SECTION("multiple items can be try_insert-ed and retreived") {
        const auto items_to_insert = std::forward_list{item_0, item_1, item_2};

        const auto result = items.try_insert(items_to_insert);
        REQUIRE(std::vector{true, true, true} == result);

        for (const auto& expected : items_to_insert) {
          const auto& retrieved_item = items.at(expected.id());
          REQUIRE(expected == retrieved_item);
        }
      }

      SECTION("result of range try_insert contains false values where insertions fail") {
        const auto items_to_insert = std::array{item_0, item_1, item_2};

        items.try_insert(item_1);

        const auto result = items.try_insert(items_to_insert);
        REQUIRE(std::vector{true, false, true} == result);

        for (const auto& expected : items_to_insert) {
          const auto& retrieved_item = items.at(expected.id());
          REQUIRE(expected == retrieved_item);
        }
      }

      SECTION("multiple items can be try_insert-ed from a forward range") {
        const auto items_to_insert = std::forward_list{item_0, item_1, item_2};

        const auto result = items.try_insert(items_to_insert);
        REQUIRE(std::vector{true, true, true} == result);

        for (const auto& expected : items_to_insert) {
          const auto& retrieved_item = items.at(expected.id());
          REQUIRE(expected == retrieved_item);
        }
      }

      SECTION("std::out_of_range is thrown if an item with the required ID is not found") {
        WITE_REQ_THROWS(
            items.at(TestItem::id_type{3}), std::out_of_range, "identifiable_item_collection failed to retreive item by ID");
      }

      SECTION("variadic try_insert of items") {
        const auto result = items.try_insert(item_0, item_1, item_2);
        REQUIRE(std::array{true, true, true} == result);

        REQUIRE(3 == items.size());
        REQUIRE(item_0 == items.at(item_0.id()));
        REQUIRE(item_1 == items.at(item_1.id()));
        REQUIRE(item_2 == items.at(item_2.id()));
      }

      SECTION("variadic try_insert of items with existing IDs returns false") {
        // clang-format off
      const auto [existing_item, expected_insertion_result] = GENERATE_REF(table<TestItem, std::array<bool, 3>>({
        {item_0, std::array{false, true, true}},
        {item_1, std::array{true, false, true}},
        {item_2, std::array{true, true, false}}
      }));
        // clang-format on

        REQUIRE(items.try_insert(existing_item));

        const auto result = items.try_insert(item_0, item_1, item_2);
        REQUIRE(expected_insertion_result == result);

        REQUIRE(3 == items.size());
        REQUIRE(item_0 == items.at(item_0.id()));
        REQUIRE(item_1 == items.at(item_1.id()));
        REQUIRE(item_2 == items.at(item_2.id()));
      }
    }

    SECTION("insert") {
      SECTION("increases the size if successful") {
        REQUIRE_NOTHROW(items.insert(item_0));

        REQUIRE(1 == items.size());

        SECTION("and the collection is not empty") {
          REQUIRE_FALSE(items.empty());
        }

        SECTION("and try_insert-ing another item with the same ID returns false") {
          WITE_REQ_THROWS(items.insert(item_0), std::logic_error, "Insertion of duplicate ID into identifiable_item_collection");

          SECTION("and the size does not change") {
            REQUIRE(1 == items.size());
          }
        }

        SECTION("item can be retreived by ID") {
          const auto& retrieved_item = items.at(TestItem::id_type{1});
          REQUIRE(retrieved_item == item_0);
        }
      }
    }
  }

  SECTION("emplacing items") {
    SECTION("emplace an item increases the size") {
      const auto& item = items.emplace(5, "the data");
      REQUIRE(TestItem::id_type{5} == item.id());
      REQUIRE("the data" == item.data);

      REQUIRE(1 == items.size());
      REQUIRE(items.contains(TestItem::id_type{5}));

      SECTION("emplacing a second item increases the size further") {
        const auto& additional_item = items.emplace(3, "more data");
        REQUIRE(TestItem::id_type{3} == additional_item.id());
        REQUIRE("more data" == additional_item.data);

        REQUIRE(2 == items.size());
        REQUIRE(items.contains(TestItem::id_type{3}));
      }

      SECTION("emplacing another item with an existing ID throws std::invalid_argument") {
        WITE_REQ_THROWS(
            items.emplace(5, "other data"), std::logic_error, "identifiable_item_collection already contains this ID");

        REQUIRE("the data" == items.at(TestItem::id_type{5}).data);
      }
    }
  }

  SECTION("retreiving items") {
    using idx_t = identifiable_item_collection<TestItem>::index_type;

    SECTION("items can be retreived by index") {
      items.try_insert(item_0);

      const auto& retrieved_item = items.at(idx_t{0});
      REQUIRE(retrieved_item == item_0);
    }

    SECTION("std::out_of_range is thrown if an item with the required ID is not found") {
      WITE_REQ_THROWS(items.at(idx_t{0}), std::out_of_range, "identifiable_item_collection failed to retreive item by index");
    }
  }

  SECTION("removing items") {
    SECTION("clear removes all items") {
      items.try_insert(item_0, item_1, item_2);

      items.clear();
      REQUIRE(items.empty());
    }

    SECTION("erase item by ID") {
      items.try_insert(item_0, item_1, item_2);

      REQUIRE(items.erase(item_1.id()));
      REQUIRE(2 == items.size());

      REQUIRE(items.contains(item_0.id()));
      REQUIRE_FALSE(items.contains(item_1.id()));
      REQUIRE(items.contains(item_2.id()));
    }

    SECTION("erasing a non-existent item returns false") {
      items.try_insert(item_0, item_1, item_2);

      REQUIRE_FALSE(items.erase(TestItem::id_type{4}));
      REQUIRE(3 == items.size());
    }

    SECTION("erase a range of item IDs") {
      const auto item_3 = TestItem{4};
      const auto item_4 = TestItem{5};
      const auto item_5 = TestItem{6};

      items.try_insert(item_0, item_1, item_2, item_3, item_4, item_5);
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

      items.try_insert(item_0, item_1, item_2, item_3, item_4, item_5);

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

  SECTION("extracting items") {
    const auto item_3 = TestItem{4};
    const auto item_4 = TestItem{5};
    const auto item_5 = TestItem{6};

    items.try_insert(item_0, item_1, item_2, item_3, item_4, item_5);

    SECTION("reduces the size of the collection") {
      const auto item = items.excise(item_2.id());
      REQUIRE(5 == items.size());
      REQUIRE(item_2 == *item);
    }

    SECTION("returns NULL if the item does not exist") {
      REQUIRE(nullptr == items.excise(TestItem::id_type{7}));
    }
  }

  SECTION("finding items") {
    items.try_insert(item_0, item_1, item_2);

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

TEST_CASE("Identifiable item collection iterator tests", "[collections]") {
  auto items = identifiable_item_collection<TestItem>{TestItem{3}, TestItem{2}, TestItem{1}};

  SECTION("begin points to the first item that was added") {
    REQUIRE(3 == items.begin()->id().value());
    REQUIRE(3 == items.cbegin()->id().value());
  }

  SECTION("The distance between begin and end is equal to the size") {
    REQUIRE(items.size() == static_cast<size_t>(std::distance(items.begin(), items.end())));
    REQUIRE(items.size() == static_cast<size_t>(std::distance(items.cbegin(), items.cend())));
    REQUIRE(items.size() == static_cast<size_t>(std::distance(items.rbegin(), items.rend())));
    REQUIRE(items.size() == static_cast<size_t>(std::distance(items.crbegin(), items.crend())));
  }

  SECTION("check std algorithms")
  {
    SECTION("legacy") {
      SECTION("find_if") {
        const auto it = std::find_if(items.begin(), items.end(), [](auto&& item) { return item.id().value() == 2; });
        REQUIRE(2 == it->id().value());
      }
    }

    SECTION("range-based"){
      SECTION("find_if"){
        const auto it = std::ranges::find_if(items, [](auto&& item) { return item.id().value() == 2; });
        REQUIRE(2 == it->id().value());
      }
    }
  }

  SECTION("check range-based for"){
    const auto ids = {1u, 2u, 3u};
    auto id = ids.begin();
    for (const auto& item : items | std::views::reverse) {
      REQUIRE(*id++ == item.id().value());
    }
  }
}
