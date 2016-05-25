#include "pydusa.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <medusa/expression.hpp>
#include <medusa/expression_visitor.hpp>

namespace py = pybind11;

MEDUSA_NAMESPACE_USE

// get string representation of an identifier expression
char const* GetExprIdIdentifier(IdentifierExpression *e)
{
  return e->GetCpuInformation()->ConvertIdentifierToName(e->GetId());
}

// Type of operation expression: get enumeration name from value
OperationExpression::Type expr_op_get_name(OperationExpression::Type t) { return t; }

void PydusaExpression(py::module& rMod)
{

  // base expression class

  py::class_<Expression, Expression::SPType> pyExpr(rMod, "Expression");
  pyExpr
    .def("__str__", &Expression::ToString)
    .def("update_child", &Expression::UpdateChild)
    .def("compare", &Expression::Compare)
    .def("visit", &Expression::Visit)
    .def_property_readonly("bit_size", &Expression::GetBitSize)
    ;

  py::enum_<Expression::CompareType>(pyExpr, "Compare")
    .value("UNKNOWN", Expression::CmpUnknown)
    .value("DIFFERENT", Expression::CmpDifferent)
    .value("SAME_EXPRESSION", Expression::CmpSameExpression)
    .value("IDENTICAL", Expression::CmpIdentical)
    ;

  // constant expression class inherited from base expression class

  py::class_<BitVectorExpression, BitVectorExpression::SPType>(rMod, "BitVectorExpression", py::base<Expression>())
    .def("__str__", &BitVectorExpression::ToString)
    .def_property_readonly("value", &BitVectorExpression::GetInt)
    ;

  // identifier expression class inherited from base expression class

  py::class_<IdentifierExpression, IdentifierExpression::SPType>(rMod, "IdentifierExpression", py::base<Expression>())
    .def("__str__", &IdentifierExpression::ToString)
    .def_property_readonly("id", &GetExprIdIdentifier)
    ;

  // assignment expression class inherited from base expression class

  py::class_<AssignmentExpression, AssignmentExpression::SPType>(rMod, "AssignmentExpression", py::base<Expression>())
    .def("__str__", &AssignmentExpression::ToString)
    .def_property_readonly("dst", &AssignmentExpression::GetDestinationExpression)
    .def_property_readonly("src", &AssignmentExpression::GetSourceExpression)
    ;

  // memory expression class inherited from base expression class

  py::class_<MemoryExpression, MemoryExpression::SPType>(rMod, "MemoryExpression", py::base<Expression>())
    .def("__str__", &MemoryExpression::ToString)
    .def_property_readonly("expr", &MemoryExpression::GetAddressExpression)
    ;

  // operation expression class inherited from base expression class

  py::class_<OperationExpression, OperationExpression::SPType>(rMod, "OperationExpression", py::base<Expression>())
    .def("__str__", &OperationExpression::ToString)
    .def_property_readonly("op", &OperationExpression::GetOperation)
    ;

  // UnaryOperationExpression expression class inherited from operation expression class

  py::class_<UnaryOperationExpression, UnaryOperationExpression::SPType>(rMod, "UnaryOperationExpression", py::base<Expression>())
    .def("__str__", &UnaryOperationExpression::ToString)
    ;

  // BinaryOperationExpression expression class inherited from operation expression class

  py::class_<BinaryOperationExpression, BinaryOperationExpression::SPType>(rMod, "BinaryOperationExpression", py::base<Expression>())
    .def("__str__", &BinaryOperationExpression::ToString)
    ;

  // helpers used to create instance of a specific expression type

  rMod
    .def("expr_make_bv", (Expression::SPType(*)(u16, ap_int))&Expr::MakeBitVector)
    .def("expr_make_bool", &Expr::MakeBoolean)
    .def("expr_make_id", &Expr::MakeId)
    .def("expr_make_mem", &Expr::MakeMem)
    ;

  // exposing enumerations

  py::enum_<OperationExpression::Type>(rMod, "expr_op_type")
    .value("OP_UNK", OperationExpression::OpUnk)
    .value("OP_NOT", OperationExpression::OpNot)
    .value("OP_NEG", OperationExpression::OpNeg)
    .value("OP_SWAP", OperationExpression::OpSwap)
    .value("OP_BSF", OperationExpression::OpBsf)
    .value("OP_BSR", OperationExpression::OpBsr)
    .value("OP_AND", OperationExpression::OpAnd)
    .value("OP_OR", OperationExpression::OpOr)
    .value("OP_XOR", OperationExpression::OpXor)
    .value("OP_LLS", OperationExpression::OpLls)
    .value("OP_LRS", OperationExpression::OpLrs)
    .value("OP_ARS", OperationExpression::OpArs)
    .value("OP_ROL", OperationExpression::OpRol)
    .value("OP_ADD", OperationExpression::OpAdd)
    .value("OP_SUB", OperationExpression::OpSub)
    .value("OP_MUL", OperationExpression::OpMul)
    .value("OP_SDIV", OperationExpression::OpSDiv)
    .value("OP_UDIV", OperationExpression::OpUDiv)
    .value("OP_SMOD", OperationExpression::OpSMod)
    .value("OP_UMOD", OperationExpression::OpUMod)
    .value("OP_SEXT", OperationExpression::OpSext)
    .value("OP_ZEXT", OperationExpression::OpZext)
    .value("OP_INSERTBITS", OperationExpression::OpInsertBits)
    .value("OP_EXTRACTBITS", OperationExpression::OpExtractBits)
    .value("OP_CLEARBITS", OperationExpression::OpClearBits)
    .value("OP_BCAST", OperationExpression::OpBcast)
    ;
  rMod.def("expr_op_get_name", expr_op_get_name);

  py::class_<ExpressionVisitor>(rMod, "ExpressionVisitor");

  py::class_<NormalizeIdentifier>(rMod, "NormalizeIdentifier", py::base<ExpressionVisitor>())
    .def(py::init<CpuInformation const&, u8>())
    ;

  py::class_<IdentifierToVariable>(rMod, "IdentifierToVariable", py::base<ExpressionVisitor>())
    .def(py::init<>())
    ;

  py::class_<FilterVisitor>(rMod, "FilterVisitor", py::base<ExpressionVisitor>())
    .def(py::init<FilterVisitor::Matcher, size_t>())
    .def("get_matched_expressions", &FilterVisitor::GetMatchedExpressions)
    ;

  py::class_<SymbolicVisitor>(rMod, "SymbolicVisitor", py::base<ExpressionVisitor>())
    .def(py::init<Document const&, u8, bool>())
    .def("fork", &SymbolicVisitor::Fork)
    .def("get_expression", &SymbolicVisitor::GetExpression)
    .def("find_expression", &SymbolicVisitor::FindExpression)
    .def("bind_expression", &SymbolicVisitor::BindExpression)
    .def("update_address", &SymbolicVisitor::UpdateAddress)
    .def("find_all_paths", &SymbolicVisitor::FindAllPaths)
    .def("__str__", &SymbolicVisitor::ToString)
    .def_property_readonly("expressions", &SymbolicVisitor::GetExpressions)
    ;
}