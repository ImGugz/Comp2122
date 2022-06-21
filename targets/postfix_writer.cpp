#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include "targets/symbol.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

//--------------------------------------------------------------------------//
//                                CDK                                       //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  std::cout << "SEQUENCE" << std::endl;
  for (size_t ix = 0; ix < node->size(); ++ix) {
    node->node(ix)->accept(this, lvl);
  }
}

void l22::postfix_writer::do_nil_node(cdk::nil_node *const node, int lvl) {}

void l22::postfix_writer::do_data_node(cdk::data_node *const node, int lvl) {}

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
  std::cout << "DOUBLE" << std::endl;
  if (_inFunctionBody) {
    _pf.DOUBLE(node->value());
  } else {
    _pf.SDOUBLE(node->value());
  }
}

void l22::postfix_writer::do_string_node(cdk::string_node *const node, int lvl) {
  std::cout << "STRING" << std::endl;
  std::string lbl = mklbl(++_lbl);
  _pf.RODATA();
  _pf.ALIGN();
  _pf.LABEL(lbl);
  _pf.SSTRING(node->value());
  if (_inFunctionBody) {
    _pf.TEXT();
    _pf.ADDR(lbl);
  } else {
    _pf.DATA();
    _pf.SADDR(lbl);
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
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _pf.NEG();
}

void l22::postfix_writer::do_identity_node(l22::identity_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
}

void l22::postfix_writer::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ();
}

//--------------------------------------------------------------------------//
//                           BINARY ARITHMETIC EXPRESSIONS                  //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_add_node(cdk::add_node *const node, int lvl) {
  std::cout << "ADD" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    auto referenced = cdk::reference_type::cast(node->right()->type())->referenced();
    _pf.INT(referenced->size());
    _pf.MUL();
  }

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto referenced = cdk::reference_type::cast(node->right()->type())->referenced();
    _pf.INT(referenced->size());
    _pf.MUL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DADD();
  } else {
    _pf.ADD();
  }
}

void l22::postfix_writer::do_sub_node(cdk::sub_node *const node, int lvl) {
  std::cout << "SUB" << std::endl;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
    auto referenced = cdk::reference_type::cast(node->right()->type())->referenced();
    _pf.INT(referenced->size());
    _pf.MUL();
  }

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto referenced = cdk::reference_type::cast(node->right()->type())->referenced();
    _pf.INT(referenced->size());
    _pf.MUL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DSUB();
  } else {
    _pf.SUB();
  }
}

void l22::postfix_writer::do_mul_node(cdk::mul_node *const node, int lvl) {
  std::cout << "MUL" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DMUL();
  } else {
    _pf.MUL();
  }
}

void l22::postfix_writer::do_div_node(cdk::div_node *const node, int lvl) {
  std::cout << "DIV" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DDIV();
  } else {
    _pf.DIV();
  }
}

void l22::postfix_writer::do_mod_node(cdk::mod_node *const node, int lvl) {
  std::cout << "MOD" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  _pf.MOD();
}

//--------------------------------------------------------------------------//
//                           BINARY LOGICAL EXPRESSIONS                     //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl) {
  std::cout << "LT" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  _pf.LT();
}

void l22::postfix_writer::do_le_node(cdk::le_node *const node, int lvl) {
  std::cout << "LE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  _pf.LE();
}

void l22::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl) {
  std::cout << "GE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  _pf.GE();
}

void l22::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl) {
  std::cout << "GT" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  _pf.GT();
}

void l22::postfix_writer::do_and_node(cdk::and_node *const node, int lvl) {
  std::cout << "AND" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  std::string lbl = mklbl(++_lbl);
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(lbl);
  node->right()->accept(this, lvl + 2);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(lbl);
}

void l22::postfix_writer::do_or_node(cdk::or_node *const node, int lvl) {
  std::cout << "OR" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  std::string lbl = mklbl(++_lbl);
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(lbl);
  node->right()->accept(this, lvl + 2);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(lbl);
}

void l22::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl) {
  std::cout << "NE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  _pf.NE();
}

