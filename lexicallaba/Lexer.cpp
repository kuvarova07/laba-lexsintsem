#include "Lexer.h"
#include <cctype>
#include <iostream>

// Конструктор - открывает файлы и инициализирует состояние
Lexer::Lexer(const std::string& inputFilename, const std::string& outputFilename)
    : line(1), position(1), hasError(false) {

    // Открываем входной файл
    inputFile.open(inputFilename);
    if (!inputFile.is_open()) {
        std::cerr << "Не удалось открыть входной файл: " << inputFilename << std::endl;
        hasError = true;
    }

    // Открываем выходной файл
    outputFile.open(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Не удалось открыть выходной файл: " << outputFilename << std::endl;
        hasError = true;
    }

    // Читаем первый символ, если файл открыт
    if (inputFile.is_open()) {
        inputFile.get(currentChar);
    }
}

Lexer::~Lexer() {
    if (inputFile.is_open()) inputFile.close();
    if (outputFile.is_open()) outputFile.close();
}

// Чтение следующего символа из файла
void Lexer::nextChar() {
    if (inputFile.get(currentChar)) {
        position++;  // увеличиваем позицию
    }
    else {
        currentChar = '\0';  // конец файла
    }
}

// Пропуск пробелов, табуляций и переводов строк
void Lexer::skipWhitespace() {
    while (std::isspace(currentChar)) {
        if (currentChar == '\n') {
            line++;     // увеличиваем номер строки
            position = 1; // сбрасываем позицию
        }
        nextChar();  // читаем следующий символ
    }
}

// Разбор идентификатора или ключевого слова
Token Lexer::parseIdentifier() {
    std::string value;        // накапливаем значение идентификатора
    int startLine = line;     // запоминаем начало идентификатора
    int startPos = position;  // запоминаем позицию начала

    // Первый символ должен быть только буквой
    if (std::isalpha(currentChar)) {
        value += currentChar;
        nextChar();
    }
    else {
        std::string errorChar(1, currentChar);
        nextChar();
        Token errorToken(TokenType::ERROR, errorChar, startLine, startPos);
        errorToken.errorMessage = "Идентификатор должен начинаться с буквы";
        return errorToken;
    }

    // Читаем остальные буквы
    while (std::isalpha(currentChar)) {
        value += currentChar;  // добавляем символ к значению
        nextChar();           // читаем следующий символ
    }

    // Если после букв идет цифра - это ошибка (идентификатор с цифрами)
    if (std::isdigit(currentChar)) {
        // Собираем весь идентификатор с цифрами для сообщения об ошибке
        while (std::isalnum(currentChar)) {
            value += currentChar;
            nextChar();
        }
        Token errorToken(TokenType::ERROR, value, startLine, startPos);
        errorToken.errorMessage = "Идентификатор не может содержать цифры";
        return errorToken;
    }

    // Если после букв идет _ - это ошибка
    if (currentChar == '_') {
        value += currentChar;
        nextChar();
        Token errorToken(TokenType::ERROR, value, startLine, startPos);
        errorToken.errorMessage = "Идентификатор не может содержать символ '_'";
        return errorToken;
    }

    // Проверяем, не является ли идентификатор ключевым словом
    if (value == "procedure") return Token(TokenType::PROCEDURE, value, startLine, startPos);
    if (value == "begin") return Token(TokenType::BEGIN, value, startLine, startPos);
    if (value == "end") return Token(TokenType::END, value, startLine, startPos);
    if (value == "var") return Token(TokenType::VAR, value, startLine, startPos);
    if (value == "integer") return Token(TokenType::INTEGER, value, startLine, startPos);
    if (value == "if") return Token(TokenType::IF, value, startLine, startPos);
    if (value == "then") return Token(TokenType::THEN, value, startLine, startPos);
    if (value == "else") return Token(TokenType::ELSE, value, startLine, startPos);

    // Если не ключевое слово, то обычный идентификатор
    return Token(TokenType::ID, value, startLine, startPos);
}

// Разбор числовой константы
Token Lexer::parseNumber() {
    std::string value;        // накапливаем число
    int startLine = line;     // запоминаем начало числа
    int startPos = position;  // запоминаем позицию начала

    // Собираем все цифры, пока они идут подряд
    while (std::isdigit(currentChar)) {
        value += currentChar;  // добавляем цифру
        nextChar();           // читаем следующую цифру
    }

    // Проверяем, что число не начинается с 0 (кроме самого 0)
    if (value.length() > 1 && value[0] == '0') {
        Token errorToken(TokenType::ERROR, value, startLine, startPos);
        errorToken.errorMessage = "Число не может начинаться с 0";
        return errorToken;
    }

    return Token(TokenType::CONST, value, startLine, startPos);
}

