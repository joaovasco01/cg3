#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h" // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC                                                 \
  {                                                                   \
    if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) \
      return;                                                         \
  }

//---------------------------------------------------------------------------
void l22::type_checker::do_BooleanLogicalExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());

    if (readl != nullptr)
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unspecified type.");
  }
  else if (!node->left()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in binary operators.");

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readr = dynamic_cast<l22::read_node *>(node->right());

    if (readr != nullptr)
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unspecified type.");
  }
  else if (!node->right()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in binary operators.");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_ScalarLogicalExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());

    if (readl != nullptr)
      node->left()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unspecified type.");
  }
  else if (!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Wrong! Expected integer or double).");

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readr = dynamic_cast<l22::read_node *>(node->right());

    if (readr != nullptr)
      node->right()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unspecified type.");
  }
  else if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Wrong! Expected integer or double.");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_GeneralLogicalExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());

    if (readl != nullptr)
      node->left()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unknown node with unspecified type.");
  }

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readr = dynamic_cast<l22::read_node *>(node->right());

    if (readr != nullptr)
      node->left()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unspecified type.");
  }

  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER))
    typeOfPointer(cdk::reference_type::cast(node->left()->type()), cdk::reference_type::cast(node->right()->type()));
  else if (node->left()->type()->name() != node->right()->type()->name())
  {
    if (!((node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) ||
          (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))))
      throw std::string("Incompatible types.");
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

std::shared_ptr<cdk::basic_type> l22::type_checker::typeOfPointer(std::shared_ptr<cdk::reference_type> leftPtr, std::shared_ptr<cdk::reference_type> rightPtr)
{
  std::shared_ptr<cdk::basic_type> left, right;
  left = leftPtr;
  right = rightPtr;
  while (left->name() == cdk::TYPE_POINTER && right->name() == cdk::TYPE_POINTER)
  {
    left = cdk::reference_type::cast(left)->referenced();
    right = cdk::reference_type::cast(right)->referenced();
  }
  if (left->name() == cdk::TYPE_POINTER || right->name() == cdk::TYPE_POINTER)
    throw std::string("Wrong pointer type.");
  if (left->name() == cdk::TYPE_INT && right->name() == cdk::TYPE_INT)
    return cdk::primitive_type::create(4, cdk::TYPE_INT);
  else if (left->name() == cdk::TYPE_DOUBLE && right->name() == cdk::TYPE_DOUBLE)
    return cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);
  else if (left->name() == cdk::TYPE_STRING && right->name() == cdk::TYPE_STRING)
    return cdk::primitive_type::create(4, cdk::TYPE_STRING);
  else
    throw std::string("Wrong pointer type.");
}

void l22::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl)
{
  for (size_t i = 0; i < node->size(); i++)
  {
    node->node(i)->accept(this, lvl + 2);
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->node(i));
    if (expression != nullptr && expression->is_typed(cdk::TYPE_UNSPEC))
    {
      l22::read_node *read = dynamic_cast<l22::read_node *>(expression);

      if (read != nullptr)
        read->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      else
        throw std::string("Unknown node with unspecified type.");
    }
  }
}

//---------------------------------------------------------------------------

