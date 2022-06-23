#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#include <l22_parser.tab.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

static std::string type_name(std::shared_ptr<cdk::basic_type> typed_node) {
  std::shared_ptr<cdk::functional_type> aux = cdk::functional_type::cast(typed_node);
  if (aux == nullptr) {
    return cdk::to_string(typed_node);
  } else {
    std::ostringstream strlit;
    strlit << "function with output " << cdk::to_string(aux->output(0))<< " and inputs " << "(";
    for (size_t i = 0; i < aux->input_length(); i++) {
      strlit << type_name(aux->input(i)) << ",";
    }
    if (aux->input_length() > 0) strlit.seekp(-1, strlit.cur);
    strlit << ")";
    return strlit.str();
  }
}


// NOTE: Check covariance regarding [void]
static bool compatible_pointed_types(std::shared_ptr<cdk::basic_type> ltype, std::shared_ptr<cdk::basic_type> rtype) {
  auto lt = ltype;
  auto rt = rtype;
  while (lt->name() == cdk::TYPE_POINTER && rt->name() == cdk::TYPE_POINTER && rt != nullptr) {
    lt = cdk::reference_type::cast(lt)->referenced();
    rt= cdk::reference_type::cast(rt)->referenced();
  }
  //bool compatible = (ft == rt) && (rtype == nullptr || (rtype != nullptr && ftype->name() == rtype->name()));

  return rtype == nullptr || ltype->name() == rtype->name();
}

// NOTE: abstract stuff in here
static bool compatible_function_types(std::shared_ptr<cdk::functional_type> ltype, std::shared_ptr<cdk::functional_type> rtype) {
  if (ltype->output(0)->name() == cdk::TYPE_POINTER) {
    // NOTE: this is a little weird, but we do know that nullptr does not exist in an output type of a funtype
    if (!(rtype->output(0)->name() == cdk::TYPE_POINTER && compatible_pointed_types(ltype->output(0), rtype->output(0)))) {
      return false;
    }
  } else if (ltype->output(0)->name() == cdk::TYPE_FUNCTIONAL) {
     if (!(rtype->output(0)->name() == cdk::TYPE_FUNCTIONAL && 
          compatible_function_types(cdk::functional_type::cast(ltype->output(0)), cdk::functional_type::cast(rtype->output(0))))) {
      return false;
    }
    // NOTE: check for each primitive type?
  } else if ((ltype->output(0)->name() != rtype->output(0)->name())) {
    return false;
  }

  if (ltype->input()->size() != rtype->input()->size()) {
    return false;
  }
  for (size_t tx = 0; tx < ltype->input_length(); tx++) {
    if (ltype->input(tx)->name() == cdk::TYPE_POINTER) {
      if (!(rtype->input(tx)->name() == cdk::TYPE_POINTER && compatible_pointed_types(ltype->input(tx), rtype->input(tx)))) {
        return false;
      }
    } else if (ltype->input(tx)->name() == cdk::TYPE_FUNCTIONAL) {
      if (!(rtype->input(tx)->name() == cdk::TYPE_FUNCTIONAL && 
          compatible_function_types(cdk::functional_type::cast(ltype->input(tx)), cdk::functional_type::cast(rtype->input(tx))))) {
        return false;
      }
    } else if ((ltype->input(tx)->name() != rtype->input(tx)->name())) {
      return false;
    }
  }

  return true;
}

//--------------------------------------------------------------------------//
//                                CDK                                       //
//--------------------------------------------------------------------------//

void l22::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

void l22::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void l22::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                                LITERALS                                  //
//--------------------------------------------------------------------------//

void l22::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void l22::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

void l22::type_checker::do_nullptr_node(l22::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, nullptr));
}

//--------------------------------------------------------------------------//
//                           UNARY ARITHMETIC EXPRESSIONS                   //
//--------------------------------------------------------------------------//

void l22::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!(node->argument()->is_typed(cdk::TYPE_INT) || node->argument()->is_typed(cdk::TYPE_DOUBLE))) {
    throw std::string("wrong type in argument of negation expression");
  }
  node->type(node->argument()->type());
}

void l22::type_checker::do_identity_node(l22::identity_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!(node->argument()->is_typed(cdk::TYPE_INT) || node->argument()->is_typed(cdk::TYPE_DOUBLE))) {
    throw std::string("wrong type in argument of identity expression");
  }
  node->type(node->argument()->type());
}

