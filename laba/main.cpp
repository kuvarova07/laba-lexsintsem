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

    std::cout << "Zapusk analizatora..." << std::endl;

    std::ofstream clearFile(outputFile);
    clearFile.close();

    std::cout << "Lexical analysis..." << std::endl;
    Lexer lexer1(inputFile, outputFile);
    lexer1.analyze();

    if (lexer1.hasErrors()) {
        std::cout << "Lexical analysis completed with errors." << std::endl;
    }
    else {
        std::cout << "Lexical analysis completed successfully." << std::endl;
    }

    std::cout << "Syntactic analysis..." << std::endl;

    std::ofstream outFile(outputFile, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file to write parser results" << std::endl;
        return 1;
    }

    Lexer parserLexer(inputFile, "temp.txt");
    Parser parser(parserLexer, outFile);
    bool parseSuccess = parser.parse();

    outFile.close();

    bool semanticSuccess = false;

    if (parseSuccess && !lexer1.hasErrors()) {
        std::cout << "Semantic analysis..." << std::endl;

        std::ofstream semOutFile(outputFile, std::ios::app);
        if (!semOutFile.is_open()) {
            std::cerr << "Error opening file for writing semantic analysis results" << std::endl;
            return 1;
        }


        Lexer semanticLexer(inputFile, "temp_sem.txt");

        std::ofstream parserTempOut("parser_sem_temp.txt");
        Parser semanticParser(semanticLexer, parserTempOut);

        semanticParser.parseForSemantic();

        ParseTreeNode* root = semanticParser.getParseTree();

        if (root) {
            SemanticAnalyzer semanticAnalyzer(semOutFile);
            semanticSuccess = semanticAnalyzer.analyze(root);

            delete root;
        }
        else {
            semOutFile << "\nError: Failed to build parse tree for semantic analysis\n";
            semanticSuccess = false;
        }

        semOutFile.close();
        parserTempOut.close();

        remove("temp.txt");
        remove("temp_sem.txt");
        remove("parser_sem_temp.txt");

        if (semanticSuccess) {
            std::cout << "Semantic analysis completed successfully." << std::endl;
        }
        else {
            std::cout << "Semantic analysis completed with errors." << std::endl;
        }
    }
    else {
        std::cout << "Semantic analysis was skipped due to errors in previous stages." << std::endl;
    }

    std::cout << "\n=== Results of the analysis ===" << std::endl;
    std::cout << "Lexical analysis: "
        << (lexer1.hasErrors() ? "Errors" : "Success") << std::endl;
    std::cout << "Syntactic analysis: "
        << (parseSuccess ? "Success" : "Errors") << std::endl;
    std::cout << "Semantic analysis: "
        << (semanticSuccess ? "Success" : "Errors") << std::endl;

    bool overallSuccess = !lexer1.hasErrors() && parseSuccess && semanticSuccess;

    std::cout << "\nOverall result: "
        << (overallSuccess ? "The program analysis is successful" : "The analysis was completed with errors")
        << std::endl;

    std::cout << "\nDetailed results are saved in a file: " << outputFile << std::endl;

    system("pause");
    return overallSuccess ? 0 : 1;
}