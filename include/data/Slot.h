#pragma once

#include "Type.h"

namespace soft {
  // Indicates a virtual register
  class Slot {
    public:
      Slot(Type type, size_t id);
      Slot();

      Type& getType();
      const Type& getType() const;
      size_t getId() const;

      void setType(Type type);
      void setId(size_t id);

    private:
      Type type;
      size_t id;
  };
}
