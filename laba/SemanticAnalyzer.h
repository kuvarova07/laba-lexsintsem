#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include "ParseTreeNode.h"
#include <string>
#include <fstream>

struct VariableInfo {
    std::string name;
    std::string type;
    int line;          
    int position;      

    VariableInfo() : name(""), type(""), line(0), position(0) {}

    VariableInfo(const std::string& n, const std::string& t, int l, int p)
        : name(n), type(t), line(l), position(p) {
    }
};

struct ProcedureInfo {
    std::string name;
    std::string returnType; 
    int line;

    ProcedureInfo() : name(""), returnType(""), line(0) {}

    ProcedureInfo(const std::string& n, const std::string& rt, int l)
        : name(n), returnType(rt), line(l) {
    }
};

class StringStack {
private:
    std::string data[100];
    int topIndex;
public:
    StringStack();
    void clear() { topIndex = -1; }
    void push(const std::string& value);
    std::string pop();
    std::string top() const;
    bool empty() const;
};

class StringSet {
private:
    std::string data[100];
    int count;
public:
    StringSet();
    void clear() { count = 0; }
    void insert(const std::string& value);
    bool contains(const std::string& value) const;
};

class ErrorList {
private:
    std::string errors[50];
    int errorCount;
public:
    ErrorList();
    void clear() { errorCount = 0; }
    void add(const std::string& error);
    int size() const;
    std::string get(int index) const;
};

struct SymbolTableEntry {
    std::string name;
    VariableInfo info;
    bool occupied;
};

class SymbolTable {
private:
    SymbolTableEntry table[100];
    int count;
public:
    SymbolTable();
    void insert(const std::string& name, const VariableInfo& info);
    VariableInfo* find(const std::string& name);
    bool contains(const std::string& name) const;
    void clear() {
        count = 0;
        for (int i = 0; i < 100; i++) {
            table[i].occupied = false;
        }
    }
};

class SemanticAnalyzer {
private:
    std::ofstream& outputFile;
    bool hasError;
    std::string currentProcedure;
    int labelCounter;

    SymbolTable globalSymbolTable;
    StringSet usedVariables;
    StringSet declaredVariables;
    StringStack typeStack;
    ErrorList errorMessages;

    std::string postfixCode;

    void error(const std::string& message, int line, int position);
    void printErrors();
    void checkVariableDeclaration(const std::string& varName, int line, int position);
    void checkVariableUsage(const std::string& varName, int line, int position);
    void checkTypeCompatibility(const std::string& expected, const std::string& actual,
        int line, int position, const std::string& context = "");

    void traverseProcedure(ParseTreeNode* node);
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

    void generatePostfixForExpr(ParseTreeNode* node);
    void generatePostfixForSimpleExpr(ParseTreeNode* node);
    void generatePostfixForCondition(ParseTreeNode* node);
    std::string generateLabel();

public:
    SemanticAnalyzer(std::ofstream& out);
    void clear();  
    bool analyze(ParseTreeNode* root);
    bool hasErrors() const { return hasError; }
};

#endif