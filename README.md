# SpreadSheet
## Электронная таблица
Программная реализация электронной таблицы это упрощённый аналог существующих решений: лист таблицы Microsoft Excel или Google Sheets.
В ячейках таблицы могут быть текст или формулы. Формулы, как и в существующих решениях, могут содержать индексы ячеек.
Проверка на зацикливание.
Сохранение ранее вычисленных значений в кеш.

## Использование:
0. Установка и настройка всех требуемых компонентов в среде разработки для запуска приложения
    - Установить Java 11 https://www.oracle.com/java/technologies/downloads/#java11 
1. ANTLR 4.7.2 уже необходимый файл лежит с проектом, дополнительно ничего делать не нужно 
2. ANTLR 4 C++ Runtime необходимый файл лежит с проектом, дополнительно ничего делать не нужно 
3. Вариант использования показан в main.cpp 

<details><summary>Пример работы с таблицей</summary>
  
~~~
// создать таблицу
auto sheet = CreateSheet();
// наполнить таблицу значениями
sheet->SetCell("E2"_pos, "=E4");
sheet->SetCell("E4"_pos, "=X9");
sheet->SetCell("X9"_pos, "=M6");
sheet->SetCell("M6"_pos, "Ready");
sheet->SetCell("A2"_pos, "10");
sheet->SetCell("A4"_pos, "=A2 + 10");
// получить значение в ячейках
ASSERT_EQUAL(sheet->GetCell("M6"_pos)->GetText(), "Ready");
ASSERT_EQUAL(std::get<double>(sheet->GetCell("A4"_pos)->GetValue()), 20.0);
~~~
</details>
  
## Cистемные требования:
- С++17 (STL);
- Java 11;
- GCC (MinGW-w64) 11.2.0.
  
## Стек технологий:
- CMake 3.22.0;
- ANTLR 4.7.2;
- ANTLR 4 C++ Runtime.
  
