#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "ParseTreeNode.h"
#include <fstream>
#include <string>
#include <vector>

class Parser {
private:
    // Основные компоненты парсера
    Lexer& lexer;                    // Ссылка на лексический анализатор
    std::ofstream& outputFile;       // Выходной файл для результатов
    Token currentToken;              // Текущий обрабатываемый токен
    bool hasError;                   // Флаг наличия ошибок
    std::vector<std::string> errorMessages;  // Список сообщений об ошибках

    // Вспомогательные методы
    void checkSemicolon();                          // Проверка точки с запятой
    void advanceToken();                            // Переход к следующему токену
    bool match(TokenType expectedType);             // Проверка типа токена
    std::string getTokenInfo();                     // Получение информации о токене
    void error(const std::string& message);         // Обработка ошибки
    void syncTo(const std::vector<TokenType>& syncTokens);  // Синхронизация после ошибки
    void printTree(ParseTreeNode* node, int depth = 0);     // Вывод дерева разбора
    void printErrors();                             // Вывод списка ошибок
    std::string getExpressionValue(ParseTreeNode* exprNode); // Получение значения выражения

    // Правила грамматики
    ParseTreeNode* parseProcedure();
    ParseTreeNode* parseBegin();
    ParseTreeNode* parseEnd();
    ParseTreeNode* parseProcedureName();
    ParseTreeNode* parseDescriptions();
    ParseTreeNode* parseDescrList();
    ParseTreeNode* parseDescr();
    ParseTreeNode* parseVarList();
    ParseTreeNode* parseType();
    ParseTreeNode* parseOperators();
    ParseTreeNode* parseOp();
    ParseTreeNode* parseAssignment();
    ParseTreeNode* parseIfStatement();
    ParseTreeNode* parseExpr();
    ParseTreeNode* parseSimpleExpr();
    ParseTreeNode* parseCondition();
    ParseTreeNode* parseRelationOperator();

public:
    // Конструктор и публичные методы
    Parser(Lexer& lex, std::ofstream& out);  // Конструктор
    bool parse();                            // Основной метод синтаксического анализа
    bool hasErrors() const { return hasError; }  // Проверка наличия ошибок
};

#endif