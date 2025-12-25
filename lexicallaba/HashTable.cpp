#include "HashTable.h"
#include <fstream>
#include <iostream>

HashTable::HashTable() : currentIndex(0) {
    table = new HashEntry[TABLE_SIZE];
}

HashTable::~HashTable() {
    delete[] table;
}

// Хеш-функция - преобразует строку в индекс таблицы
int HashTable::hashFunction(const std::string& key) {
    int hash = 0;
    // Проходим по каждому символу строки
    for (char c : key) {
        hash = (hash * 31 + c) % TABLE_SIZE;  // Преобразуем строку в число, учитывая порядок символов: каждый следующий символ "взвешивается" сильнее предыдущего (это похоже на запись числа в системе счисления с основанием 31)
    }
    return hash;
}

// Поиск свободной ячейки (метод линейного пробирования)
int HashTable::findEmptySlot(const std::string& key) {
    int index = hashFunction(key);  // начальный индекс из хеш-функции
    int originalIndex = index;      // запоминаем начальный индекс

    // Ищем свободную ячейку или ячейку с таким же ключом
    while (table[index].occupied && table[index].token.value != key) {
        index = (index + 1) % TABLE_SIZE;  // переходим к следующей ячейке
        if (index == originalIndex) {
            return -1;  // таблица полностью заполнена
        }
    }
    return index;
}

// Добавление токена в таблицу
int HashTable::insert(const Token& token) {
    // Проверяем, не переполнена ли таблица
    if (currentIndex >= TABLE_SIZE) {
        return -1;
    }

    // Ищем подходящую ячейку
    int index = findEmptySlot(token.value);
    if (index == -1) {
        return -1;  // не нашли свободное место
    }

    // Если ячейка свободна, добавляем токен
    if (!table[index].occupied) {
        table[index].token = token;      // сохраняем токен
        table[index].index = currentIndex; // присваиваем индекс
        table[index].occupied = true;    // помечаем как занятую
        currentIndex++;                  // увеличиваем счетчик
    }

    return table[index].index;  // возвращаем индекс добавленного элемента
}

// Поиск токена по значению
int HashTable::find(const std::string& value) {
    int index = hashFunction(value);  // вычисляем начальный индекс
    int originalIndex = index;        // запоминаем начальный индекс

    // Ищем токен в таблице
    while (table[index].occupied) {
        if (table[index].token.value == value) {
            return table[index].index;  // нашли - возвращаем индекс
        }
        index = (index + 1) % TABLE_SIZE;  // переходим к следующей ячейке
        if (index == originalIndex) {
            break;  // прошли всю таблицу
        }
    }
    return -1;  // не нашли
}

// Вывод хеш-таблицы в файл
void HashTable::printToFile(const std::string& filename) {
    std::ofstream outFile(filename, std::ios::app);  // открываем файл для дополнения
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть выходной файл!" << std::endl;
        return;
    }

    outFile << "\nХеш-таблица:\n";
    outFile << "Тип лексемы | Лексема | Индекс в хеш-таблице\n";
    outFile << "--------------------------------------------\n";

    // Выводим все занятые ячейки
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (table[i].occupied) {
            outFile << static_cast<int>(table[i].token.type) << " | "
                << table[i].token.value << " | "
                << table[i].index << std::endl;
        }
    }

    outFile.close();
}