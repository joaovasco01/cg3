#ifndef __L22_AST_PRINT_NODE_H__
#define __L22_AST_PRINT_NODE_H__

#include <cdk/ast/sequence_node.h>
#include "targets/basic_ast_visitor.h"

namespace l22
{
    /**
     * Class for describing print nodes.
     */

    class print_node : public cdk::basic_node
    {
        cdk::sequence_node *_argument;
        bool _newline;

    public:
        inline print_node(int lineno, cdk::sequence_node *argument, bool newline) : cdk::basic_node(lineno), _argument(argument), _newline(newline) {}

    public:
        inline cdk::sequence_node *argument() { return _argument; }

        inline bool newline() { return _newline; }

        void accept(basic_ast_visitor *sp, int level) { sp->do_print_node(this, level); }
    };

} // l22

#endif