void l22::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("wrong type in argument of not expression");
  }
  node->type(node->argument()->type());
}

//--------------------------------------------------------------------------//
//                           BINARY ARITHMETIC EXPRESSIONS                  //
//--------------------------------------------------------------------------//

// NOTE: make different function for difference of pointers??
void l22::type_checker::do_PIDExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER)) { // NOTE: is this allowed ?
    node->type(node->right()->type());
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) { // NOTE: see input expression
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else {
    throw std::string("wrong types in binary expression");
  }
}

void l22::type_checker::do_IDExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else {
    throw std::string("wrong types in binary expression");
  }
}

void l22::type_checker::do_IntOnlyExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary operator (left)");
  }

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary operator (right)");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  do_PIDExpression(node, lvl);
}
void l22::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  do_PIDExpression(node, lvl);
}
void l22::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  do_IDExpression(node, lvl);
}
void l22::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  do_IDExpression(node, lvl);
}
void l22::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  do_IntOnlyExpression(node, lvl);
}

//--------------------------------------------------------------------------//
//                           BINARY LOGICAL EXPRESSIONS                     //
//--------------------------------------------------------------------------//

// NOTE: does this allow doubles? 
void l22::type_checker::do_ScalarLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary logical expression (left)");
  }

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary logical expression (right)");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_BooleanLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary expression");
  }

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary expression");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_GeneralLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  if (node->left()->type() != node->right()->type()) {
    throw std::string("same type expected on both sides of equality operator");
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  do_BooleanLogicalExpression(node, lvl);
}
void l22::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  do_BooleanLogicalExpression(node, lvl);
}
void l22::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  do_GeneralLogicalExpression(node, lvl);
}
void l22::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  do_GeneralLogicalExpression(node, lvl);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<l22::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void l22::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

// NOTE: check covariance regarding [void] and functions
void l22::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 4);
  node->rvalue()->accept(this, lvl + 4);

  if (node->lvalue()->is_typed(cdk::TYPE_INT)) {

    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {       
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else {
      throw std::string("wrong assignment to integer");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {

    if (node->rvalue()->is_typed(cdk::TYPE_DOUBLE) || node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {            // NOTE: is this correct??
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      node->rvalue()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    } else {
      throw std::string("wrong assignment to real");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_STRING)) {

    if (node->rvalue()->is_typed(cdk::TYPE_STRING)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {            // NOTE: is this correct??
      node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
      node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
    } else {
      throw std::string("wrong assignment to string");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {

    if (node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
      if (!(compatible_pointed_types(node->lvalue()->type(), node->rvalue()->type()))) {
        throw std::string("wrong assignment to pointer.");
      }
      node->type(node->rvalue()->type());
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {              // NOTE: is this correct??
      node->type(cdk::primitive_type::create(4, cdk::TYPE_ERROR));
      node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_ERROR));
    } else {
      throw std::string("wrong assignment to pointer");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_FUNCTIONAL)) {  

      if (node->rvalue()->is_typed(cdk::TYPE_FUNCTIONAL)) {
        if (!(compatible_function_types(cdk::functional_type::cast(node->lvalue()->type()), 
                                    cdk::functional_type::cast(node->rvalue()->type()))
          || (node->rvalue()->is_typed(cdk::TYPE_POINTER) && 
              cdk::reference_type::cast(node->rvalue()->type())->referenced() == nullptr))) {
              
          throw std::string("wrong type for initializer (function expected).");
        }
      node->type(node->rvalue()->type());
   
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {              // NOTE: is this correct??
      node->type(cdk::primitive_type::create(4, cdk::TYPE_ERROR));
      node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_ERROR));
    } else {
      throw std::string("wrong assignment to function");
    }

  } else {
    throw std::string("wrong types in assignment");
  }

}

//---------------------------------------------------------------------------

void l22::type_checker::do_program_node(l22::program_node *const node, int lvl) {
  auto mainfun = l22::make_symbol(cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)), "_main", 0);
  auto cdkInt = cdk::primitive_type::create(4, cdk::TYPE_INT);
  std::vector<std::shared_ptr<cdk::basic_type>> input_types;
  auto mainat = l22::make_symbol(cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)), "@", 0);
  if (_symtab.find_local(mainat->name())) {
    _symtab.replace(mainat->name(), mainat);
  } else {
    if (!_symtab.insert(mainat->name(), mainat)) {
      std::cerr << "ERROR INSERTING MAIN @" << std::endl;
      return;
    }
  }
  _parent->set_new_symbol(mainat);
}

