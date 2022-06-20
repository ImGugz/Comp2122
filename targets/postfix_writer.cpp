#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

//--------------------------------------------------------------------------//
//                                CDK                                       //
//--------------------------------------------------------------------------//

void l22::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  std::cout << "SEQUENCE" << std::endl;
  for (size_t ix = 0; ix < node->size(); ++ix) {
    node->node(i)->accept(this, lvl);
  }
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
  
}