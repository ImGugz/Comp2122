#ifndef __L22_TARGETS_SYMBOL_H__
#define __L22_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace l22 {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::string _name;
    long _value; // hack!
    std::vector<std::shared_ptr<cdk::basic_type>> _input_types;
    std::shared_ptr<cdk::basic_type> _output_type;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value) :
        _type(type), _name(name), _value(value) {
    }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    long value() const {
      return _value;
    }
    long value(long v) {
      return _value = v;
    }

    void set_input_types(const std::vector<std::shared_ptr<cdk::basic_type>> &types) {
      _input_types = types;
    }

    void set_output_type(const std::shared_ptr<cdk::basic_type> type) {
      _output_type = type;
    }

    std::shared_ptr<cdk::basic_type> input_type(size_t ax) const {
      return _input_types[ax];
    }

    const std::vector<std::shared_ptr<cdk::basic_type>>& input_types() const {
      return _input_types;
    }

    size_t number_of_inputs() const {
      return _input_types.size();
    }

    std::shared_ptr<cdk::basic_type> output_type() {
      return _output_type;
    }

  };

  inline auto make_symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value) {
    return std::make_shared<symbol>(type, name, value);
  }


} // l22

#endif
