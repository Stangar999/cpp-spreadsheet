#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet()
{}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    CheckPosAndResizeTable(pos);
    // повторный вызов метода Sheet::SetCell() с теми же аргументами — за O(1);
    if(table_[pos.row][pos.col].GetText() == text) {
        return;
    }
    // создается новая ячейка с заданным текстом и списком ячеек от которых она звисит
    Cell cell(text, this);
    // присваиваем список всех ячеек которые зависят от этой позиции новой ячейке
    cell.GetReferencedBackCells() = table_[pos.row][pos.col].GetReferencedBackCells();
    // перед проверкой на цикличность надо сбросить флаг провереного кеша у требуемой ветви
    InValidedFlagCicl(cell.GetReferencedBackCells());
    // проверка на цикличность
    CheckOnCicl(&cell, pos);
    // инвалидируем кеш и флаг проверки на цикл для зависимых ячеек
    InValidedCache(cell.GetReferencedBackCells());
    // если в формуле использовались еще не заданные ячейки
    for(Position pos_in_ref : cell.GetReferencedCells()) {
        CheckPosAndResizeTable(pos_in_ref);
    }
    // если от ячейки кто-то зависел значит надо удалить ее из списков обратных зависимостей этих ячеек
    ClearCellFromOldBackRefs(table_[pos.row][pos.col].GetReferencedCells(), pos);
    // заполняем обратные зависимости у тех ячеек от которых зависит новая ячейка
    FillBackRefsInCell(cell.GetReferencedCells(), pos);
    table_[pos.row][pos.col] = std::move(cell);
    ResizePrintSize(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckPos(pos);
    if(PosRowMoreTableRow(pos) || PosColMoreTableCol(pos)) {
        return nullptr;
    }
    return &table_[pos.row][pos.col];
}

void Sheet::ClearCell(Position pos) {
    CheckPos(pos);
    if(PosRowMoreTableRow(pos) || PosColMoreTableCol(pos)) {
        return;
    }
    table_[pos.row][pos.col].Clear();
    std::optional<Size> new_size;
    std::optional<size_t> new_max_pos_in_row_after_del;
    for(int row = table_.size(); row > 0; --row) {
        for(int col = table_[row - 1].size(); col > 0; --col) {
            if(!table_[row - 1][col - 1].GetText().empty()) {
                if(!new_size) {
                    new_size = {0, 0};
                }
                new_size->rows = new_size->rows < static_cast<int>(row) ? row : new_size->rows;
                new_size->cols = new_size->cols < static_cast<int>(col) ? col : new_size->cols;
                if(pos.row == static_cast<int>(row - 1)) {
                    new_max_pos_in_row_after_del = new_size->cols;
                }
                break;
            }
        }
    }
    // после удаления остались не пустые ячейки, приводит область печати и размер таблицы к новому размеру
    if(new_size) {
        print_size_ = new_size.value();
        table_.resize(new_size->rows);
    } else { // все ячейки пустые приводит область печати и размер таблицы к новому размеру
        print_size_ = {0, 0};
        table_.resize(0);
    }

    if(new_max_pos_in_row_after_del) {
        table_[pos.row].resize(new_max_pos_in_row_after_del.value());
    }
}

Size Sheet::GetPrintableSize() const {
   return print_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    ParseTableForPrint(output, std::bind(&Sheet::GetValueVariantAndSetOutput, this, std::placeholders::_1, std::placeholders::_2 ));
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto func = [this](Position pos, std::ostream& output) {
        output << table_[pos.row - 1][pos.col - 1].GetText();
    };
    ParseTableForPrint(output, func);
}

void Sheet::CheckPos(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("!pos.IsValid()");
    }
}