void l22::type_checker::do_nil_node(cdk::nil_node *const node, int lvl)
{
  // EMPTY
}
void l22::type_checker::do_data_node(cdk::data_node *const node, int lvl)
{
  // EMPTY
}
void l22::type_checker::do_double_node(cdk::double_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void l22::type_checker::do_not_node(cdk::not_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (node->argument()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *read = dynamic_cast<l22::read_node *>(node->argument());

    if (read != nullptr)
    {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong type in argument of unary expression (Integer expected).");
}

void l22::type_checker::do_and_node(cdk::and_node *const node, int lvl)
{
  do_BooleanLogicalExpression(node, lvl);
}
void l22::type_checker::do_or_node(cdk::or_node *const node, int lvl)
{
  do_BooleanLogicalExpression(node, lvl);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_integer_node(cdk::integer_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_string_node(cdk::string_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void l22::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl)
{
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in argument of unary expression");

  // in Simple, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_neg_node(cdk::neg_node *const node, int lvl)
{
  processUnaryExpression(node, lvl);
}

void l22::type_checker::do_IDExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());
    l22::read_node *readr = dynamic_cast<l22::read_node *>(node->right());

    if (readl != nullptr && readr != nullptr)
    {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if (node->left()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());

    if (readl != nullptr)
    {
      node->left()->type(node->right()->type());
      node->type(node->right()->type());
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if (node->right()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readr = dynamic_cast<l22::read_node *>(node->right());

    if (readr != nullptr)
    {
      node->right()->type(node->left()->type());
      node->type(node->left()->type());
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong types in binary expression.");
}

void l22::type_checker::do_IntOnlyExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());

    if (readl != nullptr)
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  node->right()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());

    if (readl != nullptr)
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else
    throw std::string("Integer expression expected in (left and right) binary operators.");
}

//---------------------------------------------------------------------------

void l22::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(node->left()->type());
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER))
    node->type(node->right()->type());
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER))
  {
    typeOfPointer(cdk::reference_type::cast(node->left()->type()), cdk::reference_type::cast(node->right()->type()));
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());
    l22::read_node *readr = dynamic_cast<l22::read_node *>(node->right());

    if (readl != nullptr)
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");

    if (readr != nullptr)
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else if (node->left()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readl = dynamic_cast<l22::read_node *>(node->left());

    if (readl != nullptr)
    {
      if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT))
        node->left()->type(node->right()->type());
      else
        throw std::string("Invalid expression in right argument of binary expression.");
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if (node->right()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *readr = dynamic_cast<l22::read_node *>(node->right());

    if (readr != nullptr)
    {
      if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->left()->is_typed(cdk::TYPE_INT))
        node->right()->type(node->left()->type());
      else
        throw std::string("Invalid expression in left argument of binary expression.");
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong types in binary expression.");
}

void l22::type_checker::do_add_node(cdk::add_node *const node, int lvl)
{
  processBinaryExpression(node, lvl);
}
void l22::type_checker::do_sub_node(cdk::sub_node *const node, int lvl)
{
  processBinaryExpression(node, lvl);
}
void l22::type_checker::do_mul_node(cdk::mul_node *const node, int lvl)
{
  do_IDExpression(node, lvl);
}
void l22::type_checker::do_div_node(cdk::div_node *const node, int lvl)
{
  do_IDExpression(node, lvl);
}
void l22::type_checker::do_mod_node(cdk::mod_node *const node, int lvl)
{
  do_IntOnlyExpression(node, lvl);
}
void l22::type_checker::do_lt_node(cdk::lt_node *const node, int lvl)
{
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_le_node(cdk::le_node *const node, int lvl)
{
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_ge_node(cdk::ge_node *const node, int lvl)
{
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_gt_node(cdk::gt_node *const node, int lvl)
{
  do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_ne_node(cdk::ne_node *const node, int lvl)
{
  do_GeneralLogicalExpression(node, lvl);
}
void l22::type_checker::do_eq_node(cdk::eq_node *const node, int lvl)
{
  do_GeneralLogicalExpression(node, lvl);
}
//---

void l22::type_checker::do_address_of_node(l22::address_of_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::type_checker::do_again_node(l22::again_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::type_checker::do_block_node(l22::block_node *const node, int lvl)
{
  if (node->declarations())
    node->declarations()->accept(this, lvl + 2);

  if (node->instructions())
    node->instructions()->accept(this, lvl + 2);
}
void l22::type_checker::do_function_call_node(l22::function_call_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::type_checker::do_function_definition_node(l22::function_definition_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::type_checker::do_index_node(l22::index_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  if (!node->base()->is_typed(cdk::TYPE_POINTER))
    throw std::string("Lvalue must be a pointer.");

  node->index()->accept(this, lvl + 2);
  if (node->index()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *read = dynamic_cast<l22::read_node *>(node->index());

    if (read != nullptr)
      node->index()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unspecified type.");
  }
  else if (!node->index()->is_typed(cdk::TYPE_INT))
    throw std::string("Not an integer expression - Index!.");

  node->type(cdk::reference_type::cast(node->base()->type())->referenced());
}
void l22::type_checker::do_nullptr_node(l22::nullptr_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, nullptr));
}
void l22::type_checker::do_return_node(l22::return_node *const node, int lvl)
{
  if (node->retval())
  {
    if (_function->type()->name() == cdk::TYPE_VOID)
      throw std::string("Void function cannot return values.");

    node->retval()->accept(this, lvl + 2);

    if (_function->type()->name() == cdk::TYPE_INT)
    {
      if (!node->retval()->is_typed(cdk::TYPE_INT))
        throw std::string("Wrong type for initializer (integer expected).");
    }
    else if (_function->type()->name() == cdk::TYPE_DOUBLE)
    {
      if (!node->retval()->is_typed(cdk::TYPE_INT) &&
          !node->retval()->is_typed(cdk::TYPE_DOUBLE))
        throw std::string("Wrong type for initializer (integer or double expected).");
    }
    else if (_function->type()->name() == cdk::TYPE_STRING)
    {
      if (!node->retval()->is_typed(cdk::TYPE_STRING))
        throw std::string("Wrong type for initializer (string expected).");
    }
    else if (_function->type()->name() == cdk::TYPE_POINTER)
    {
      /*typeOfPointer(cdk::reference_type::cast(node->retval()->type()),
                    cdk::reference_type::cast(_function->type()));*/
    }
    else
    {
      throw std::string("Unknown type for return expression.");
    }
  }
}
void l22::type_checker::do_sizeof_node(l22::sizeof_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->expression()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}
void l22::type_checker::do_stack_alloc_node(l22::stack_alloc_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_UNSPEC))
  {
    l22::read_node *read = dynamic_cast<l22::read_node *>(node->argument());

    if (read != nullptr)
      node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unspecified type.");
  }
  else if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer was expected - allocation expression.");

  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}
void l22::type_checker::do_stop_node(l22::stop_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::type_checker::do_variable_declaration_node(l22::variable_declaration_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::type_checker::do_identity_node(l22::identity_node *const node, int lvl)
{
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_variable_node(cdk::variable_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<l22::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr)
  {
    node->type(symbol->type());
  }
  else
  {
    throw id;
  }
}

void l22::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  try
  {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  }
  catch (const std::string &id)
  {
    throw "undeclared variable '" + id + "'";
  }
}

void l22::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl)
{
  ASSERT_UNSPEC;

  try
  {
    node->lvalue()->accept(this, lvl);
  }
  catch (const std::string &id)
  {
    auto symbol = std::make_shared<l22::symbol>(cdk::primitive_type::create(4, cdk::TYPE_INT), id, 0);
    _symtab.insert(id, symbol);
    _parent->set_new_symbol(symbol);   // advise parent that a symbol has been inserted
    node->lvalue()->accept(this, lvl); // DAVID: bah!
  }

  if (!node->lvalue()->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in left argument of assignment expression");

  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in right argument of assignment expression");

  // in Simple, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void l22::type_checker::do_program_node(l22::program_node *const node, int lvl)
{
  // EMPTY
}

void l22::type_checker::do_evaluation_node(l22::evaluation_node *const node, int lvl)
{
  node->argument()->accept(this, lvl + 2);
}

void l22::type_checker::do_print_node(l22::print_node *const node, int lvl)
{
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_read_node(l22::read_node *const node, int lvl)
{
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void l22::type_checker::do_while_node(l22::while_node *const node, int lvl)
{
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_if_node(l22::if_node *const node, int lvl)
{
  node->condition()->accept(this, lvl + 4);
}

void l22::type_checker::do_if_else_node(l22::if_else_node *const node, int lvl)
{
  node->condition()->accept(this, lvl + 4);
}