void l22::type_checker::do_block_node(l22::block_node * const node, int lvl) {
}

void l22::type_checker::do_evaluation_node(l22::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void l22::type_checker::do_write_node(l22::write_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);
}

void l22::type_checker::do_input_node(l22::input_node *const node, int lvl) {
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//--------------------------------------------------------------------------//
//                           CYCLE INTRUCTION                               //
//--------------------------------------------------------------------------//

// NOTE: abstract all functions below??
void l22::type_checker::do_while_node(l22::while_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  if (!node->condition()->is_typed(cdk::TYPE_INT)) throw std::string("expected integer condition");
}

//--------------------------------------------------------------------------//
//                           CONDITIONAL INSTRUCTIONS                       //
//--------------------------------------------------------------------------//

void l22::type_checker::do_if_node(l22::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  if (!node->condition()->is_typed(cdk::TYPE_INT)) throw std::string("expected integer condition");
}

void l22::type_checker::do_if_else_node(l22::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  if (!node->condition()->is_typed(cdk::TYPE_INT)) throw std::string("expected integer condition");
}

//--------------------------------------------------------------------------//
//                             FUNCTIONS                                    //
//--------------------------------------------------------------------------//

void l22::type_checker::do_function_call_node(l22::function_call_node * const node, int lvl) {
  ASSERT_UNSPEC;
  std::vector<std::shared_ptr<cdk::basic_type>> input_types;
  std::shared_ptr<cdk::basic_type> output_type;
  
  if (!node->identifier()) {            // recursive call 
    auto symbol = _symtab.find("@", 1);
    if (symbol == nullptr) {
      throw std::string("recursive call outside function");
    }
    input_types = cdk::functional_type::cast(symbol->type())->input()->components();
    output_type = cdk::functional_type::cast(symbol->type())->output(0);
  } else {                              // non recursive call: just check functional type
    node->identifier()->accept(this, lvl + 2);
    if (!(node->identifier()->type()->name() == cdk::TYPE_FUNCTIONAL)) {
      throw std::string("expected function pointer on function call");
    }
    input_types = cdk::functional_type::cast(node->identifier()->type())->input()->components();
    output_type = cdk::functional_type::cast(node->identifier()->type())->output(0);
  }
  
  node->type(output_type);   

  if (node->arguments()->size() == input_types.size()) {
    node->arguments()->accept(this, lvl + 4);
    for (size_t ax = 0; ax < node->arguments()->size(); ax++) {
      if (node->argument(ax)->type()->name() == input_types[ax]->name()) continue;
      if (input_types[ax]->name() == cdk::TYPE_DOUBLE && node->argument(ax)->is_typed(cdk::TYPE_INT)) continue;
      throw std::string("type mismatch for argument " + std::to_string(ax + 1) + ".");
    }
  } else {
    throw std::string(
        "number of arguments in call (" + std::to_string(node->arguments()->size()) + ") must match declaration ("
            + std::to_string(input_types.size()) + ").");
  }
}


void l22::type_checker::do_function_definition_node(l22::function_definition_node * const node, int lvl) {
  ASSERT_UNSPEC;
  std::vector<std::shared_ptr<cdk::basic_type>> input_types;
  for (size_t ax = 0; ax < node->arguments()->size(); ax++) {
    input_types.push_back(node->argument(ax)->type());
  }
  node->type(cdk::functional_type::create(input_types, node->outputType()));

  auto function = l22::make_symbol(node->type(), "@", 0);
  //function->set_input_types(input_types);
  //function->set_output_type(node->outputType());

  // NOTE: _symtab.replace_local has a delete bug
  if (_symtab.find_local(function->name())) {
    _symtab.replace(function->name(), function);
  } else {
    if (!_symtab.insert(function->name(), function)) {
      std::cerr << "ERROR INSERTING FUNCTION @" << std::endl;
      return;
    }
  }
  _parent->set_new_symbol(function);
}

void l22::type_checker::do_return_node(l22::return_node * const node, int lvl) {
  
  auto symbol = _symtab.find("@", 1);
  if (symbol == nullptr) {
    symbol = _symtab.find("_main", 0);
    if (symbol == nullptr) {
      throw std::string("return statement outside program block");
    } else {

      if (!node->retval()) {
         throw std::string("wrong type for program return expression (integer expected).");
      }
      node->retval()->accept(this, lvl + 2);
      if (!node->retval()->is_typed(cdk::TYPE_INT)) {

        throw std::string("wrong type for program return expression (integer expected).");
      }

    }
  } else {

    if (node->retval()) {
      std::shared_ptr<cdk::functional_type> rettype = cdk::functional_type::cast(symbol->type());
      if (rettype->output() != nullptr && rettype->output(0)->name() == cdk::TYPE_VOID) {
        std::cout << "RETURNING FROM " << symbol->name() << std::endl;
        throw std::string("return value specified for void function.");
      }

      node->retval()->accept(this, lvl + 2);

      if (rettype->output() != nullptr && rettype->output(0)->name() == cdk::TYPE_INT) {
        if (!node->retval()->is_typed(cdk::TYPE_INT)) {
          throw std::string("wrong type for return expression (integer expected).");
        }
      } else if (rettype->output() != nullptr && rettype->output(0)->name() == cdk::TYPE_DOUBLE) {
        if (!node->retval()->is_typed(cdk::TYPE_INT) && !node->retval()->is_typed(cdk::TYPE_DOUBLE)) {
          throw std::string("wrong type for return expression (integer or double expected).");
        }
      } else if (rettype->output() != nullptr && rettype->output(0)->name() == cdk::TYPE_STRING) {
        if (!node->retval()->is_typed(cdk::TYPE_STRING)) {
          throw std::string("wrong type for return expression (string expected).");
        }
      } else if (rettype->output() != nullptr && rettype->output(0)->name() == cdk::TYPE_POINTER) {
        if (node->retval()->is_typed(cdk::TYPE_POINTER)) {
          if (!(compatible_pointed_types(rettype->output(0), node->retval()->type()))) {
            throw std::string("wrong type for return expression (pointer expected).");
          }
        }
      } else if (rettype->output() != nullptr && rettype->output(0)->name() == cdk::TYPE_FUNCTIONAL) {
        node->retval()->accept(this, lvl + 2);
        if (node->retval()->is_typed(cdk::TYPE_FUNCTIONAL)) {
          if (!(compatible_function_types(cdk::functional_type::cast(rettype->output(0)), 
                                      cdk::functional_type::cast(node->retval()->type()))
            || (node->retval()->is_typed(cdk::TYPE_POINTER) && 
                cdk::reference_type::cast(node->retval()->type())->referenced() == nullptr))) {
            throw std::string("wrong type for return expression (function expected).");
          }
        }
      } else {
        throw std::string("unknown type for return expression.");
      }
    }
  }
}

//--------------------------------------------------------------------------//
//                           BLOCK FLUX INSTRUCTIONS                        //
//--------------------------------------------------------------------------//

// NOTE: THESE ARE ALL EMPTY
void l22::type_checker::do_stop_node(l22::stop_node * const node, int lvl) {
  // EMPTY
}

void l22::type_checker::do_again_node(l22::again_node * const node, int lvl) {
  // EMPTY
}

//--------------------------------------------------------------------------//
//                           MEMORY RELATED                                 //
//--------------------------------------------------------------------------//

void l22::type_checker::do_index_node(l22::index_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->base()->accept(this, lvl + 2);
  // NOTE: Why check null base (see og & gr8)??
  std::shared_ptr<cdk::reference_type> basetype = cdk::reference_type::cast(node->base()->type());
  if (!node->base()->is_typed(cdk::TYPE_POINTER)) throw std::string("pointer expression expected in index left-value");

  node->index()->accept(this, lvl + 2);
  if (!node->index()->is_typed(cdk::TYPE_INT)) throw std::string("integer expression expected in left-value index");

  node->type(basetype->referenced());
}

void l22::type_checker::do_stack_alloc_node(l22::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in allocation expression");
  }
  // NOTE: What about other types?? How to decide at compile time?? Is it just because double is the biggest type?
  auto mytype = cdk::reference_type::create(4, cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  node->type(mytype);
}

void l22::type_checker::do_address_of_node(l22::address_of_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

void l22::type_checker::do_sizeof_node(l22::sizeof_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->expression()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//--------------------------------------------------------------------------//
//                          DECLARATION                                     //
//--------------------------------------------------------------------------//

// NOTE: check type covariance regarding functions and nullptr and [void]
void l22::type_checker::do_declaration_node(l22::declaration_node * const node, int lvl) {
  //_symtab.print_table();
  if (node->initializer() != nullptr) {
    node->initializer()->accept(this, lvl + 2);
    if (node->type()) {
      if (node->is_typed(cdk::TYPE_INT)) {
        if (!node->initializer()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type for initializer (integer expected).");
      } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (!node->initializer()->is_typed(cdk::TYPE_INT) && !node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
          throw std::string("wrong type for initializer (integer or double expected).");
        }
      } else if (node->is_typed(cdk::TYPE_STRING)) {
        if (!node->initializer()->is_typed(cdk::TYPE_STRING)) {
          throw std::string("wrong type for initializer (string expected).");
        }
      } else if (node->is_typed(cdk::TYPE_POINTER)) {
        if (!(node->initializer()->is_typed(cdk::TYPE_POINTER) && 
          compatible_pointed_types(node->type(), node->initializer()->type()))) {
          throw std::string("wrong type for initializer (pointer expected).");
        }
      } else if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {  // f = function or f = nullptr
        if (!((node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL) && 
            compatible_function_types(cdk::functional_type::cast(node->type()), 
                                      cdk::functional_type::cast(node->initializer()->type())))
            || ((node->initializer()->is_typed(cdk::TYPE_POINTER) && 
                cdk::reference_type::cast(node->initializer()->type())->referenced() == nullptr)))) {
          throw std::string("wrong type for initializer (function expected).");
        }
      } else {
        throw std::string("unknown type for initializer.");
      }

    } else {
      node->type(node->initializer()->type());
    }

  }

  std::string id;
  if (id == "_main") {
    id = "._main";
  } else {
    id = node->identifier();
  }

  auto symbol = l22::make_symbol(node->type(), id, (bool)node->initializer());
  std::shared_ptr<l22::symbol> previous = _symtab.find_local(symbol->name());
  //_symtab.print_table();

  if (previous) { // Function redeclaration
    if (previous->type()->name() == cdk::TYPE_FUNCTIONAL && 
        symbol->type()->name() == cdk::TYPE_FUNCTIONAL && 
        compatible_function_types(cdk::functional_type::cast(previous->type()), cdk::functional_type::cast(symbol->type()))) {
        _symtab.replace(symbol->name(), symbol);
        //if (previous->is_decl()) {  
        //   std::cout << "Redeclaração de função!" << std::endl;
        //  symbol->set_decl(true);
        //  symbol->set_redecl(true); 
        //}
       
    } else if (previous->type()->name() == cdk::TYPE_POINTER && 
        symbol->type()->name() == cdk::TYPE_POINTER && 
        compatible_pointed_types(previous->type(), symbol->type())) {
        _symtab.replace(symbol->name(), symbol);
        //if (previous->is_decl()) {  
        //   std::cout << "Redeclaração de ponteiro!" << std::endl;
        //  symbol->set_decl(true);
        //  symbol->set_redecl(true); 
        //}
       
    } else if (previous->type()->name() == symbol->type()->name()) {
        _symtab.replace(symbol->name(), symbol);
         std::cout << "Redeclaração de primitivo!" << std::endl;
        //if (previous->is_decl()) {  
        //  std::cout << "Redeclaração de primitivo!" << std::endl;
        //  symbol->set_decl(true);
        //  symbol->set_redecl(true); 
        //}
      
    } else {
      throw std::string(id + " was redefined.");
    }
  } else {
    _symtab.insert(id, symbol);
  } 

  _parent->set_new_symbol(symbol);
  //std::cout << "After seeing new symbol in type checker: " << std::endl;
  //_symtab.print_table();

  if (node->qualifier() == tUSE) {
    symbol->set_extern(true);
  } else if (node->qualifier() == tFOREIGN) {
    symbol->set_foreign(true);
  }

  // _symtab.print_table();
}




