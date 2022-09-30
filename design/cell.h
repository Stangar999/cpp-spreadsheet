#pragma once

#include "common.h"
#include "formula.h"
#include <optional>
#include <set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell();
    Cell(std::string text, const SheetInterface* sheet);
    ~Cell();

    Cell(Cell&& cell);

    Cell& operator=(Cell&& cell);

    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    std::set<Position>& GetReferencedBackCells();

    bool CacheIsValid() const;

    void SetInValidCache();

    std::optional<bool>& GetFlagIsNotCicl();

private:
    class Impl {
    public:
        std::variant<std::monostate, std::string> str_;
        virtual ~Impl() = default;
    };
    class EmptyImpl : public Impl{
    public:
        EmptyImpl(){
            str_= std::monostate();
        }
    };
    class TextImpl : public Impl{
    public:
        TextImpl(const std::string& str = ""){
            str_= str;
        }
    };
    class FormulaImpl : public Impl{
    public:
        FormulaImpl(const std::string& str = ""){
            str_ = str;
        }
        std::unique_ptr<FormulaInterface> form_inter_;
    };

    // устанавливает значение ячейки и указатель на страницу
    void Set(std::string text);

    // проверяет что в ячейке нет циклической зависимости
    bool IsNotCicl(const std::vector<Position>& vec_pos_cel) const;

    void FillBackRef(const std::vector<Position>& vec_pos_cel) const;

    std::unique_ptr<Impl> impl_;

    // вычисленное ранее значение
    mutable std::optional<Value> cache_;

    std::optional<bool> is_not_cicl;

    // указатель на страницу
    const SheetInterface* sheet_;

    // список ячеек, которые непосредственно задействованы в данной
    // формуле. Список отсортирован по возрастанию и не содержит повторяющихся
    // ячеек. В случае текстовой ячейки список пуст.
    std::vector<Position> referenced_cells_;

    // список ячеек, которые обратно зависят(зависимы) от этой ячейки
    std::set<Position> referenced_back_cells_;
};

