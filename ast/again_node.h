#ifndef __L22_AST_AGAIN_H__
#define __L22_AST_AGAIN_H__

#include <cdk/ast/basic_node.h>

namespace l22
{
    /**
     * Class for describing again-node.
     */

    class again_node : public cdk::basic_node
    {

    public:
        inline again_node(int lineno) : cdk::basic_node(lineno) {}

    public:
        void accept(basic_ast_visitor *sp, int level) { sp->do_again_node(this, level); }
    };

} // namespace l22

#endif