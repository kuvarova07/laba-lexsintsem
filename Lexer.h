#pragma once
#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include "HashTable.h"
#include <fstream>
#include <string>

class Lexer {
private:
    std::ifstream inputFile;   // дл€ чтени€ входного файла
    std::ofstream outputFile;  // дл€ записи выходного файла

    HashTable hashTable;       // хеш-таблица дл€ хранени€ токенов

    // “екущее состо€ние анализатора
    char currentChar;          // текущий обрабатываемый символ
    int line;                 // текуща€ строка
    int position;             // текуща€ позици€ в строке
    bool hasError;            // флаг ошибки

    // ѕриватные методы анализа:
    void skipWhitespace();              // пропуск пробелов и переводов строк
    void nextChar();                    // чтение следующего символа
    Token parseIdentifier();            // разбор идентификатора или ключевого слова
    Token parseNumber();                // разбор числовой константы
    Token parseOperator();              // разбор операторов и разделителей
    std::string tokenTypeToString(TokenType type); // преобразование типа в строку

public:
    //  онструктор и деструктор
    Lexer(const std::string& inputFilename, const std::string& outputFilename);
    ~Lexer();

    // ќсновные методы
    Token getNextToken();      // получение следующего токена
    bool hasErrors() const { return hasError; } // проверка наличи€ ошибок
    void analyze();            // основной метод анализа
};

#endif

