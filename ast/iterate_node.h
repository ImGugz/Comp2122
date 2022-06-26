#ifndef __L22_AST_ITERATE_NODE_H__
#define __L22_AST_ITERATE_NODE_H__

#include <cdk/ast/expression_node.h>

namespace l22 {

  /**
   * Class for describing while-cycle nodes.
   */
  class iterate_node: public cdk::basic_node {
    cdk::expression_node *_vector, *_count, *_condition;
    std::string _function;

  public:
    inline iterate_node(int lineno, cdk::expression_node *vector, cdk::expression_node *count, std::string &function, cdk::expression_node *condition) :
        basic_node(lineno), _vector(vector), _count(count), _condition(condition), _function(function) {
    }

  public:
    inline cdk::expression_node *vector() {
      return _vector;
    }
    inline cdk::expression_node *count() {
      return _count;
    }
     inline cdk::expression_node *condition() {
      return _condition;
    }
    inline std::string function() {
      return _function;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_iterate_node(this, level);
    }

  };

} // l22

#endif
