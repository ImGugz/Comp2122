#ifndef __CDK17_TYPES_TYPES_H__
#define __CDK17_TYPES_TYPES_H__

#include <cdk/types/basic_type.h>
#include <cdk/types/primitive_type.h>
#include <cdk/types/reference_type.h>
#include <cdk/types/structured_type.h>
#include <cdk/types/functional_type.h>
#include <memory>

namespace cdk {

  inline std::string to_string(std::shared_ptr<basic_type> type) {
    if (type->name() == TYPE_INT) return "integer";
    if (type->name() == TYPE_DOUBLE) return "double";
    if (type->name() == TYPE_STRING) return "string";
    if (type->name() == TYPE_VOID) return "void";
    if (type->name() == TYPE_POINTER) {
      auto r = cdk::reference_type::cast(type)->referenced();
      return "pointer to " + to_string(r);
    } else {
      return "(unknown or unsupported type)";
    }
  }

} // cdk

#endif
