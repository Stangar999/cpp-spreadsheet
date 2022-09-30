#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    void CheckPos(Position pos) const;
    void CheckOnCicl(CellInterface* cur_cell, Position main_pos);
    void FillBackRefsInCell(const std::vector<Position>& vec_pos_cel, Position pos);
    void ClearCellFromOldBackRefs(const std::vector<Position>& vec_pos_cel, Position pos);
    //void ClearFlagIsNotCiclInBackRefsCell(const std::vector<Position>& vec_pos_cel, Position pos);
    void InValidedCacheAndFlagCicl(const std::set<Position>& set_pos_back_cel);

    std::vector<std::vector<Cell>> table_;

    Size print_size_;
};
