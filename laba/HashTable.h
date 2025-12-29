#pragma once
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "Token.h"
#include <string>

struct HashEntry {
    Token token;
    int index;
    bool occupied;

    HashEntry() : token(), index(-1), occupied(false) {}
};

class HashTable {
private:
    static const int TABLE_SIZE = 100;
    HashEntry* table;
    int currentIndex;

    int hashFunction(const std::string& key);     
    int findEmptySlot(const std::string& key);

public:
    HashTable();
    ~HashTable();

    int insert(const Token& token);
    int find(const std::string& value);
    void printToFile(const std::string& filename);
    Token* getTokenByIndex(int index);
    Token* getTokenByValue(const std::string& value);
};

#endif