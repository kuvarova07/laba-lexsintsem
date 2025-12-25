#include "Lexer.h"
#include "Parser.h"
#include <iostream>
#include <windows.h>
#include <fstream>

int main() {
    SetConsoleOutputCP(1251);

    std::string inputFile = "input.txt";
    std::string outputFile = "output.txt";

    std::cout << "Запуск анализатора..." << std::endl;

    // Очищаем выходной файл
    std::ofstream clearFile(outputFile);
    clearFile.close();

    // === ЛЕКСИЧЕСКИЙ АНАЛИЗ ===
    std::cout << "Лексический анализ..." << std::endl;
    Lexer lexer1(inputFile, outputFile);  // переименовал в lexer1
    lexer1.analyze();

    if (lexer1.hasErrors()) {
        std::cout << "Лексический анализ завершен с ошибками." << std::endl;
    }
    else {
        std::cout << "Лексический анализ завершен успешно." << std::endl;
    }

    // === СИНТАКСИЧЕСКИЙ АНАЛИЗ ===  
    std::cout << "Синтаксический анализ..." << std::endl;

    std::ofstream outFile(outputFile, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Ошибка открытия файла для записи результатов парсера" << std::endl;
        return 1;
    }

    outFile << "\n" << std::string(50, '=') << std::endl;
    outFile << "СИНТАКСИЧЕСКИЙ АНАЛИЗ" << std::endl;
    outFile << std::string(50, '=') << std::endl;

    // Создаем новый лексический анализатор для парсера
    Lexer parserLexer(inputFile, "temp.txt");
    Parser parser(parserLexer, outFile);
    bool parseSuccess = parser.parse();

    outFile.close();

    if (parseSuccess && !lexer1.hasErrors()) {
        std::cout << "Анализ завершен успешно!" << std::endl;
    }
    else {
        std::cout << "Анализ завершен с ошибками." << std::endl;
    }

    system("pause");
    return (parseSuccess && !lexer1.hasErrors()) ? 0 : 1;
}