void Sheet::CheckOnCicl(CellInterface* current_cell, Position main_pos) {
    Cell* cur_cell = dynamic_cast<Cell*>(current_cell);
    // Случай когда "A1"_pos, "=B2" а B2 никто не создал
    // чтобы не портить таблицу до проверки на цикличность
    if(cur_cell == nullptr) {
        return;
    }
    if(cur_cell->GetFlagIsNotCicl() == std::nullopt) {
        for(const Position& pos : current_cell->GetReferencedCells() ) {
            if(main_pos == pos) {
                throw CircularDependencyException("FALSE IsCicl");
            }
            CellInterface* cur_cell = GetCell(pos);
            CheckOnCicl(cur_cell, main_pos);
        }
        // если не выпали с исключением значит для этой ячейки цикла нет
        cur_cell->GetFlagIsNotCicl() = true;
    }
}

void Sheet::FillBackRefsInCell(const std::vector<Position>& vec_pos_cel, Position pos) {
    for(const Position& cur_pos : vec_pos_cel ) {
        dynamic_cast<Cell*>(GetCell(cur_pos))->GetReferencedBackCells().insert(pos);
    }
}

void Sheet::ClearCellFromOldBackRefs(const std::vector<Position>& vec_pos_cel, Position pos) {
    for(const auto pos_curr : vec_pos_cel) {
        Cell* cell = dynamic_cast<Cell*>(GetCell(pos_curr));
        cell->GetReferencedBackCells().erase(pos);
    }
}

void Sheet::InValidedFlagCicl(const std::set<Position>& set_pos_back_cel) {
    for(const auto pos_curr : set_pos_back_cel) {
        Cell* cell = dynamic_cast<Cell*>(GetCell(pos_curr));
        if(cell->GetFlagIsNotCicl()) {
            cell->GetFlagIsNotCicl() = std::nullopt;
            InValidedFlagCicl(cell->GetReferencedBackCells());
        }

    }
}

void Sheet::InValidedCache(const std::set<Position>& set_pos_back_cel) {
    for(const auto pos_curr : set_pos_back_cel) {
        Cell* cell = dynamic_cast<Cell*>(GetCell(pos_curr));
        // если кеш валидный делает его не валидным
        // и опускается глубже пока не встретит невалидный кеш
        // когда встречен не валидный кеш или закончилось перечисление
        // обход начинается в ширину
        if(cell->CacheIsValid()) {
            cell->SetInValidCache();
            InValidedCache(cell->GetReferencedBackCells());
        }
    }
}

void Sheet::CheckPosAndResizeTable(Position pos) {
    CheckPos(pos);
    if(PosRowMoreTableRow(pos)) {
        table_.resize(pos.row + 1);
    }
    if(PosColMoreTableCol(pos)) {
        table_[pos.row].resize(pos.col + 1);
    }
}

bool Sheet::PosRowMoreTableRow(Position pos) {
    return table_.size() < static_cast<size_t>(pos.row + 1);
}

bool Sheet::PosColMoreTableCol(Position pos) {
    return table_[pos.row].size() < static_cast<size_t>(pos.col + 1);
}

void Sheet::ResizePrintSize(Position pos) {
    if(print_size_.rows < pos.row + 1) {
        print_size_.rows = pos.row + 1;
    }
    if(print_size_.cols < pos.col + 1) {
        print_size_.cols = pos.col + 1;
    }
}

void Sheet::GetValueVariantAndSetOutput(Position pos, std::ostream& output) const {
    std::visit([&output](const auto &elem)
    { output << elem; }
    , table_[pos.row - 1][pos.col -  1].GetValue());
}

void Sheet::ParseTableForPrint(std::ostream& output, std::function<void (Position, std::ostream&)> func) const {
    for(int row = 1; row <= print_size_.rows; ++row) {
        bool first_val = true;
        for(int col = 1; col <= print_size_.cols; ++col) {
            if(!first_val) {
                output << '\t';
            }
            first_val = false;
            if(table_[row - 1].size() >= static_cast<size_t>(col)) {
                func(Position().SetRow(row).SetCol(col), output);
            }
        }
        output << '\n';
    }
}


std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}


