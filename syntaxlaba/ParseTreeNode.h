#ifndef PARSETREENODE_H
#define PARSETREENODE_H

#include <string>
#include <vector>

// Узел дерева разбора для хранения синтаксической структуры программы
struct ParseTreeNode {
    std::string name;                       // Имя узла (тип конструкции)
    std::string value;                      // Значение (для терминалов)
    std::vector<ParseTreeNode*> children;   // Дочерние узлы
    int line;                               // Номер строки в исходном коде
    int position;                           // Позиция в строке

    // Конструктор
    ParseTreeNode(const std::string& n, const std::string& v = "", int l = 0, int p = 0)
        : name(n), value(v), line(l), position(p) {
    }

    // Деструктор
    ~ParseTreeNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

#endif