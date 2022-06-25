#ifndef __L22_TARGETS_POSTFIX_WRITER_H__
#define __L22_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <cdk/emitters/basic_postfix_emitter.h>
#include <stack>
#include <set>

namespace l22
{

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer : public basic_ast_visitor
  {
    cdk::symbol_table<l22::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl, _flbl;
    bool _inFunctionBody = false;
    bool _inFunctionArgs = false;
    std::shared_ptr<l22::symbol> _function;
    std::stack<int> _whileIni, _whileStep, _whileEnd;
    std::set<std::string> _functions_to_declare;
    cdk::typename_type _lvalueType;
    int _offset = 0;

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<l22::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) : basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0), _flbl(0)
    {
    }

  public:
    ~postfix_writer()
    {
      os().flush();
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl)
    {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }
    inline std::string mklblf(int flbl)
    {
      std::ostringstream oss;
      if (flbl < 0)
        oss << ".F" << -flbl;
      else
        oss << "_F" << flbl;
      return oss.str();
    }

  public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h" // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end
  };

} // l22

#endif
