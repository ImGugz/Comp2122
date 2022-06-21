#include <string>
#include <sstream>
#include "targets/postfix_writer.h"
#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

//--------------------------------------------------------------------------//
//                                CDK                                       //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  std::cout << "SEQUENCE" << std::endl;
  for (size_t ix = 0; ix < node->size(); ++ix) {
    node->node(i)->accept(this, lvl);
  }
}

void l22::postfix_writer::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void l22::postfix_writer::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                                LITERALS                                  //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_integer_node(cdk::integer_node *const node, int lvl) {
  std::cout << "INTEGER" << std::endl;
  if (_inFunctionBody) {
    _pf.INT(node->value());
  } else {
    _pf.SINT(node->value());
  }
}

void l22::postfix_writer::do_double_node(cdk::double_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_string_node(cdk::string_node *const node, int lvl) {
  std::cout << "STRING" << std::endl;
  std::string label = mklbl(++_lbl);
  _pf.RODATA();
  _pf.ALIGN();
  _pf.LABEL(label);
  _pf.SSTRING(node->value());
  if (_inFunctionBody) {
    _pf.TEXT();
    _pf.ADDR(label);
  } else {
    _pf.DATA();
    _pf.SADDR(label);
  }
}

void l22::postfix_writer::do_nullptr_node(l22::nullptr_node *const node, int lvl) {
  std::cout << "NULLPTR" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}

//--------------------------------------------------------------------------//
//                           UNARY ARITHMETIC EXPRESSIONS                   //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_neg_node(cdk::neg_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_identity_node(l22::identity_node * const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_not_node(cdk::not_node *const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                           BINARY ARITHMETIC EXPRESSIONS                  //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_add_node(cdk::add_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_sub_node(cdk::sub_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_mul_node(cdk::mul_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_div_node(cdk::div_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_mod_node(cdk::mod_node *const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                           BINARY LOGICAL EXPRESSIONS                     //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_le_node(cdk::le_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_and_node(cdk::and_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_or_node(cdk::or_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_program_node(l22::program_node *const node, int lvl) {
  std::cout << "PROGRAM" << std::endl;
  std::shared_ptr<l22::symbol> symbol;
  symbol = new_symbol();
  _fun_symbols.push_back(symbol);
  reset_new_symbol();

  _symtab.push(); // _level++; new context;
  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");

  frame_size_calculator lsc(_compiler, _symtab, symbol);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize());

  _inFunctionBody = true;
  if (node->statements()) {
    node->statements()->accept(this, lvl + 2);
  }
  _inFunctionBody = false;

  _symtab.pop();
  _pf.LEAVE();
  _pf.RET();

  _fun_symbols.pop_back();
  for (std::string ext : _external_functions) {
    _pf.EXTERN(ext);
  }
  _external_functions.clear();
}

void l22::postfix_writer::do_block_node(l22::block_node * const node, int lvl) {
  std::cout << "BLOCK" << std::endl;
  _symtab.push();
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
  _symtab.pop();
}

void l22::type_checker::do_evaluation_node(l22::evaluation_node *const node, int lvl) {
  // EMPTY
}

void l22::type_checker::do_write_node(l22::write_node *const node, int lvl) {
  std::cout << "WRITE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  for (int ix = node->arguments()->size() - 1; ix >= 0; --ix) {
    auto arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ix));
    arg->accept(this, lvl);
    if (arg->is_typed(cdk::TYPE_INT)) {
      _external_functions.push_back("printi");
      _pf.CALL("printi");
      _pf.TRASH(4);
    }
    else if (arg->is_typed(cdk::TYPE_DOUBLE)) {
      _external_functions.push_back("printd");
      _pf.CALL("printd");
      _pf.TRASH(8);
    }
    else if (arg->is_typed(cdk::TYPE_STRING)) {
      _external_functions.push_back("prints");
      _pf.CALL("prints");
      _pf.TRASH(4);
    }
    else {
      std::cerr << "cannot write expression of unknown type" << std::endl;
      return;
    }
  }
  if (node->newline()) {
    _external_functions.push_back("println");
    _pf.CALL("println");
  }
}

void l22::postfix_writer::do_input_node(l22::input_node *const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                           CYCLE INTRUCTION                               //
//--------------------------------------------------------------------------//

// NOTE: abstract all functions below??
void l22::postfix_writer::do_while_node(l22::while_node *const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                           CONDITIONAL INSTRUCTIONS                       //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_if_node(l22::if_node *const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_if_else_node(l22::if_else_node *const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                             FUNCTIONS                                    //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_function_call_node(l22::function_call_node * const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_function_definition_node(l22::function_definition_node * const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_return_node(l22::return_node * const node, int lvl) {
  std::cout << "RETURN" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  auto currFun = _fun_symbols.back();
  std::shared_ptr<cdk::basic_type> outputType = currFun->output_type();
  if (outputType->name() != cdk::TYPE_VOID) {
    node->retval()->accept(this, lvl);
    if (outputType->name() == cdk::TYPE_INT || outputType->name() == cdk::TYPE_DOUBLE ||
        outputType->name() == cdk::TYPE_POINTER) {
          _pf.STFVAL32();
    }
    else if(outputType->name() == cdk::TYPE_DOUBLE) {
      if (node->retval()->type()->name() == cdk::TYPE_INT) _pf.I2D();
      _pf.STFVAL64();
    }
  }
}

//--------------------------------------------------------------------------//
//                           BLOCK FLUX INSTRUCTIONS                        //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_stop_node(l22::stop_node * const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_again_node(l22::again_node * const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                           MEMORY RELATED                                 //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_index_node(l22::index_node * const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_stack_alloc_node(l22::stack_alloc_node * const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_address_of_node(l22::address_of_node * const node, int lvl) {
  // EMPTY
}

void l22::postfix_writer::do_sizeof_node(l22::sizeof_node * const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                          DECLARATION                                     //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_declaration_node(l22::declaration_node * const node, int lvl) {
  // EMPTY
}
