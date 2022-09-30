#pragma once

#include "FormulaLexer.h"
#include "common.h"

#include <forward_list>
#include <functional>
#include <stdexcept>

namespace ASTImpl {
class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr,
                        std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute(const SheetInterface& sheet) const;
    void PrintCells(std::ostream& out) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    std::forward_list<Position>& GetCells() {
        return cells_;
    }

    const std::forward_list<Position>& GetCells() const {
        return cells_;
    }

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;

    // physically stores cells so that they can be
    // efficiently traversed without going through
    // the whole AST
    std::forward_list<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);

template <typename T>
bool SafeAdd(T a, T b) {
    if(a >= 0 && b >= 0) {
        if(a > std::numeric_limits<T>::max() - b) {
            return false;
        }
    } else if (a < 0 && b < 0) {
        if(a < std::numeric_limits<T>::min() - b) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool SafeSubtract(T a, T b) {
    if(a >= 0 && b <= 0) {
        if(a > std::numeric_limits<T>::max() + b) {
            return false;
        }
    } else if (a < 0 && b > 0) {
        if(a < std::numeric_limits<T>::min() + b) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool SafeMultiply(T a, T b) {
    // если b = 0
    if(std::abs(b) < std::numeric_limits<double>::epsilon()) {
        return true;
    }
    if(std::abs(a) > std::numeric_limits<T>::max() / std::abs(b)) {
        return false;
    }
    return true;
}
