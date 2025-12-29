#include "Lexer.h"
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string& inputFilename, const std::string& outputFilename)
    : line(1), position(1), hasError(false) {

    inputFile.open(inputFilename);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open input file: " << inputFilename << std::endl;
        hasError = true;
    }

    outputFile.open(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file: " << outputFilename << std::endl;
        hasError = true;
    }

    if (inputFile.is_open()) {
        inputFile.get(currentChar);
    }
}

Lexer::~Lexer() {
    if (inputFile.is_open()) inputFile.close();
    if (outputFile.is_open()) outputFile.close();
}

void Lexer::nextChar() {
    if (inputFile.get(currentChar)) {
        position++;
    }
    else {
        currentChar = '\0';
    }
}

void Lexer::skipWhitespace() {
    while (std::isspace(currentChar)) {
        if (currentChar == '\n') {
            line++;
            position = 1;
        }
        nextChar();
    }
}

Token Lexer::parseIdentifier() {
    std::string value;
    int startLine = line;
    int startPos = position;

    if (std::isalpha(currentChar)) {
        value += currentChar;
        nextChar();
    }
    else {
        std::string errorChar(1, currentChar);
        nextChar();
        Token errorToken(TokenType::ERROR, errorChar, startLine, startPos);
        errorToken.errorMessage = "Identifikator dolzhen nachinat'sya s bukvy";
        return errorToken;
    }

    while (std::isalpha(currentChar)) {
        value += currentChar;
        nextChar();
    }

    if (std::isdigit(currentChar)) {
        while (std::isalnum(currentChar)) {
            value += currentChar;
            nextChar();
        }
        Token errorToken(TokenType::ERROR, value, startLine, startPos);
        errorToken.errorMessage = "The identifier cannot contain numbers";
        return errorToken;
    }

    if (currentChar == '_') {
        value += currentChar;
        nextChar();
        Token errorToken(TokenType::ERROR, value, startLine, startPos);
        errorToken.errorMessage = "Identifikator ne mozhet soderzhat' simvol '_'";
        return errorToken;
    }

    if (value == "procedure") return Token(TokenType::PROCEDURE, value, startLine, startPos);
    if (value == "begin") return Token(TokenType::BEGIN, value, startLine, startPos);
    if (value == "end") return Token(TokenType::END, value, startLine, startPos);
    if (value == "var") return Token(TokenType::VAR, value, startLine, startPos);
    if (value == "integer") return Token(TokenType::INTEGER, value, startLine, startPos);
    if (value == "if") return Token(TokenType::IF, value, startLine, startPos);
    if (value == "then") return Token(TokenType::THEN, value, startLine, startPos);
    if (value == "else") return Token(TokenType::ELSE, value, startLine, startPos);

    return Token(TokenType::ID, value, startLine, startPos);
}

Token Lexer::parseNumber() {
    std::string value;
    int startLine = line;
    int startPos = position;

    while (std::isdigit(currentChar)) {
        value += currentChar;
        nextChar();
    }

    if (value.length() > 1 && value[0] == '0') {
        Token errorToken(TokenType::ERROR, value, startLine, startPos);
        errorToken.errorMessage = "The number cannot start with 0";
        return errorToken;
    }

    return Token(TokenType::CONST, value, startLine, startPos);
}

Token Lexer::parseOperator() {
    int startLine = line;
    int startPos = position;
    Token token;

    switch (currentChar) {
    case ':':
        nextChar();
        if (currentChar == '=') {
            nextChar();
            token = Token(TokenType::ASSIGN, ":=", startLine, startPos);
        }
        else {
            token = Token(TokenType::COLON, ":", startLine, startPos);
        }
        break;

    case ';':
        nextChar();
        token = Token(TokenType::SEMICOLON, ";", startLine, startPos);
        break;
    case ',':
        nextChar();
        token = Token(TokenType::COMMA, ",", startLine, startPos);
        break;
    case '(':
        nextChar();
        token = Token(TokenType::LPAREN, "(", startLine, startPos);
        break;
    case ')':
        nextChar();
        token = Token(TokenType::RPAREN, ")", startLine, startPos);
        break;
    case '+':
        nextChar();
        token = Token(TokenType::PLUS, "+", startLine, startPos);
        break;
    case '-':
        nextChar();
        token = Token(TokenType::MINUS, "-", startLine, startPos);
        break;
    case '=':
        nextChar();
        token = Token(TokenType::EQUAL, "=", startLine, startPos);
        break;
    case '>':
        nextChar();
        token = Token(TokenType::GREATER, ">", startLine, startPos);
        break;
    case '<':
        nextChar();
        if (currentChar == '>') {
            nextChar();
            token = Token(TokenType::NOT_EQUAL, "<>", startLine, startPos);
        }
        else {
            token = Token(TokenType::LESS, "<", startLine, startPos);
        }
        break;

    default:
        std::string errorChar(1, currentChar);
        nextChar();
        token = Token(TokenType::ERROR, errorChar, startLine, startPos);
        break;
    }

    return token;
}

std::string Lexer::tokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::PROCEDURE: return "PROCEDURE";
    case TokenType::BEGIN: return "BEGIN";
    case TokenType::END: return "END";
    case TokenType::VAR: return "VAR";
    case TokenType::INTEGER: return "INTEGER";
    case TokenType::IF: return "IF";
    case TokenType::THEN: return "THEN";
    case TokenType::ELSE: return "ELSE";
    case TokenType::ID: return "ID";
    case TokenType::CONST: return "CONST";
    case TokenType::ASSIGN: return "ASSIGN";
    case TokenType::PLUS: return "PLUS";
    case TokenType::MINUS: return "MINUS";
    case TokenType::EQUAL: return "EQUAL";
    case TokenType::NOT_EQUAL: return "NOT_EQUAL";
    case TokenType::GREATER: return "GREATER";
    case TokenType::LESS: return "LESS";
    case TokenType::SEMICOLON: return "SEMICOLON";
    case TokenType::COLON: return "COLON";
    case TokenType::COMMA: return "COMMA";
    case TokenType::LPAREN: return "LPAREN";
    case TokenType::RPAREN: return "RPAREN";
    case TokenType::END_OF_FILE: return "END_OF_FILE";
    case TokenType::ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}

Token Lexer::getNextToken() {
    skipWhitespace();

    if (currentChar == '\0') {
        return Token(TokenType::END_OF_FILE, "", line, position);
    }

    if (std::isalpha(currentChar) || currentChar == '_') {
        return parseIdentifier();
    }

    if (std::isdigit(currentChar)) {
        return parseNumber();
    }

    return parseOperator();
}

void Lexer::analyze() {
    if (!outputFile.is_open()) return;

    outputFile << "Results of lexical analysis:\n";
    outputFile << "Stroka | Pozitsiya | Tip | Leksema\n";
    outputFile << "---------------------------------\n";

    Token token;
    do {
        token = getNextToken();

        if (token.type != TokenType::END_OF_FILE) {
            outputFile << token.line << " | " << token.position << " | "
                << tokenTypeToString(token.type) << " | " << token.value << std::endl;
        }

        if (token.type == TokenType::ERROR) {
            outputFile << "Error in line " << token.line << ", position " << token.position
                << ": " << token.value;
            if (!token.errorMessage.empty()) {
                outputFile << " - " << token.errorMessage;
            }
            else {
                outputFile << " - Nedopustimyy simvol";
            }
            outputFile << std::endl;
            hasError = true;
        }
        else if (token.type != TokenType::END_OF_FILE) {
            hashTable.insert(token);
        }

    } while (token.type != TokenType::END_OF_FILE);

    hashTable.printToFile("output.txt");
}