void l22::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl) {
  std::cout << "EQ" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.I2D();
  }
  _pf.EQ();
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl) {
  std::cout << "VARIABLE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  const std::string &id = node->name();
  auto symbol = _symtab.find(id);
  if (symbol->global()) {
    _pf.ADDR(symbol->name());
  } else {
    _pf.LOCAL(symbol->offset());
  }
}

void l22::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  std::cout << "RVALUE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDDOUBLE();
  } else {
    _pf.LDINT();
  }
}

void l22::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  std::cout << "ASSIGNMENT" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      _pf.I2D();
    }
    _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl); // TODO: check lvl
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_program_node(l22::program_node *const node, int lvl) {
  std::cout << "PROGRAM" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
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

void l22::postfix_writer::do_evaluation_node(l22::evaluation_node *const node, int lvl) {
  std::cout << "EVALUATION" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.TRASH(node->argument()->type()->size());
}

void l22::postfix_writer::do_write_node(l22::write_node *const node, int lvl) {
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
  std::cout << "INPUT" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  if (node->is_typed(cdk::TYPE_INT)) {
    _external_functions.push_back("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  }
  else if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _external_functions.push_back("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else {
    std::cerr << "CANNOT READ INPUT TYPE" << std::endl;
  }
}

//--------------------------------------------------------------------------//
//                           CYCLE INTRUCTION                               //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_while_node(l22::while_node *const node, int lvl) {
  std::cout << "WHILE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;

  _whileCond.push(++_lbl);
  _whileEnd.push(++_lbl);

  _symtab.push(); // new context

  _pf.ALIGN();
  _pf.LABEL(mklbl(_whileCond.top()));
  node->condition()->accept(this, lvl + 2);
  _pf.JZ(mklbl(_whileEnd.top()));

  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(_whileCond.top()));
  _pf.ALIGN();
  _pf.LABEL(mklbl(_whileEnd.top()));

  _symtab.pop();
  
  _whileEnd.pop();
  _whileCond.pop();
}

//--------------------------------------------------------------------------//
//                           CONDITIONAL INSTRUCTIONS                       //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_if_node(l22::if_node *const node, int lvl) {
  std::cout << "IF" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  std::string lbl = mklbl(++_lbl);
  node->condition()->accept(this, lvl);
  _pf.JZ(lbl);
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(lbl);
}

void l22::postfix_writer::do_if_else_node(l22::if_else_node *const node, int lvl) {
  std::cout << "IF ELSE" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  std::string lbl1 = mklbl(++_lbl);
  std::string lbl2 = mklbl(++_lbl);
  node->condition()->accept(this, lvl);
  _pf.JZ(lbl1);
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(lbl2);
  _pf.LABEL(lbl1);
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(lbl2);
}

//--------------------------------------------------------------------------//
//                             FUNCTIONS                                    //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_function_call_node(l22::function_call_node * const node, int lvl) {
  std::cout << "FUNCTION CALL" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;

  // Note that at this point we have made sure that the funcion call node is
  std::vector<std::shared_ptr<cdk::basic_type>> inputTypes;

  std::cout << "simbolos de funcao atuais: " << _funsymbols_string() << std::endl;
  if (node->identifier()) {   // non recursive case: formal types are encolsed in identifier type!
    inputTypes = cdk::functional_type::cast(node->identifier()->type())->input()->components();
  } else {                     // recursive case: must fetch formal types from current function symbol
    auto currFun = _fun_symbols.back();
    inputTypes = currFun->input_types();
  }

  // Remeber that at this point we have made sure that actuals and formals are compatible
  size_t argsSize = 0;
  if (node->arguments()) {
    for (int ix = node->arguments()->size() - 1; ix >= 0; --ix) {
      auto arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ix));
      arg->accept(this, lvl + 2);
      if (arg->is_typed(cdk::TYPE_INT) && inputTypes.at(ix)->name() == cdk::TYPE_DOUBLE) {
        _pf.I2D();
        argsSize += 4;
      }
      argsSize += arg->type()->size();
    }
  }

  if (node->identifier()) {  // non recursive case: need to get address value of the pointer and jump to it
    node->identifier()->accept(this, lvl + 2);
    _pf.BRANCH();
  } else {  // recursive case: just call last pushed function label
    _pf.CALL(_return_labels.back());
  }

  if (argsSize != 0) {
    _pf.TRASH(argsSize);
  }
  
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else {
    _pf.LDFVAL32();
  }
}

