#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h" // all_nodes.h is automatically generated
#include "targets/frame_size_calculator.h"
#include "l22_parser.tab.h"

//---------------------------------------------------------------------------

void l22::postfix_writer::do_nil_node(cdk::nil_node *const node, int lvl)
{
  // EMPTY
}
void l22::postfix_writer::do_data_node(cdk::data_node *const node, int lvl)
{
  // EMPTY
}
void l22::postfix_writer::do_double_node(cdk::double_node *const node, int lvl)
{
  if (_inFunctionBody)
  {
    _pf.DOUBLE(node->value()); // load number to the stack
  }
  else
  {
    _pf.SDOUBLE(node->value()); // double is on the DATA segment
  }
}
void l22::postfix_writer::do_not_node(cdk::not_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ();
}
void l22::postfix_writer::do_and_node(cdk::and_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}
void l22::postfix_writer::do_or_node(cdk::or_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}
void l22::postfix_writer::do_address_of_node(l22::address_of_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // since the argument is an lvalue, it is already an address
  node->argument()->accept(this, lvl + 2);
}

//--

void l22::postfix_writer::do_again_node(l22::again_node *const node, int lvl)
{
  if (_whileIni.size() != 0)
  {
    _pf.JMP(mklbl(_whileStep.top())); // jump to next cycle
  }
  else
    std::cerr << "Continue instruction can only be used inside a while cycle." << std::endl;
}
void l22::postfix_writer::do_block_node(l22::block_node *const node, int lvl)
{
  _symtab.push(); // for block-local vars
  if (node->declarations())
    node->declarations()->accept(this, lvl + 2);
  if (node->instructions())
  {
    node->instructions()->accept(this, lvl + 2);
  }
  _symtab.pop();
}

