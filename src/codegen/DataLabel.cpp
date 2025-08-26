#include "codegen/DataLabel.h"
#include "common.h"

namespace soft {
  namespace codegen {
    Data::Data(Constant value)
      : value(std::move(value)) {}
    Data::Data() = default;

    Type& Data::getType() { return this->value.getType(); }
    Constant& Data::getValue() { return this->value; }

    const Type& Data::getType() const { return this->value.getType(); }
    const Constant& Data::getValue() const { return this->value; }

    void Data::setType(Type type) { this->value.setType(std::move(type)); }
    void Data::setValue(Constant constant) { this->value = std::move(constant); }

    std::string Data::toString() const
    {
      std::string result;
      const Type& type = getType();

      if (type.isInteger())
      {
        if (type.getBitwidth() == 8) result += ".byte";
        else if (type.getBitwidth() == 16) result += ".word";
        else if (type.getBitwidth() == 32) result += ".long";
        else if (type.getBitwidth() == 64) result += ".quad";
        else unreachable();

        result += std::format(" {}", this->value.getIntegerValue());
      }
      else // if (type.isFloatingPoint())
      {
        if (type.getBitwidth() == 32) result += ".float";
        else if (type.getBitwidth() == 64) result += ".double";
        else unreachable();

        result += std::format(" {}", this->value.getFloatValue());
      }

      result += '\n';
      return result;
    }

    DataLabel::DataLabel(std::string name, std::vector<Data> data)
      : name(std::move(name)), data(std::move(data)) {}
    DataLabel::DataLabel() = default;

    std::string DataLabel::getName() const { return this->name; }
    std::vector<Data> DataLabel::getData() const { return this->data; }

    void DataLabel::setName(std::string name) { this->name = std::move(name); }
    void DataLabel::setData(std::vector<Data> data) { this->data = std::move(data); }

    std::string DataLabel::toString() const
    {
      std::string result = std::format("{}:\n", this->getName());

      for (const auto& elm : data)
        result += std::format("  {}", elm.toString());

      return result;
    }
  }
}
