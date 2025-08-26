#include "stl.h"
#include "data/Constant.h"

namespace soft {
  namespace codegen {
    class Data {
      public:
        Data(Constant value);
        Data();

        Type& getType();
        Constant& getValue();

        const Type& getType() const;
        const Constant& getValue() const;

        void setType(Type type);
        void setValue(Constant constant);

        std::string toString() const;

      private:
        // the Constant already has a type
        Constant value;
    };
    class DataLabel {
      public:
        DataLabel(std::string name, std::vector<Data> data);
        DataLabel();

        std::string getName() const;
        std::vector<Data> getData() const;

        void setName(std::string name);
        void setData(std::vector<Data> data);

        std::string toString() const;

      private:
        std::string name;
        std::vector<Data> data;
    };
  }
}