void l22::postfix_writer::do_function_call_node(l22::function_call_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::postfix_writer::do_function_definition_node(l22::function_definition_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::postfix_writer::do_index_node(l22::index_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  if (node->base())
  {
    node->base()->accept(this, lvl);
  }
  else
  {
    if (_function)
    {
      _pf.LOCV(-_function->type()->size());
    }
    else
    {
      std::cerr << "FATAL: " << node->lineno() << ": trying to use return value outside function" << std::endl;
    }
  }
  node->index()->accept(this, lvl);
  _pf.INT(3);
  _pf.SHTL();
  _pf.ADD(); // add pointer and index
}
void l22::postfix_writer::do_nullptr_node(l22::nullptr_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS; // a pointer is a 32-bit integer
  if (_inFunctionBody)
  {
    _pf.INT(0);
  }
  else
  {
    _pf.SINT(0);
  }
}
void l22::postfix_writer::do_return_node(l22::return_node *const node, int lvl)
{
  /* EMPTY */
}
void l22::postfix_writer::do_sizeof_node(l22::sizeof_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->expression()->type()->size());
}
void l22::postfix_writer::do_stack_alloc_node(l22::stack_alloc_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if (cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
  else
    _pf.INT(2);

  _pf.SHTL();
  _pf.ALLOC();
  _pf.SP();
}
void l22::postfix_writer::do_stop_node(l22::stop_node *const node, int lvl)
{
  if (_whileIni.size() != 0)
  {
    _pf.JMP(mklbl(_whileEnd.top())); // jump to for end
  }
  else
    std::cerr << "Break instruction can only be used inside a while cycle." << std::endl;
}
void l22::postfix_writer::do_variable_declaration_node(l22::variable_declaration_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  std::vector<std::string> *identifiers = new std::vector<std::string>();
  std::string id = node->identifier();
  std::string delimiter = ",";
  size_t pos = 0;
  std::string token;
  while ((pos = id.find(delimiter)) != std::string::npos)
  {
    token = id.substr(0, pos);
    identifiers->push_back(token);
    id.erase(0, pos + delimiter.length());
  }
  identifiers->push_back(id);

  int offset = 0;
  int size = node->type()->size();
  if (_inFunctionBody)
  {
    _offset -= size;
    offset = _offset;
  }
  else if (_inFunctionArgs)
  {
    offset = _offset;
    _offset += size;
  }
  else
    offset = 0;

  std::shared_ptr<l22::symbol> symbol = new_symbol();

  if (symbol)
  {
    symbol->set_offset(offset);
    reset_new_symbol();
  }

  if (node->initializer())
  {
    if (_inFunctionBody)
    {
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_DOUBLE))
      {
        if (node->initializer()->is_typed(cdk::TYPE_INT))
          _pf.I2D();

        _pf.LOCAL(offset);
        _pf.STDOUBLE();
      }
      else
      {
        _pf.LOCAL(offset);
        _pf.STINT();
      }
    }
    else if (!_inFunctionBody && !_inFunctionArgs)
    {
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_DOUBLE))
      {
        _pf.DATA();
        _pf.ALIGN();
        if (node->qualifier() == tPUBLIC)
        {
          _pf.GLOBAL(id, _pf.OBJ());
        }
        _pf.LABEL(id);
        if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER))
        {
          node->initializer()->accept(this, lvl);
        }
        else if (node->is_typed(cdk::TYPE_DOUBLE))
        {
          if (node->initializer()->is_typed(cdk::TYPE_DOUBLE))
            node->initializer()->accept(this, lvl);
          else if (node->initializer()->is_typed(cdk::TYPE_INT))
          {
            cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(node->initializer());
            cdk::double_node ddi(dclini->lineno(), dclini->value());
            ddi.accept(this, lvl);
          }
          else
          {
            std::cerr << node->lineno() << ": '" << id << "' wrong initializer for real variable.\n";
            exit(2);
          }
        }
      }
      else if (node->is_typed(cdk::TYPE_STRING))
      {
        _pf.DATA();
        _pf.ALIGN();
        _pf.LABEL(node->identifier());
        node->initializer()->accept(this, lvl);
      }
    }
    else
    {
      std::cerr << node->lineno() << ": '" << id << "' has an unexpected initializer.\n";
      exit(2);
    }
  }
  else
  {
    if (!_inFunctionBody && !_inFunctionArgs && (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_STRING)))
    {
      _pf.BSS();
      _pf.ALIGN();
      _pf.LABEL(id);
      _pf.SALLOC(size);
    }
  }
}
void l22::postfix_writer::do_identity_node(l22::identity_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_sequence_node(cdk::sequence_node *const node, int lvl)
{
  for (size_t i = 0; i < node->size(); i++)
  {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_integer_node(cdk::integer_node *const node, int lvl)
{
  if (_inFunctionBody)
  {
    _pf.INT(node->value()); // integer literal is on the stack: push an integer
  }
  else
  {
    _pf.SINT(node->value()); // integer literal is on the DATA segment
  }
}

void l22::postfix_writer::do_string_node(cdk::string_node *const node, int lvl)
{
  int lbl1;

  _pf.RODATA();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  _pf.SSTRING(node->value());

  _pf.ALIGN();
  if (_inFunctionBody)
  {
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1));
  }
  else
  {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_neg_node(cdk::neg_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DNEG();
  }
  else if (node->is_typed(cdk::TYPE_INT))
  {
    _pf.NEG();
  }
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_add_node(cdk::add_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT))
  {

    if (cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    {
      _pf.INT(3);
    }
    else
    {
      _pf.INT(2);
    }
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
  {
    if (cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    {
      _pf.INT(3);
    }
    else
    {
      _pf.INT(2);
    }
    _pf.SHTL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DADD();
  }
  else
  {
    _pf.ADD();
  }
}
void l22::postfix_writer::do_sub_node(cdk::sub_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT))
  {
    if (cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    {
      _pf.INT(3);
    }
    else
    {
      _pf.INT(2);
    }
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
  {
    if (cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    {
      _pf.INT(3);
    }
    else
    {
      _pf.INT(2);
    }
    _pf.SHTL();
  }
  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER))
  {
    int lbl1;

    _pf.SUB();
    _pf.INT(cdk::reference_type::cast(node->left()->type())->referenced()->size());
    _pf.DIV();
    _pf.DUP32();
    _pf.INT(0);
    _pf.LT();
    _pf.JZ(mklbl(lbl1 = ++_lbl));
    _pf.NEG();
    _pf.ALIGN();
    _pf.LABEL(mklbl(lbl1));
  }
  else
  {
    if (node->is_typed(cdk::TYPE_DOUBLE))
    {
      _pf.DSUB();
    }
    else
    {
      _pf.SUB();
    }
  }
}
void l22::postfix_writer::do_mul_node(cdk::mul_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DMUL();
  }
  else
  {
    _pf.MUL();
  }
}
void l22::postfix_writer::do_div_node(cdk::div_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DDIV();
  }
  else
  {
    _pf.DIV();
  }
}
void l22::postfix_writer::do_mod_node(cdk::mod_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void l22::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.LT();
}
void l22::postfix_writer::do_le_node(cdk::le_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.LE();
}
void l22::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.GE();
}
void l22::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.GT();
}
void l22::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.NE();
}
void l22::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.I2D();
  }
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.EQ();
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  const std::string &id = node->name();
  auto symbol = _symtab.find(id);
  if (symbol->global())
  {
    _pf.ADDR(symbol->name());
  }
  else
  {
    _pf.LOCAL(symbol->offset());
    // std::cerr << "LVAL " << symbol->name() << ":" << symbol->type()->size() << ":" << symbol->offset() << std::endl;
  }
}