void l22::postfix_writer::do_function_definition_node(l22::function_definition_node * const node, int lvl) {
  std::cout << "FUNCTION DEFINITION" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  auto symbol = new_symbol();
  std::cout << "WTF: " << (symbol == nullptr) << std::endl;

  std::cout << "simbolos de funcao atuais antes: " << _funsymbols_string() << std::endl;
  if (symbol) {
    std::cout << "Não imprime!" << std::endl;
    _fun_symbols.push_back(symbol);
    reset_new_symbol();
  }
  std::cout << "simbolos de funcao atuais depois: " << _funsymbols_string() << std::endl;
  
  _offset = 8;
  _symtab.push();
  if(node->arguments()) {
    _inFunctionArgs = true;
    for (size_t ix = 0; ix < node-> arguments()->size(); ix++){
      cdk::basic_node *argument = node->arguments()->node(ix);
      if (!argument ) break;
      argument->accept(this,0);
    }
    _inFunctionArgs = false;
  }

  _pf.TEXT();
  _pf.ALIGN();
  std::string lbl = mklbl(++_lbl);
  _return_labels.push_back(lbl);
  _pf.LABEL(lbl);
  frame_size_calculator lsc(_compiler, _symtab, symbol);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize());

  // TODO: Check this case
  _inFunctionBody = true;
  if (node->outputType()->size() == 0) {
    _offset = -lsc.retsize();
  } else {
    _offset = -node->outputType()->size();
  }

  if (node->block()) {
    node->block()->accept(this, lvl + 4);
  }
  _inFunctionBody = false;

  _symtab.pop();
  _pf.LEAVE();
  _pf.RET();

  _return_labels.pop_back();
  _fun_symbols.pop_back();
  std::cout << "depois da fundef: " << _funsymbols_string() << std::endl;

  // Since a function definition is an expression, the last line must be its value (i.e., the address of its code)
  if (_inFunctionBody) {
    // local variable initializer
    _pf.TEXT();
    _pf.ADDR(lbl);
  } else {
    // global variable initializer
    _pf.DATA();
    _pf.SADDR(lbl);
  }

  _fun_label = lbl;
}

void l22::postfix_writer::do_return_node(l22::return_node * const node, int lvl) {
  std::cout << "RETURN" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  std::cout << "num return: " << _funsymbols_string() << std::endl;
  auto currFun = _fun_symbols.back();
  std::shared_ptr<cdk::basic_type> outputType = cdk::functional_type::cast(currFun->type())->output(0);
  if (currFun->name() == "_main") { // Different case
    node->retval()->accept(this, lvl);
    _pf.STFVAL32();
  }
  else if (outputType->name() != cdk::TYPE_VOID) {
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
  _pf.LEAVE();
  _pf.RET();
}

//--------------------------------------------------------------------------//
//                           BLOCK FLUX INSTRUCTIONS                        //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_stop_node(l22::stop_node * const node, int lvl) {
  if (_whileCond.size() != 0) {
    _pf.JMP(mklbl(_whileEnd.top()));
  } else {
    std::cerr << "STOP INSTRUCTION OUTSIDE CYCLE" << std::endl;
  }
}

void l22::postfix_writer::do_again_node(l22::again_node * const node, int lvl) {
  if (_whileCond.size() != 0) {
    _pf.JMP(mklbl(_whileCond.top()));
  } else {
    std::cerr << "AGAIN INSTRUCTION OUTSIDE CYCLE" << std::endl;
  }
}

//--------------------------------------------------------------------------//
//                           MEMORY RELATED                                 //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_index_node(l22::index_node * const node, int lvl) {
  std::cout << "INDEX" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);
  _pf.INT(node->type()->size());
  _pf.MUL();
  _pf.ADD();
}

