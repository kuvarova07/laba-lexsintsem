#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include "ParseTreeNode.h"
#include "HashTable.h"
#include <string>
#include <fstream>


class StringStack {
private:
    std::string data[100];
    int topIndex;
public:
    StringStack();
    void clear();
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
    void clear();
    void insert(const std::string& value);
    bool contains(const std::string& value) const;
};

class ErrorList {
private:
    std::string errors[50];
    int errorCount;
public:
    ErrorList();
    void clear();
    void add(const std::string& error);
    int size() const;
    std::string get(int index) const;
};

class SemanticAnalyzer {
private:
    std::ofstream& outputFile;
    bool hasError;
    std::string currentProcedure;
    int labelCounter;

    HashTable symbolTable;
    StringSet usedVariables;
    StringSet declaredVariables;
    StringStack typeStack;
    ErrorList errorMessages;

    std::string postfixCode;

    std::string getVariableType(const std::string& varName);
    void markVariableInitialized(const std::string& varName);
    void checkUnusedVariables();

    void error(const std::string& message, int line, int position);
    void printErrors();
    void checkVariableUsage(const std::string& varName, int line, int position);
    void checkTypeCompatibility(const std::string& expected, const std::string& actual,
        int line, int position, const std::string& context = "");

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

public:
    SemanticAnalyzer(std::ofstream& out);
    bool analyze(ParseTreeNode* root);
    bool hasErrors() const { return hasError; }
};

#endif