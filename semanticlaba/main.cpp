#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
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

    // ЛЕКСИЧЕСКИЙ АНАЛИЗ
    std::cout << "Лексический анализ..." << std::endl;
    Lexer lexer1(inputFile, outputFile);
    lexer1.analyze();

    if (lexer1.hasErrors()) {
        std::cout << "Лексический анализ завершен с ошибками." << std::endl;
    }
    else {
        std::cout << "Лексический анализ завершен успешно." << std::endl;
    }

    // СИНТАКСИЧЕСКИЙ АНАЛИЗ 
    std::cout << "Синтаксический анализ..." << std::endl;

    std::ofstream outFile(outputFile, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Ошибка открытия файла для записи результатов парсера" << std::endl;
        return 1;
    }

    // Создаем новый лексический анализатор для парсера
    Lexer parserLexer(inputFile, "temp.txt");
    Parser parser(parserLexer, outFile);
    bool parseSuccess = parser.parse();

    outFile.close();

    // СЕМАНТИЧЕСКИЙ АНАЛИЗ
    bool semanticSuccess = false;

    if (parseSuccess && !lexer1.hasErrors()) {
        std::cout << "Семантический анализ..." << std::endl;

        // Открываем файл для добавления результатов семантического анализа
        std::ofstream semOutFile(outputFile, std::ios::app);
        if (!semOutFile.is_open()) {
            std::cerr << "Ошибка открытия файла для записи результатов семантического анализа" << std::endl;
            return 1;
        }


        // Создаем новый лексический анализатор и парсер для семантического анализа
        Lexer semanticLexer(inputFile, "temp_sem.txt");

        // Создаем парсер, который сохранит дерево разбора
        std::ofstream parserTempOut("parser_sem_temp.txt");
        Parser semanticParser(semanticLexer, parserTempOut);

        // Разбираем программу и сохраняем дерево
        semanticParser.parseForSemantic();

        // Получаем дерево разбора
        ParseTreeNode* root = semanticParser.getParseTree();

        if (root) {
            // Создаем и запускаем семантический анализатор
            SemanticAnalyzer semanticAnalyzer(semOutFile);
            semanticSuccess = semanticAnalyzer.analyze(root);

            // Удаляем дерево разбора
            delete root;
        }
        else {
            semOutFile << "\nОшибка: не удалось построить дерево разбора для семантического анализа\n";
            semanticSuccess = false;
        }

        semOutFile.close();
        parserTempOut.close();

        // Удаляем временные файлы
        remove("temp.txt");
        remove("temp_sem.txt");
        remove("parser_sem_temp.txt");

        if (semanticSuccess) {
            std::cout << "Семантический анализ завершен успешно." << std::endl;
        }
        else {
            std::cout << "Семантический анализ завершен с ошибками." << std::endl;
        }
    }
    else {
        std::cout << "Семантический анализ пропущен из-за ошибок на предыдущих этапах." << std::endl;
    }

    // === ИТОГОВЫЙ РЕЗУЛЬТАТ ===
    std::cout << "\n=== ИТОГИ АНАЛИЗА ===" << std::endl;
    std::cout << "Лексический анализ: "
        << (lexer1.hasErrors() ? "ОШИБКИ" : "УСПЕХ") << std::endl;
    std::cout << "Синтаксический анализ: "
        << (parseSuccess ? "УСПЕХ" : "ОШИБКИ") << std::endl;
    std::cout << "Семантический анализ: "
        << (semanticSuccess ? "УСПЕХ" : "ОШИБКИ/ПРОПУЩЕН") << std::endl;

    bool overallSuccess = !lexer1.hasErrors() && parseSuccess && semanticSuccess;

    std::cout << "\nОБЩИЙ РЕЗУЛЬТАТ: "
        << (overallSuccess ? "АНАЛИЗ ПРОГРАММЫ УСПЕШЕН" : "АНАЛИЗ ЗАВЕРШЕН С ОШИБКАМИ")
        << std::endl;

    std::cout << "\nПодробные результаты сохранены в файле: " << outputFile << std::endl;

    system("pause");
    return overallSuccess ? 0 : 1;
}