void l22::postfix_writer::do_stack_alloc_node(l22::stack_alloc_node * const node, int lvl) {
  std::cout << "STACK ALLOC" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  auto referenced = cdk::reference_type::cast(node->type())->referenced();
  node->argument()->accept(this, lvl);
  _pf.INT(referenced->size());
  _pf.MUL();
  _pf.ALLOC();
  _pf.SP();
}

void l22::postfix_writer::do_address_of_node(l22::address_of_node * const node, int lvl) {
  std::cout << "ADDRESS OF" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl + 2);
}

void l22::postfix_writer::do_sizeof_node(l22::sizeof_node * const node, int lvl) {
  std::cout << "SIZE OF" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody) {
    _pf.INT(node->expression()->type()->size());
  } else {
    _pf.SINT(node->expression()->type()->size());
  }
}

//--------------------------------------------------------------------------//
//                          DECLARATION                                     //
//--------------------------------------------------------------------------//

void l22::postfix_writer::do_declaration_node(l22::declaration_node * const node, int lvl) {
  std::cout << "DECLARATION" << std::endl;
  ASSERT_SAFE_EXPRESSIONS;
  auto id = node->identifier();
  std::cout << "(DEBUG) _offset: " << _offset << std::endl;
  int offset = 0, typesize = node->type()->size();
  if (_inFunctionBody) {
    _offset -= typesize;
    offset = _offset;
  } else if (_inFunctionArgs) {
    offset = _offset;
    _offset += typesize;
  } else {
    offset = 0;
  }
  std::shared_ptr<l22::symbol> symbol = new_symbol();
  if (symbol) {
    symbol->set_offset(offset);
    reset_new_symbol();
  }

  if (_inFunctionBody) {
    if (node->initializer()) { // var x = a
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER)) {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      } else if(node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT)) {
          _pf.I2D();
        }
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else {
        std::cerr << "UNKNOWN DECLARATION NODE TYPE" << std::endl;
        return;
      }
    }
  } else {
    if (!node->initializer()) { // int x
      _pf.BSS();
      _pf.ALIGN();
      _pf.LABEL(id);
      _pf.SALLOC(typesize);
    } else { // e.g int x = 4 
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER)) {
        _pf.DATA();
        _pf.ALIGN();
        _pf.LABEL(id);
        if (node->is_typed(cdk::TYPE_INT)) {
          node->initializer()->accept(this, lvl);
        } else if (node->is_typed(cdk::TYPE_POINTER)) {
          node->initializer()->accept(this, lvl);
        } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
          if (node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
            node->initializer()->accept(this, lvl);
          }
          else if (node->initializer()->is_typed(cdk::TYPE_INT)) {
            cdk::integer_node *dclini = dynamic_cast<cdk::integer_node*>(node->initializer());
            cdk::double_node ddi(dclini->lineno(), dclini->value());
            ddi.accept(this, lvl);
          } else {
            std::cerr << "BAD DECLARATION FOR REAL VALUE" << std::endl;
          }
        }
      } else if (node->is_typed(cdk::TYPE_STRING)) {
          _pf.DATA();
          _pf.ALIGN();
          _pf.LABEL(id);
          node->initializer()->accept(this, lvl);
      } else if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {
        std::cout << "DEBUG 0.5" << std::endl;
        // we must push current symbol since it pertains to a function
        std::cout << "antes da declaracao: " << _funsymbols_string() << std::endl;
        _fun_symbols.push_back(symbol);
        std::cout << "depois da declaracao: " << _funsymbols_string() << std::endl;
        reset_new_symbol();
        node->initializer()->accept(this, lvl);
        _pf.DATA();
        _pf.ALIGN();
        _pf.LABEL(id);
        std::string label = _fun_label;
        _fun_label.clear();
        _pf.SADDR(label);
      } else {
        std::cerr << "UNEXPECTED INITIALIZER IN DECLARATION" << std::endl;
      }
    }
  }

}
