#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "ParseTreeNode.h"
#include <fstream>
#include <string>
#include <vector>

class Parser {
private:
    Lexer& lexer;
    std::ofstream& outputFile;
    Token currentToken;
    bool hasError;
    std::vector<std::string> errorMessages;
    ParseTreeNode* parseTreeRoot;  // НОВОЕ ПОЛЕ: корень дерева разбора

    void checkSemicolon();
    void advanceToken();
    bool match(TokenType expectedType);
    void error(const std::string& message);
    void syncTo(const std::vector<TokenType>& syncTokens);
    void printTree(ParseTreeNode* node, int depth = 0);
    void printErrors();

    // Правила грамматики
    ParseTreeNode* parseProcedure();
    ParseTreeNode* parseBegin();
    ParseTreeNode* parseDescriptions();
    ParseTreeNode* parseDescrList();
    ParseTreeNode* parseDescr();
    ParseTreeNode* parseVarList();
    ParseTreeNode* parseOperators();
    ParseTreeNode* parseOp();
    ParseTreeNode* parseAssignment();
    ParseTreeNode* parseIfStatement();
    ParseTreeNode* parseExpr();
    ParseTreeNode* parseSimpleExpr();
    ParseTreeNode* parseCondition();

public:
    Parser(Lexer& lex, std::ofstream& out);
    bool parse();                         // Старый метод (удаляет дерево)
    bool parseForSemantic();              // Новый метод (сохраняет дерево)
    ParseTreeNode* getParseTree() const { return parseTreeRoot; }  // Получение дерева
    bool hasErrors() const { return hasError; }
};

#endif