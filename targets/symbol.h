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
    int _qualifier;
  
   // std::vector<std::shared_ptr<cdk::basic_type>> _input_types;
   // std::shared_ptr<cdk::basic_type> _output_type;
   // bool _initialized;
   // bool _is_redcl;
   // bool _is_decl;
    bool _is_extern;
    bool _is_foreign;
    bool _is_main;

    int _offset = 0;


  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value, int qualifier) :
        _type(type), _name(name), _value(value), _qualifier(qualifier), _is_extern(false), _is_foreign(false), _is_main(false) {
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
    int qualifier() {
      return _qualifier;
    }

    void set_extern(bool val) {
      _is_extern = true;
    }

    bool is_extern() {
      return _is_extern;
    }

    void set_foreign(bool val) {
      _is_foreign = true;
    }

    bool is_foreign() {
      return _is_foreign;
    }

    void set_main(bool val) {
      _is_main = true;
    }

    bool is_main() {
      return _is_main;
    }

    int offset() const {
      return _offset;
    }
    
    bool global() const {
      return _offset == 0;
    }

    void set_offset(int offset) {
      _offset = offset;
    }

  };

  inline auto make_symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value, int qualifier) {
    return std::make_shared<symbol>(type, name, value, qualifier);
  }


} // l22

#endif