// Разбор операторов и разделителей
Token Lexer::parseOperator() {
    int startLine = line;     // запоминаем начало оператора
    int startPos = position;  // запоминаем позицию начала
    Token token;              // создаем токен для результата

    // Анализируем текущий символ
    switch (currentChar) {
    case ':':  // может быть : или :=
        nextChar();  // читаем следующий символ
        if (currentChar == '=') {
            nextChar();  // читаем еще один символ
            token = Token(TokenType::ASSIGN, ":=", startLine, startPos);
        }
        else {
            token = Token(TokenType::COLON, ":", startLine, startPos);
        }
        break;

    case ';':  // точка с запятой
        nextChar();
        token = Token(TokenType::SEMICOLON, ";", startLine, startPos);
        break;
    case ',':  // запятая
        nextChar();
        token = Token(TokenType::COMMA, ",", startLine, startPos);
        break;
    case '(':  // левая скобка
        nextChar();
        token = Token(TokenType::LPAREN, "(", startLine, startPos);
        break;
    case ')':  // правая скобка
        nextChar();
        token = Token(TokenType::RPAREN, ")", startLine, startPos);
        break;
    case '+':  // плюс
        nextChar();
        token = Token(TokenType::PLUS, "+", startLine, startPos);
        break;
    case '-':  // минус
        nextChar();
        token = Token(TokenType::MINUS, "-", startLine, startPos);
        break;
    case '=':  // равно
        nextChar();
        token = Token(TokenType::EQUAL, "=", startLine, startPos);
        break;
    case '>':  // больше
        nextChar();
        token = Token(TokenType::GREATER, ">", startLine, startPos);
        break;
    case '<':  // может быть < или <>
        nextChar();
        if (currentChar == '>') {
            nextChar();
            token = Token(TokenType::NOT_EQUAL, "<>", startLine, startPos);
        }
        else {
            token = Token(TokenType::LESS, "<", startLine, startPos);
        }
        break;

    default:  // недопустимый символ
        std::string errorChar(1, currentChar);  // создаем строку из одного символа
        nextChar();
        token = Token(TokenType::ERROR, errorChar, startLine, startPos);
        break;
    }

    return token;
}

// Преобразование типа токена в строку для вывода
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

// Основной метод - получение следующего токена
Token Lexer::getNextToken() {
    skipWhitespace();  // пропускаем пробелы

    // Проверяем конец файла
    if (currentChar == '\0') {
        return Token(TokenType::END_OF_FILE, "", line, position);
    }

    // Определяем тип токена по первому символу
    if (std::isalpha(currentChar) || currentChar == '_') {
        return parseIdentifier();  // идентификатор или ключевое слово
    }

    if (std::isdigit(currentChar)) {
        return parseNumber();      // числовая константа
    }

    return parseOperator();        // оператор или разделитель
}

// Главный метод анализа - обрабатывает весь файл
void Lexer::analyze() {
    if (!outputFile.is_open()) return; // проверяем, что файл открыт

    // Заголовок выходного файла
    outputFile << "Результаты лексического анализа:\n";
    outputFile << "Строка | Позиция | Тип | Лексема\n";
    outputFile << "---------------------------------\n";

    Token token; // переменная для хранения текущего токена
    // Основной цикл анализа - пока не достигнем конца файла
    do {
        token = getNextToken(); // получаем следующий токен

        if (token.type != TokenType::END_OF_FILE) {
            outputFile << token.line << " | " << token.position << " | "
                << tokenTypeToString(token.type) << " | " << token.value << std::endl;
        }

        // Обрабатываем ошибки
        if (token.type == TokenType::ERROR) {
            outputFile << "ОШИБКА в строке " << token.line << ", позиция " << token.position
                << ": " << token.value;
            // Добавляем конкретное сообщение об ошибке, если есть
            if (!token.errorMessage.empty()) {
                outputFile << " - " << token.errorMessage;
            }
            else {
                outputFile << " - Недопустимый символ";
            }
            outputFile << std::endl;
            hasError = true;
        }
        else if (token.type != TokenType::END_OF_FILE) {
            // Добавляем токен в хеш-таблицу (кроме конца файла и ошибок)
            hashTable.insert(token);
        }

    } while (token.type != TokenType::END_OF_FILE); // пока не конец файла

    hashTable.printToFile("output.txt"); // Выводим хеш-таблицу в файл
}