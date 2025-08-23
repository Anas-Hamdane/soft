#include "data/Slot.h"
#include <bits/move.h>

namespace soft {
  Slot::Slot(Type type, size_t id)
    : type(std::move(type)), id(id) {}
  Slot::Slot() = default;

  Type& Slot::getType() { return this->type; }
  const Type& Slot::getType() const { return this->type; }
  size_t Slot::getId() const { return this->id; }

  void Slot::setType(Type type) { this->type = std::move(type); }
  void Slot::setId(size_t id) { this->id = id; }
}
