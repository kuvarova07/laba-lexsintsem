#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include "ParseTreeNode.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include <string>
#include <fstream>

// Структура для хранения информации о переменной
struct VariableInfo {
    std::string name;
    std::string type;
    int line;          // Строка объявления
    int position;      // Позиция в строке

    // Конструктор по умолчанию
    VariableInfo() : name(""), type(""), line(0), position(0) {}

    // Конструктор с параметрами
    VariableInfo(const std::string& n, const std::string& t, int l, int p)
        : name(n), type(t), line(l), position(p) {
    }
};

// Структура для хранения информации о процедуре
struct ProcedureInfo {
    std::string name;
    std::string returnType;  // "void" для процедур без возвращаемого значения
    int line;

    // Конструктор по умолчанию
    ProcedureInfo() : name(""), returnType(""), line(0) {}

    // Конструктор с параметрами
    ProcedureInfo(const std::string& n, const std::string& rt, int l)
        : name(n), returnType(rt), line(l) {
    }
};

class SemanticAnalyzer {
private:
    std::ofstream& outputFile;
    std::vector<std::string> errorMessages;
    bool hasError;

    // Таблицы символов
    std::unordered_map<std::string, VariableInfo> globalSymbolTable;
    std::unordered_map<std::string, ProcedureInfo> procedureTable;

    // Для отслеживания текущего контекста
    std::string currentProcedure;
    std::unordered_set<std::string> usedVariables;  // Для проверки объявленных переменных
    std::unordered_set<std::string> declaredVariables; // Объявленные переменные

    // Стек для хранения типов выражений
    std::stack<std::string> typeStack;

    // Для генерации постфиксной записи
    std::string postfixCode;

    // Вспомогательные методы
    void error(const std::string& message, int line, int position);
    void printErrors();

    // Для меток и переходов
    int labelCounter;
    std::string generateLabel();

    // Методы для проверки семантических правил
    void checkVariableDeclaration(const std::string& varName, int line, int position);
    void checkVariableUsage(const std::string& varName, int line, int position);
    void checkTypeCompatibility(const std::string& expected, const std::string& actual,
        int line, int position, const std::string& context = "");

    // Методы для обхода дерева разбора
    void traverseProcedure(ParseTreeNode* node);
    void traverseBegin(ParseTreeNode* node);
    void traverseDescriptions(ParseTreeNode* node);
    void traverseDescr(ParseTreeNode* node);
    void traverseVarList(ParseTreeNode* node, const std::string& type);
    void traverseOperators(ParseTreeNode* node);
    void traverseOp(ParseTreeNode* node);
    void traverseAssignment(ParseTreeNode* node);
    void traverseIfStatement(ParseTreeNode* node);
    void traverseExpr(ParseTreeNode* node);
    void traverseSimpleExpr(ParseTreeNode* node);
    void traverseCondition(ParseTreeNode* node);

    // Методы для генерации постфиксной записи
    void generatePostfixForExpr(ParseTreeNode* node);
    void generatePostfixForSimpleExpr(ParseTreeNode* node);
    void generatePostfixForCondition(ParseTreeNode* node);

public:
    SemanticAnalyzer(std::ofstream& out);
    bool analyze(ParseTreeNode* root);
    void printPostfixCode();
    bool hasErrors() const { return hasError; }
};

#endif