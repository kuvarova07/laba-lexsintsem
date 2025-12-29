#include "HashTable.h"
#include <fstream>
#include <iostream>

HashTable::HashTable() : currentIndex(0) {
    table = new HashEntry[TABLE_SIZE];
}

HashTable::~HashTable() {
    delete[] table;
}

int HashTable::hashFunction(const std::string& key) {
    int hash = 0;
    for (char c : key) {
        hash = (hash * 31 + c) % TABLE_SIZE;
    }
    return hash;
}

int HashTable::findEmptySlot(const std::string& key) {
    int index = hashFunction(key);  
    int originalIndex = index;     

    while (table[index].occupied && table[index].token.value != key) {
        index = (index + 1) % TABLE_SIZE;  
        if (index == originalIndex) {
            return -1;  
        }
    }
    return index;
}

int HashTable::insert(const Token& token) {
    if (currentIndex >= TABLE_SIZE) {
        return -1;
    }

    int index = findEmptySlot(token.value);
    if (index == -1) {
        return -1;  
    }

    if (!table[index].occupied) {
        table[index].token = token;      
        table[index].index = currentIndex; 
        table[index].occupied = true;   
        currentIndex++;                  
    }

    return table[index].index; 
}

int HashTable::find(const std::string& value) {
    int index = hashFunction(value); 
    int originalIndex = index;        

    while (table[index].occupied) {
        if (table[index].token.value == value) {
            return table[index].index;  
        }
        index = (index + 1) % TABLE_SIZE;  
        if (index == originalIndex) {
            break; 
        }
    }
    return -1; 
}

void HashTable::printToFile(const std::string& filename) {
    std::ofstream outFile(filename, std::ios::app);  
    if (!outFile.is_open()) {
        std::cerr << "Failed to open output file!" << std::endl;
        return;
    }

    outFile << "\nHash table:\n";
    outFile << "Token type | Token | Index in hash table\n";
    outFile << "--------------------------------------------\n";

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (table[i].occupied) {
            outFile << static_cast<int>(table[i].token.type) << " | "
                << table[i].token.value << " | "
                << table[i].index << std::endl;
        }
    }

    outFile.close();
}