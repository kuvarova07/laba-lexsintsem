#pragma once
#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include "HashTable.h"
#include <fstream>
#include <string>

class Lexer {
private:
    std::ifstream inputFile;   
    std::ofstream outputFile;  

    HashTable hashTable;       

    char currentChar;          
    int line;                 
    int position;             
    bool hasError;            

    void skipWhitespace();              
    void nextChar();                    
    Token parseIdentifier();            
    Token parseNumber();                
    Token parseOperator();              
    std::string tokenTypeToString(TokenType type); 

public:
    Lexer(const std::string& inputFilename, const std::string& outputFilename);
    ~Lexer();

    Token getNextToken();      
    bool hasErrors() const { return hasError; } 
    void analyze();            
};

#endif

