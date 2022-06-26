#ifndef __L22_AST_WITH_NODE_H__
#define __L22_AST_WITH_NODE_H__

#include <cdk/ast/expression_node.h>

namespace l22 {

  /**
   * Class for describing while-cycle nodes.
   */
  class with_node: public cdk::basic_node {
    std::string _function;
    cdk::expression_node *_vector, *_low, *_high;

  public:
    inline with_node(int lineno, const std::string &function, cdk::expression_node *vector, cdk::expression_node *low, cdk::expression_node *high) :
        basic_node(lineno), _function(function), _vector(vector), _low(low), _high(high) {
    }

  public:
    inline std::string function() {
      return _function;
    }
    inline cdk::expression_node *vector() {
      return _vector;
    }
    inline cdk::expression_node *low() {
      return _low;
    }
    inline cdk::expression_node *high() {
      return _high;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_with_node(this, level);
    }

  };

} // l22

#endif
