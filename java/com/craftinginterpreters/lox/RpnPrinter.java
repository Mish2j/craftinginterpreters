package com.craftinginterpreters.lox;

class RpnPrinter implements Expr.Visitor<String> {

    String print(Expr expr) {
        return expr.accept(this);
    }

   @Override
    public String visitConditionalExpr(Expr.Conditional expr) {
    return expr.condition.accept(this) + " " +
            expr.thenBranch.accept(this) + " " +
            expr.elseBranch.accept(this) + " ?:";
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        if (expr.value == null) return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return expr.expression.accept(this);
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        // operand then operator
        return expr.right.accept(this) + " " + expr.operator.lexeme;
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        // left right op
        return expr.left.accept(this) + " " + expr.right.accept(this) + " " + expr.operator.lexeme;
    }

    @Override
    public String visitVariableExpr(Expr.Variable expr) {
        return expr.name.lexeme;
    }

    @Override
    public String visitAssignExpr(Expr.Assign expr) {
        // value name =
        return expr.value.accept(this) + " " + expr.name.lexeme + " =";
    }

    @Override
    public String visitLogicalExpr(Expr.Logical expr) {
        // left right (and/or)
        return expr.left.accept(this) + " " + expr.right.accept(this) + " " + expr.operator.lexeme;
    }

    @Override
    public String visitCallExpr(Expr.Call expr) {
        // callee args... callN  (encode arity so it's unambiguous)
        StringBuilder sb = new StringBuilder();
        sb.append(expr.callee.accept(this));
        for (Expr arg : expr.arguments) {
            sb.append(" ").append(arg.accept(this));
        }
        sb.append(" call").append(expr.arguments.size());
        return sb.toString();
    }

    @Override
    public String visitGetExpr(Expr.Get expr) {
        // object name .
        return expr.object.accept(this) + " " + expr.name.lexeme + " .";
    }

    @Override
    public String visitSetExpr(Expr.Set expr) {
        // object value name set
        return expr.object.accept(this) + " " + expr.value.accept(this) + " " + expr.name.lexeme + " set";
    }

    @Override
    public String visitThisExpr(Expr.This expr) {
        return "this";
    }

    @Override
    public String visitSuperExpr(Expr.Super expr) {
        // super method .
        return "super " + expr.method.lexeme + " .";
    }
}
