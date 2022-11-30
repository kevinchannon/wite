#include <wite/collections/identifiable_item_collection.hpp>
#include <wite/core/id.hpp>
#include <wite/core/io.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A single sensor that has a value and an ID.
/// </summary>
struct Sensor {
  using id_type = wite::id<Sensor, uint8_t>;

  const id_type _id{};
  size_t value{};

  explicit Sensor(id_type::value_type id) : _id{id} {}

  const id_type& id() const { return _id; }
};

using SensorCollection = wite::collections::identifiable_item_collection<Sensor>;

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A single servo that has current and target positions and an ID.
/// </summary>
struct Servo : public wite::id<Servo, uint8_t> {
  using id_type = wite::id<Servo, uint8_t>;

  id_type _id{};
  size_t current_position{};
  size_t target_position{};

  explicit Servo(id_type::value_type id, size_t target) : _id{id}, target_position{target} {}

  const id_type& id() const { return _id; }
};

using ServoCollection = wite::collections::identifiable_item_collection<Servo>;

///////////////////////////////////////////////////////////////////////////////

void populate(SensorCollection& sensors) {
  // Values are inserted by value, so if you don't need to use the local value again, you can move it in.
  auto sensor_1 = Sensor{1};
  sensors.insert(std::move(sensor_1));

  // This would throw std::logic_error, because the ID already exists in the collection.
  // sensors.insert(Sensor{1});

  // No local variable for this one, just construct in the call.
  sensors.insert(Sensor{2});

  // Insert multiple values in one call, if you know ahead of time what you want to add.
  const auto insert_result = sensors.try_insert(Sensor{3}, Sensor{4}, Sensor{5});
  assert(std::ranges::all_of(insert_result, [](auto&& result) { return true == result; }));
}

///////////////////////////////////////////////////////////////////////////////

void populate(ServoCollection& servos) {
  // emplace constructs values directly in the memory used for storage.
  auto& servo            = servos.emplace(2, 1000);
  servo.current_position = 500;  // emplace returns a stable reference to the thing that was inserted into the collection, so you
                                 // can edit and use it, if you need to.

  auto servos_vec = std::vector{Servo{1, 0}, Servo{2, 1000}, Servo{3, 500}};

  // insert items from an existing range. The second element of the result will be FALSE, since a servo with that element's ID
  // already existed in the collection
  const auto result = servos.try_insert(std::move(servos_vec));
}

///////////////////////////////////////////////////////////////////////////////

int main() {
  // Some empty collections
  auto sensors = SensorCollection{};
  auto servos  = ServoCollection{};

  populate(sensors);
  populate(servos);

  //
  // Check the size of the collections
  //
  std::cout << "Added " << sensors.size() << " sensors" << std::endl;
  std::cout << "Added " << servos.size() << " servos" << std::endl;

  //
  // Check that particular items are present
  //
  const auto id_1 = Sensor::id_type{2};
  const auto id_2 = Servo::id_type{3};

  // Compilation will fail if we mix up these (poorly named) IDs. So, you can't write something like `sensors.contains(id_2);`
  // because it won't compile.
  if (not sensors.contains(id_1) or not servos.contains(id_2)) {
    std::cerr << "Failed to find necessary sensor or servo" << std::endl;
    return 1;
  }

  //
  // Get using the ID. Again, the IDs can't be confused in these calls, or compilation will fail
  //
  const auto& sensor = sensors.at(id_1);
  const auto& servo  = servos.at(id_2);

  //
  // NOTE: stream insertion of id types is possible, but you need to #include <wite/core/io.hpp> to get it.
  //
  std::cout << "Have sensor with ID " << sensor.id() << std::endl;
  std::cout << "Have servo with ID " << servo.id() << std::endl;
  //
  // Loop over a collection.
  //
  // COMING SOON! Iterators, so that you don't have to type this safe, but verbose, stuff :)
  for (auto i = SensorCollection::index_type{0}; i < SensorCollection::index_type{sensors.size()}; ++i) {
    const auto& s =
        sensors.at(i);  // This is an overload of at() that takes an index value. Like the id values, the indices are typed to the
                        // container that they index into, so you can't accidentally use an index intended for use in the servo
                        // collection to get an object from the sensor collection; it won't compile.
    std::cout << "Sensor " << s.id() << ", value: " << s.value << std::endl;
  }

  //
  // Remove items from a collection
  //
  sensors.erase(id_1);  // Typed-ID here means that it's not possible to accidentally put `id_2` here, or compilation will fail.

  // Erase multiple elements (returns an array of boolean values indicating whether the item was erased, or not.
  // `result` <-- [true, false], because we already erased id_1 from the collection.
  const auto result = sensors.erase(Sensor::id_type{3}, id_1);

  // To remove an item, but keep hold of it locally, then an item can be excised from the collection. NOTE: this is not called
  // `extract`, which would seem to mirror standard library containers more closely. However, extract has some specific behaviours
  // and uses that excise does not really implement at all, so it seems safer to give it a different name.
  auto local_servo = servos.excise(id_2);

  // Clear everything, if you need to do that, for some reason.
  servos.clear();
  

  return 0;
}
