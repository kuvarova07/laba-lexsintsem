#pragma once
#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    PROCEDURE, BEGIN, END, VAR, INTEGER, IF, THEN, ELSE,

    ID, CONST,

    ASSIGN, PLUS, MINUS, EQUAL, NOT_EQUAL, GREATER, LESS,

    SEMICOLON, COLON, COMMA, LPAREN, RPAREN,

    END_OF_FILE, ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int position;
    std::string errorMessage;

    Token(TokenType t = TokenType::END_OF_FILE,
        const std::string& v = "",
        int l = 1, int p = 1)
        : type(t), value(v), line(l), position(p) {
    }
};

#endif
