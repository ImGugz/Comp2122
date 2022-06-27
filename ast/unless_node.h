#ifndef __L22_AST_UNLESS_NODE_H__
#define __L22_AST_UNLESS_NODE_H__

#include <cdk/ast/expression_node.h>

namespace l22 {

  /**
   * Class for describing unless instruction nodes.
   */
  class unless_node: public cdk::basic_node {
    cdk::expression_node *_condition, *_pointer, *_count, *_function;

  public:
    inline unless_node(int lineno, cdk::expression_node *condition, cdk::expression_node *pointer,
                                   cdk::expression_node *count, cdk::expression_node *function) :
        basic_node(lineno), _condition(condition), _pointer(pointer), _count(count), _function(function) {
    }

  public:
    inline cdk::expression_node *condition() {
      return _condition;
    }
    inline cdk::expression_node *pointer() {
      return _pointer;
    }
    inline cdk::expression_node *count() {
      return _count;
    }
    inline cdk::expression_node *function() {
      return _function;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_unless_node(this, level);
    }

  };

} // l22

#endif
