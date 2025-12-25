#pragma once
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "Token.h"
#include <string>

struct HashEntry {
    Token token;    // хранимый токен
    int index;      // индекс в таблице
    bool occupied;  // флаг - зан€та ли €чейка

    HashEntry() : token(), index(-1), occupied(false) {}
};

class HashTable {
private:
    static const int TABLE_SIZE = 100;  // фиксированный размер таблицы
    HashEntry* table;                   // массив элементов таблицы
    int currentIndex;                   // текущий свободный индекс

    // ѕриватные методы:
    int hashFunction(const std::string& key);      // вычисление хеша
    int findEmptySlot(const std::string& key);     // поиск свободной €чейки

public:
    HashTable();                        // конструктор
    ~HashTable();                       // деструктор

    int insert(const Token& token);     // добавление токена
    int find(const std::string& value); // поиск токена по значению
    void printToFile(const std::string& filename); // вывод таблицы в файл
};

#endif