#ifndef __L22_TARGETS_POSTFIX_WRITER_H__
#define __L22_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"
#include "targets/symbol.h"
#include <cdk/symbol_table.h>
#include <cdk/emitters/basic_postfix_emitter.h>
#include <cdk/types/functional_type.h>

#include <set>
#include <sstream>
#include <vector>
#include <stack>

namespace l22 {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<l22::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;

    bool _inFunctionBody;
    bool _inFunctionArgs;
    bool _returnSeen;
    std::set<std::string> _external_functions;
    std::vector<std::shared_ptr<l22::symbol>> _fun_symbols;
    std::vector<std::string> _return_labels;
    int _offset;
    std::string _fun_label;
    std::stack<int> _whileCond, _whileEnd;
    std::vector<std::set<std::string>> _symbols_to_declare;
    std::vector<std::string> _doubt_symbols;
    std::string _doubt_symbol;

    bool _first_declarations;
    //bool _possible_extern_call;
    std::string _extern_label;
    //std::shared_ptr<l22::symbol> _symbol_to_define;
    //std::vector<std::shared_ptr<cdk::basic_type>> _intended_ret_types;


  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<l22::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0),
        _inFunctionBody(false), _inFunctionArgs(false), _returnSeen(false),  _offset(0), _first_declarations(true) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }
  
  protected:
  void do_initializer(cdk::expression_node * const node, int lvl, std::shared_ptr<l22::symbol> const symbol);

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

    std::string _funsymbols_string() {
      std::ostringstream oss;
      oss << "<";
      for (size_t ix = 0; ix < _fun_symbols.size(); ix++) {
        oss << _fun_symbols[ix]->name() << ",";
      }
      oss << ">";
      return oss.str();
    }

    std::string _symbols_string() {
      std::ostringstream oss;
      oss << "<";
      for (std::string name : _symbols_to_declare.back()) {
        oss << name << ",";
      }
      oss << ">";
      return oss.str();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // l22

#endif
