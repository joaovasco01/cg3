#ifndef __L22_TARGET_FRAME_SIZE_CALCULATOR_H__
#define __L22_TARGET_FRAME_SIZE_CALCULATOR_H__

#include "targets/basic_ast_visitor.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stack>
#include <cdk/symbol_table.h>
#include "targets/symbol.h"

namespace l22
{

    class frame_size_calculator : public basic_ast_visitor
    {
        cdk::symbol_table<l22::symbol> &_symtab;

        size_t _localsize;

    public:
        frame_size_calculator(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<l22::symbol> &symtab) : basic_ast_visitor(compiler), _symtab(symtab), _localsize(0)
        {
        }

    public:
        ~frame_size_calculator();

    public:
        size_t localsize() const
        {
            return _localsize;
        }

    public:
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"
#undef __IN_VISITOR_HEADER__
    };

} // l22

#endif
