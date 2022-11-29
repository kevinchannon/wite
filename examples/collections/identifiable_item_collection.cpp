#include <wite/collections/identifiable_item_collection.hpp>
#include <wite/core/id.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
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

  return 0;
}
