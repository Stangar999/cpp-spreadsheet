#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::Cell() {}

Cell::Cell(std::string text, const SheetInterface* sheet)
    : sheet_(sheet)
{
    Set(std::move(text));
}

Cell::~Cell() {}

Cell::Cell(Cell&& cell)
    :impl_(std::move(cell.impl_))
    ,cache_(std::move(cell.cache_))
    ,is_not_cicl(std::move(cell.is_not_cicl))
    ,sheet_(cell.sheet_)
    ,referenced_cells_(std::move(cell.referenced_cells_))
    ,referenced_back_cells_(std::move(cell.referenced_back_cells_))
{}

Cell& Cell::operator=(Cell&& cell)
{
    if(this == &cell) {
        return *this;
    }
    impl_ = std::move(cell.impl_);
    cache_ = std::move(cell.cache_);
    is_not_cicl = std::move(cell.is_not_cicl);
    sheet_ = cell.sheet_;
    referenced_cells_ = std::move(cell.referenced_cells_);
    referenced_back_cells_ = std::move(cell.referenced_back_cells_);
    return *this;
}

void Cell::Set(std::string text) {
    if(text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if(text.size() > 1 && text[0] == FORMULA_SIGN) {
        std::string str = text.substr(1);

        FormulaImpl fl_imp(text);
        fl_imp.form_inter_ = ParseFormula(text.substr(1));
        referenced_cells_ = fl_imp.form_inter_->GetReferencedCells();
        impl_ = std::make_unique<FormulaImpl>(std::move(fl_imp));
    } else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

std::optional<bool>& Cell::GetFlagIsNotCicl() {
    return is_not_cicl;
}

bool Cell::CacheIsValid() const {
    return cache_.has_value();
}

void Cell::SetInValidCache() {
    cache_ = std::nullopt;
}

void Cell::Clear() {
    impl_ = nullptr;
    sheet_ = nullptr;
    SetInValidCache();
    is_not_cicl = std::nullopt;
    referenced_cells_.clear();
    referenced_back_cells_.clear();
}

Cell::Value Cell::GetValue() const {
    if(cache_.has_value()) {
        return cache_.value();
    }
    if(TextImpl* impl = dynamic_cast<TextImpl*>(impl_.get()); impl){
        const std::string& str = std::get<std::string>(impl->str_);
        if(str[0] == ESCAPE_SIGN) {
            cache_ = str.substr(1);
            return cache_.value();
        }
        cache_ = str;
        return str;
    } else if (FormulaImpl* impl = dynamic_cast<FormulaImpl*>(impl_.get()); impl) {
        auto value = impl->form_inter_->Evaluate(*sheet_);
        if(std::holds_alternative<double>(value)) {
            cache_ = std::get<double>(value);
            return cache_.value();
        }
        cache_ = std::get<FormulaError>(value);
        return std::get<FormulaError>(value);
    }
    cache_ = "";
    return "";
}

std::string Cell::GetText() const {
    if(TextImpl* impl = dynamic_cast<TextImpl*>(impl_.get()); impl){
        const std::string& str = std::get<std::string>(impl->str_);
        return str;
    } else if (FormulaImpl* impl = dynamic_cast<FormulaImpl*>(impl_.get()); impl) {
        return FORMULA_SIGN + impl->form_inter_->GetExpression();
    }
    return "";
}

std::vector<Position> Cell::GetReferencedCells() const {
    return referenced_cells_;
}

std::set<Position>& Cell::GetReferencedBackCells() {
    return referenced_back_cells_;
}
