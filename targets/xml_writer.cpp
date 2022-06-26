#include <string>
#include <string>
#include "targets/xml_writer.h"
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <sstream>

#include "l22_parser.tab.h"


static std::string qualifier_name(int qualifier) {
  if (qualifier == tPUBLIC) return "public";
  if (qualifier == tPRIVATE)
    return "private";
  if (qualifier == tUSE)
    return "use";
  if (qualifier == tFOREIGN)
    return "foreign";
  else
    return "unknown qualifier";
}

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

//---------------------------------------------------------------------------

void l22::xml_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}
void l22::xml_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void l22::xml_writer::do_double_node(cdk::double_node * const node, int lvl) {
  process_literal(node, lvl);
}
void l22::xml_writer::do_not_node(cdk::not_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}
void l22::xml_writer::do_and_node(cdk::and_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_or_node(cdk::or_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<sequence_node size='" << node->size() << "'>" << std::endl;
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  process_literal(node, lvl);
}

void l22::xml_writer::do_string_node(cdk::string_node * const node, int lvl) {
  process_literal(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_unary_operation(cdk::unary_operation_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_binary_operation(cdk::binary_operation_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_add_node(cdk::add_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_div_node(cdk::div_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_le_node(cdk::le_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void l22::xml_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  os() << std::string(lvl, ' ') << "<" << node->label() << ">" << node->name() << "</" << node->label() << ">" << std::endl;
}

void l22::xml_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);
  node->lvalue()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);

  node->lvalue()->accept(this, lvl + 4);
  reset_new_symbol();

  node->rvalue()->accept(this, lvl + 4);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_program_node(l22::program_node * const node, int lvl) {
  openTag(node, lvl);
  node->statements()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_evaluation_node(l22::evaluation_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

// void l22::xml_writer::do_print_node(l22::print_node * const node, int lvl) {
//   /* ASSERT_SAFE_EXPRESSIONS */
//   openTag(node, lvl);
//   node->argument()->accept(this, lvl + 2);
//   closeTag(node, lvl);
// }

//---------------------------------------------------------------------------

// void l22::xml_writer::do_read_node(l22::read_node * const node, int lvl) {
//   /* ASSERT_SAFE_EXPRESSIONS */
//   openTag(node, lvl);
//   node->argument()->accept(this, lvl + 2);
//   closeTag(node, lvl);
// }

//---------------------------------------------------------------------------

void l22::xml_writer::do_while_node(l22::while_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("block", lvl + 2);
  node->block()->accept(this, lvl + 4);
  closeTag("block", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_if_node(l22::if_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("then", lvl + 2);
  node->block()->accept(this, lvl + 4);
  closeTag("then", lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_if_else_node(l22::if_else_node * const node, int lvl) {
  /* ASSERT_SAFE_EXPRESSIONS */
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("then", lvl + 2);
  node->thenblock()->accept(this, lvl + 4);
  closeTag("then", lvl + 2);
  openTag("else", lvl + 2);
  node->elseblock()->accept(this, lvl + 4);
  closeTag("else", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void l22::xml_writer::do_nullptr_node(l22::nullptr_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}

void l22::xml_writer::do_function_call_node(l22::function_call_node * const node, int lvl) {
  
  openTag(node, lvl);
  openTag("function pointer", lvl + 2);
  if (node->identifier()) node->identifier()->accept(this, lvl + 4);
  else { os() << std::string(lvl + 4, ' ') << "<recursive>" << std::endl <<  std::string(lvl + 4, ' ') << "</recursive>" << std::endl; }
  closeTag("function pointer", lvl + 2);

  openTag("arguments", lvl + 2);
  if (node->arguments()) node->arguments()->accept(this, lvl + 4);
  closeTag("arguments", lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_block_node(l22::block_node * const node, int lvl) {
  openTag(node, lvl);
  openTag("declarations", lvl + 2);
  if (node->declarations()) {
    node->declarations()->accept(this, lvl + 4);
  }
  closeTag("declarations", lvl + 2);
  openTag("instructions", lvl + 2);
  if (node->instructions()) {
    node->instructions()->accept(this, lvl + 4);
  }
  closeTag("instructions", lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_stop_node(l22::stop_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}

void l22::xml_writer::do_again_node(l22::again_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}

void l22::xml_writer::do_return_node(l22::return_node * const node, int lvl) {
  openTag(node, lvl);
  if (node->retval()) node->retval()->accept(this, lvl + 4);
  closeTag(node, lvl);
}

void l22::xml_writer::do_index_node(l22::index_node * const node, int lvl) {
  openTag(node, lvl);
  openTag("base", lvl + 2);
  node->base()->accept(this, lvl + 4);
  closeTag("base", lvl + 2);
  openTag("index", lvl + 2);
  node->index()->accept(this, lvl + 4);
  closeTag("index", lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_stack_alloc_node(l22::stack_alloc_node * const node, int lvl) {
  openTag(node, lvl);
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_address_of_node(l22::address_of_node * const node, int lvl) {
  openTag(node, lvl);
  node->lvalue()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_sizeof_node(l22::sizeof_node * const node, int lvl) {
  openTag(node, lvl);
  node->expression()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_declaration_node(l22::declaration_node * const node, int lvl) {

  /* provisory solution for 'var' types while the type checker is not finished */
  os() << std::string(lvl, ' ') << "<" << node->label() << " name='" << node->identifier() << "' qualifier='"
      << qualifier_name(node->qualifier()) << "' type='" << (node->type() ? type_name(node->type()) : "var") << "'>" << std::endl;

  if (node->initializer()) {
    openTag("initializer", lvl + 2);
    node->initializer()->accept(this, lvl + 4);
    closeTag("initializer", lvl + 2);
  }
  closeTag(node, lvl);
}

void l22::xml_writer::do_function_definition_node(l22::function_definition_node * const node, int lvl) {
  
  os() << std::string(lvl, ' ') << "<" << node->label() << " return_type='" << type_name(node->outputType()) << "'>" << std::endl;

  openTag("arguments", lvl + 2);
  if (node->arguments()) {
    node->arguments()->accept(this, lvl + 4);
  }
  closeTag("arguments", lvl + 2);
  node->block()->accept(this, lvl + 2);
  closeTag(node, lvl);

}

void l22::xml_writer::do_input_node(l22::input_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}

void l22::xml_writer::do_write_node(l22::write_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<" << node->label() << " newline='" << (node->newline() ? "true" : "false") << "'>" << std::endl;
  node->arguments()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void l22::xml_writer::do_identity_node(l22::identity_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

void l22::xml_writer::do_with_node(l22::with_node *const node, int lvl) {
  //
}