void l22::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE)
  {
    _pf.LDDOUBLE();
  }
  else
  {
    // integers, pointers, and strings
    _pf.LDINT();
  }
}

void l22::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl); // determine the new value
  _pf.DUP32();
  if (new_symbol() == nullptr)
  {
    node->lvalue()->accept(this, lvl); // where to store the value
  }
  else
  {
    _pf.DATA();                      // variables are all global and live in DATA
    _pf.ALIGN();                     // make sure we are aligned
    _pf.LABEL(new_symbol()->name()); // name variable location
    reset_new_symbol();
    _pf.SINT(0);                       // initialize it to 0 (zero)
    _pf.TEXT();                        // return to the TEXT segment
    node->lvalue()->accept(this, lvl); // DAVID: bah!
  }
  _pf.STINT(); // store the value at address
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_program_node(l22::program_node *const node, int lvl)
{
  // Note that Simple doesn't have functions. Thus, it doesn't need
  // a function node. However, it must start in the main function.
  // The ProgramNode (representing the whole program) doubles as a
  // main function node.

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");
  frame_size_calculator lsc(_compiler, _symtab);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize());

  _inFunctionBody = true;
  node->statements()->accept(this, lvl);
  _inFunctionBody = false;

  // end the main function
  _pf.INT(0);
  _pf.STFVAL32();
  _pf.LEAVE();
  _pf.RET();

  // these are just a few library function imports
  _pf.EXTERN("readi");
  _pf.EXTERN("printi");
  _pf.EXTERN("prints");
  _pf.EXTERN("println");
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_evaluation_node(l22::evaluation_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if (node->argument()->is_typed(cdk::TYPE_INT) ||
      node->argument()->is_typed(cdk::TYPE_STRING) ||
      node->argument()->is_typed(cdk::TYPE_POINTER))
  {
    _pf.TRASH(4);
  }
  else if (node->argument()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.TRASH(8);
  }
  else if (!node->argument()->is_typed(cdk::TYPE_VOID) &&
           !node->argument()->is_typed(cdk::TYPE_STRUCT))
  {
    std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
  }
}

void l22::postfix_writer::do_print_node(l22::print_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  for (size_t i = 0; i < node->argument()->size(); i++)
  {
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->argument()->node(i));
    if (expression != nullptr)
    {
      if (expression->is_typed(cdk::TYPE_INT))
      {
        _pf.CALL("printi");
        _pf.TRASH(4);
      }
      else if (expression->is_typed(cdk::TYPE_STRING))
      {
        _pf.CALL("prints");
        _pf.TRASH(4);
      }
      else if (expression->is_typed(cdk::TYPE_DOUBLE))
      {
        _pf.CALL("printd");
        _pf.TRASH(8);
      }
    }
    else
    {
      std::cerr << "ERROR: Invalid type." << std::endl;
      exit(1);
    }
  }
  if (node->newline())
    _pf.CALL("println");
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_read_node(l22::read_node *const node, int lvl)
{
  // ASSERT_SAFE_EXPRESSIONS;
  if (_lvalueType == cdk::TYPE_DOUBLE)
  {
    _functions_to_declare.insert("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  }
  else if (_lvalueType == cdk::TYPE_INT)
  {
    _functions_to_declare.insert("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  }
  else
  {
    std::cerr << "FATAL: " << node->lineno() << ": cannot read type" << std::endl;
    return;
  }
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_while_node(l22::while_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl2 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2));
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_if_node(l22::if_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_if_else_node(l22::if_else_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}
