#include "Lexer.h"
#include <iostream>
#include <windows.h>

int main() {
    SetConsoleOutputCP(1251);
    std::string inputFile = "input.txt";
    std::string outputFile = "output.txt";

    // Создаем лексический анализатор
    Lexer lexer(inputFile, outputFile);

    // Запускаем анализ
    lexer.analyze();

    if (lexer.hasErrors()) {
        std::cout << "Лексический анализ завершен с ошибками. Проверьте выходной файл." << std::endl;
    }
    else {
        std::cout << "Лексический анализ успешно завершен." << std::endl;
    }

    return 0;
}