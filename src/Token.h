#pragma once
#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Перечисление всех возможных типов токенов (лексем)
enum class TokenType {
    // Ключевые слова языка
    PROCEDURE, BEGIN, END, VAR, INTEGER, IF, THEN, ELSE,

    // Идентификаторы (имена переменных) и константы (числа)
    ID, CONST,

    // Операторы: присваивание, арифметические, сравнения
    ASSIGN, PLUS, MINUS, EQUAL, NOT_EQUAL, GREATER, LESS,

    // Разделители: точка с запятой, двоеточие, запятая, скобки
    SEMICOLON, COLON, COMMA, LPAREN, RPAREN,

    // Служебные типы для управления анализом
    END_OF_FILE, ERROR  // конец файла и ошибка
};

// Структура для хранения информации о токене
struct Token {
    TokenType type;      // тип токена (из перечисления выше)
    std::string value;   // текстовое значение токена (например, "x", "10", "+")
    int line;           // номер строки в исходном коде
    int position;       // позиция в строке
    std::string errorMessage;  // поле для сообщения об ошибке

    // Конструктор с параметрами по умолчанию
    Token(TokenType t = TokenType::END_OF_FILE,
        const std::string& v = "",
        int l = 1, int p = 1)
        : type(t), value(v), line(l), position(p) {
    }
};

#endif
