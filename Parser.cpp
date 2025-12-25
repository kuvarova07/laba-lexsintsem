#include "Parser.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// Конструктор парсера
Parser::Parser(Lexer& lex, std::ofstream& out)
    : lexer(lex), outputFile(out), hasError(false) {
    advanceToken();  // Загружаем первый токен
}

// Переход к следующему токену
void Parser::advanceToken() {
    currentToken = lexer.getNextToken(); // Получаем следующий токен от лексера
}

// Проверка соответствия текущего токена ожидаемому типу (используется для проверки, является ли текущий токен тем, что мы ожидаем по грамматике)
bool Parser::match(enum TokenType expectedType) {
    return currentToken.type == expectedType; // Просто сравниваем типы
}

// Обработка синтаксической ошибки
void Parser::error(const std::string& message) {
    std::stringstream errorMsg;
    errorMsg << "Строка " << currentToken.line << ", позиция " << currentToken.position
        << ": " << message << " (получен '" << currentToken.value << "')";

    // Добавляем ошибку в список и устанавливаем флаг наличия ошибок
    errorMessages.push_back(errorMsg.str());
    hasError = true; // Парсер теперь знает, что есть хотя бы одна ошибка
}

// Синхронизация после ошибки - пропуск токенов до одного из синхронизирующих
void Parser::syncTo(const std::vector<TokenType>& syncTokens) {
    int skipped = 0; // Счетчик пропущенных токенов для отладочной информации
    // Пропускаем токены пока не дойдем до конца файла
    while (currentToken.type != TokenType::END_OF_FILE) {
        // Проверяем, является ли текущий токен одним из синхронизирующих
        for (TokenType syncToken : syncTokens) {
            if (match(syncToken)) {
                // Если пропустили хотя бы один токен, выводим отладочную информацию
                if (skipped > 0) {
                    outputFile << "  [Восстановление: пропущено " << skipped << " токенов]" << std::endl;
                }
                return; // Нашли синхронизирующий токен - выходим
            }
        }
        // Текущий токен не синхронизирующий - пропускаем его
        skipped++;
        advanceToken();
    }
}

// Рекурсивный вывод дерева разбора с отступами
void Parser::printTree(ParseTreeNode* node, int depth) {
    if (!node) return; // Базовый случай рекурсии - пустой узел

    // Создаем отступ пропорционально глубине узла в дереве (каждый уровень вложенности добавляет 2 пробела)
    for (int i = 0; i < depth; i++) {
        outputFile << "  ";
    }

    // Вывод узла в формате: ИмяУзла [значение] или просто ИмяУзла (для терминальных узлов (имеющих значение) выводим и имя и значение)
    // Для нетерминальных узлов выводим только имя
    if (!node->value.empty()) {
        outputFile << node->name << " [" << node->value << "]";
    }
    else {
        outputFile << node->name;
    }

    outputFile << std::endl;

    // Каждый ребенок выводится с увеличенной глубиной (depth + 1)
    for (auto child : node->children) {
        printTree(child, depth + 1);
    }
}

// Вывод списка всех обнаруженных синтаксических ошибок
void Parser::printErrors() {
    if (!errorMessages.empty()) {
        outputFile << "\nОБНАРУЖЕННЫЕ ОШИБКИ (всего: " << errorMessages.size() << "):\n";
        outputFile << std::string(50, '-') << std::endl;
        // Нумерованный список всех ошибок
        for (size_t i = 0; i < errorMessages.size(); ++i) {
            outputFile << i + 1 << ". " << errorMessages[i] << std::endl;
        }
        outputFile << std::string(50, '-') << std::endl;
    }
}

// Procedure -> Begin Descriptions Operators End (корневое правило, с которого начинается разбор всей программы)
ParseTreeNode* Parser::parseProcedure() {
    // Создаем корневой узел для всей процедуры
    ParseTreeNode* node = new ParseTreeNode("Procedure");

    // Begin
    if (match(TokenType::PROCEDURE)) { // Проверяем, что текущий токен - ключевое слово 'procedure'
        // Вызываем метод для разбора блока Begin
        ParseTreeNode* beginNode = parseBegin();
        if (beginNode) {
            // Если разбор успешен, добавляем узел Begin как ребенка
            node->children.push_back(beginNode);
        }
    }
    else {
        // Ошибка: программа должна начинаться с 'procedure'
        error("Ожидается 'procedure'");
        return node; // Возвращаем то, что успели разобрать
    }

    // Descriptions (опционально)
    if (match(TokenType::VAR)) { // Проверяем, есть ли ключевое слово 'var' (начало объявлений)
        ParseTreeNode* descrNode = parseDescriptions();
        if (descrNode) {
            node->children.push_back(descrNode);
        }
    }

    // begin (разбор начала блока операторов)
    if (match(TokenType::BEGIN)) {
        // Создаем узел для терминала 'begin'
        node->children.push_back(new ParseTreeNode("keyword", "begin", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему токену после 'begin'
    }
    else {
        // Ошибка: после объявлений должен быть 'begin'
        error("Ожидается 'begin'");
        // Пытаемся восстановиться - ищем начало операторов или конец
        syncTo({ TokenType::ID, TokenType::IF, TokenType::END });
    }

    // Operators
    ParseTreeNode* operatorsNode = parseOperators();
    if (operatorsNode) {
        node->children.push_back(operatorsNode);
    }

    // End
    if (match(TokenType::END)) {
        node->children.push_back(new ParseTreeNode("End", "end", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему токену после 'end'
    }
    else {
        error("Ожидается 'end'"); // Ошибка: процедура должна заканчиваться 'end'
    }

    return node; // Возвращаем полное дерево разбора процедуры
}

// Begin -> procedure ProcedureName ;  (разбор заголовка процедуры: procedure ИмяПроцедуры)
ParseTreeNode* Parser::parseBegin() {
    ParseTreeNode* node = new ParseTreeNode("Begin");

    // procedure (уже проверено)
    node->children.push_back(new ParseTreeNode("keyword", "procedure", currentToken.line, currentToken.position));
    advanceToken(); // Переходим к следующему токену (должен быть идентификатор)

    // ProcedureName (должно быть идентификатором)
    if (match(TokenType::ID)) {
        node->children.push_back(new ParseTreeNode("ProcedureName", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему токену (должна быть ;)
    }
    else {
        error("Ожидается имя процедуры");
    }

    // ;
    if (match(TokenType::SEMICOLON)) {
        node->children.push_back(new ParseTreeNode("semicolon", ";", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему токену
    }
    else {
        error("Ожидается ';' после имени процедуры");
    }

    return node;
}

// Descriptions -> var DescrList (разбор раздела объявлений переменных: var список объявлений)
ParseTreeNode* Parser::parseDescriptions() {
    ParseTreeNode* node = new ParseTreeNode("Descriptions");
    node->children.push_back(new ParseTreeNode("keyword", "var", currentToken.line, currentToken.position)); // Ключевое слово 'var'
    advanceToken(); // Переходим к следующему токену (должен быть первый идентификатор)

    // Разбор списка объявлений переменных
    ParseTreeNode* descrListNode = parseDescrList();
    if (descrListNode) {
        node->children.push_back(descrListNode);
    }

    return node;
}

// DescrList -> Descr | Descr DescrList (разбор списка объявлений: одно или несколько объявлений подряд)
ParseTreeNode* Parser::parseDescrList() {
    ParseTreeNode* node = new ParseTreeNode("DescrList");

    // Обрабатываем все объявления переменных пока они есть
    while (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Идентификатор") != std::string::npos)) {
        // Пытаемся разобрать одно объявление
        ParseTreeNode* descrNode = parseDescr();
        if (descrNode) {
            // Если объявление разобрано успешно, добавляем его в список
            node->children.push_back(descrNode);
        }
        else {
            // Если не удалось разобрать объявление, выходим из цикла
            break;
        }
    }

    return node;
}

// Descr -> VarList : Type ; (разбор одного объявления: список переменных)
ParseTreeNode* Parser::parseDescr() {
    ParseTreeNode* node = new ParseTreeNode("Descr");

    // Разбор списка переменных (например: a, b, c)
    ParseTreeNode* varListNode = parseVarList();
    if (varListNode) {
        node->children.push_back(varListNode);
    }
    else { // Ошибка в списке переменных - удаляем узел и возвращаем nullptr
        delete node;
        return nullptr;
    }

    // :
    if (match(TokenType::COLON)) {
        node->children.push_back(new ParseTreeNode("colon", ":", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к типу
    }
    else {
        error("Ожидается ':' после списка переменных");
        delete node;
        return nullptr;
    }

    // Разбор типа переменных 
    if (match(TokenType::INTEGER)) {
        node->children.push_back(new ParseTreeNode("Type", "integer", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к точке с запятой
    }
    else {
        error("Ожидается 'integer'");
        delete node;
        return nullptr;
    }

    // ;
    if (match(TokenType::SEMICOLON)) {
        node->children.push_back(new ParseTreeNode("semicolon", ";", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему объявлению или операторам
    }
    else {
        error("Ожидается ';' после типа");
        delete node;
        return nullptr;
    }

    return node;
}

// VarList -> Id | Id , VarList (разбор списка переменных в объявлении: идентификатор или несколько через запятую)
ParseTreeNode* Parser::parseVarList() {
    ParseTreeNode* node = new ParseTreeNode("VarList");

    // Первый идентификатор в списке
    if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Идентификатор") != std::string::npos)) {
        node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему токену (запятая или двоеточие)
    }
    else {
        error("Ожидается идентификатор в списке переменных");
        delete node;
        return nullptr; // Нельзя разобрать список без первого идентификатора
    }

    // Дополнительные идентификаторы через запятые (ноль или более)
    while (match(TokenType::COMMA)) {
        // Запятая между идентификаторами
        node->children.push_back(new ParseTreeNode("comma", ",", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему идентификатору

        // Идентификатор после запятой
        if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Идентификатор") != std::string::npos)) {
            node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
            advanceToken(); // Переходим к следующему токену
        }
        else {
            error("Ожидается идентификатор после запятой");
            break; // Выходим из цикла, но не прерываем весь разбор
        }
    }

    return node;
}

// Operators -> Op | Op Operators (разбор блока операторов: один или несколько операторов подряд)
ParseTreeNode* Parser::parseOperators() {
    ParseTreeNode* node = new ParseTreeNode("Operators");

    // Обрабатываем все операторы до конца блока (пока не встретим 'end') или до конца файла
    while (!match(TokenType::END) && currentToken.type != TokenType::END_OF_FILE) {
        // Пытаемся разобрать один оператор
        ParseTreeNode* opNode = parseOp();
        if (opNode) {
            // Если оператор разобран успешно, добавляем его в блок
            node->children.push_back(opNode);
        }
        else {
            // Если не удалось разобрать оператор (синтаксическая ошибка), используем механизм восстановления после ошибок
            outputFile << "  [Восстановление: пропущен токен '" << currentToken.value << "']" << std::endl;
            advanceToken();
            
            // Пропускаем токены до одного из синхронизирующих
            syncTo({ TokenType::SEMICOLON, TokenType::END, TokenType::IF, TokenType::ID });

            // Если нашли точку с запятой, пропускаем ее
            if (match(TokenType::SEMICOLON)) {
                outputFile << "  [Пропущена ';']" << std::endl;
                advanceToken();
            }

            // Если дошли до конца блока, выходим из цикла
            if (match(TokenType::END)) {
                break;
            }
            
        }
    }

    return node;
}


// Op ? Id := Expr ; | if Condition then Op | if ( Condition ) then Op else Op (разбор одного оператора: может быть присваиванием или условным оператором)
ParseTreeNode* Parser::parseOp() {
    ParseTreeNode* opNode = nullptr;

    // Оператор присваивания (начинается с идентификатора)
    if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Идентификатор") != std::string::npos)) {
        // Разбираем присваивание: идентификатор := выражение
        opNode = parseAssignment();

        // После присваивания проверяем точку с запятой
        if (opNode) {
            // Если есть точка с запятой, просто пропускаем ее
            if (match(TokenType::SEMICOLON)) {
                advanceToken();  // Пропускаем ;
            }
            // Если точки с запятой нет, но следующий токен - не конец блока и не else, то это ошибка (в большинстве случаев после присваивания нужна ;)
            else if (!match(TokenType::END) && !match(TokenType::ELSE)) {
                error("Ожидается ';' после присваивания");
                
                // Восстанавливаемся - ищем место для продолжения разбора
                syncTo({ TokenType::SEMICOLON, TokenType::ID, TokenType::IF, TokenType::END });
                // Если нашли точку с запятой, пропускаем ее
                if (match(TokenType::SEMICOLON)) {
                    advanceToken();
                }
                
                if (!match(TokenType::ID) && !match(TokenType::IF)) {
                    syncTo({ TokenType::SEMICOLON, TokenType::ID, TokenType::IF, TokenType::END });
                }
            }
            // Если следующий токен - end или else, точка с запятой не требуется
        }
    }
    // Условный оператор (начинается с if)
    else if (match(TokenType::IF)) {
        opNode = parseIfStatement();
        // Для условного оператора точка с запятой не проверяется здесь (она может проверяться внутри, для операторов в ветках then/else)
    }
    // Не оператор (что то непонятное)
    else {
        return nullptr; // Это не оператор - возвращаем nullptr
    }

    return opNode;
}

void Parser::checkSemicolon() {
    if (!match(TokenType::END) && !match(TokenType::ELSE) && !match(TokenType::THEN)) {
        if (match(TokenType::SEMICOLON)) {
            advanceToken(); // пропускаем ;
        }
        else {
            error("Ожидается ';' после присваивания");
        }
    }
}

// Присваивание: Id := Expr (разбор оператора присваивания: переменная := выражение)
ParseTreeNode* Parser::parseAssignment() {
    ParseTreeNode* node = new ParseTreeNode("Assignment");

    // Левая часть - идентификатор (имя переменной)
    node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
    advanceToken(); // Переходим к оператору присваивания

    // Оператор присваивания := 
    if (match(TokenType::ASSIGN)) {
        node->children.push_back(new ParseTreeNode("assign", ":=", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к выражению
    }
    else {
        error("Ожидается ':='");
        delete node; // Освобождаем память
        return nullptr; // Нельзя разобрать присваивание без :=
    }

    // Правая часть - выражение (то, что присваивается)
    ParseTreeNode* exprNode = parseExpr();
    if (exprNode) {
        node->children.push_back(exprNode);
    }
    else {
        error("Ожидается выражение после ':='");
        delete node;
        return nullptr; // Нельзя разобрать присваивание без выражения
    }

    return node;  // Возвращаем узел присваивания
}


// Условный оператор: if Condition then Op [else Op] (разбор условного оператора: if условие then оператор [else оператор])
ParseTreeNode* Parser::parseIfStatement() {
    ParseTreeNode* node = new ParseTreeNode("IfStatement");

    // Ключевое слово if
    node->children.push_back(new ParseTreeNode("keyword", "if", currentToken.line, currentToken.position));
    advanceToken(); // Переходим к условию

    // Условие (выражение с оператором отношения)
    ParseTreeNode* conditionNode = parseCondition();
    if (conditionNode) {
        node->children.push_back(conditionNode);
    }
    else {
        error("Ожидается условие после 'if'");
        delete node;
        return nullptr; // Нельзя разобрать if без условия
    }

    // Ключевое слово then
    if (match(TokenType::THEN)) {
        node->children.push_back(new ParseTreeNode("keyword", "then", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к телу then
    }
    else {
        error("Ожидается 'then'");
        // не удаляем узел, пытаемся восстановиться и продолжить (это позволяет разобрать тело then даже при отсутствии then)
        syncTo({ TokenType::ID, TokenType::IF, TokenType::END, TokenType::SEMICOLON });
    }

    // Тело then - оператор, выполняемый если условие истинно
    ParseTreeNode* thenOpNode = parseOp();
    if (thenOpNode) {
        node->children.push_back(thenOpNode);
    }
    // Если не удалось разобрать тело then, продолжаем (может быть пустой оператор)

    // Ветка else 
    if (match(TokenType::ELSE)) {
        node->children.push_back(new ParseTreeNode("keyword", "else", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к телу else

        // Тело else
        ParseTreeNode* elseOpNode = parseOp();
        if (elseOpNode) {
            node->children.push_back(elseOpNode);
        }
    }

    return node; // Возвращаем узел условного оператора
}

// Expr ? SimpleExpr | SimpleExpr + Expr | SimpleExpr - Expr (разбор выражения: простое выражение или выражение с операциями +/-)
ParseTreeNode* Parser::parseExpr() {
    ParseTreeNode* node = new ParseTreeNode("Expr");

    // Разбор простого выражения (обязательная часть)
    ParseTreeNode* simpleExprNode = parseSimpleExpr();
    if (simpleExprNode) {
        node->children.push_back(simpleExprNode);
    }
    else {
        // Не удалось разобрать простое выражение - ошибка
        delete node;
        return nullptr;
    }

    // Опциональная часть: операция + или - и продолжение выражения
    if (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        // Сохраняем оператор (+, -)
        std::string op = currentToken.value;
        node->children.push_back(new ParseTreeNode("operator", op, currentToken.line, currentToken.position));
        advanceToken(); // Переходим к правой части

        // Рекурсивно разбираем правую часть выражения
        ParseTreeNode* exprNode = parseExpr();
        if (exprNode) {
            node->children.push_back(exprNode);
        }
        // Если не удалось разобрать правую часть, продолжаем с тем что есть
    }

    return node; // Возвращаем узел выражения
}

// SimpleExpr ? Id | Const | ( Expr ) (разбор простого выражения: идентификатор, константа или выражение в скобках)
ParseTreeNode* Parser::parseSimpleExpr() {
    ParseTreeNode* node = new ParseTreeNode("SimpleExpr");

    // Идентификатор (переменная)
    if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Идентификатор") != std::string::npos)) {
        node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); // Переходим к следующему токену
    }
    // Константа (число)
    else if (match(TokenType::CONST)) {
        node->children.push_back(new ParseTreeNode("const", currentToken.value, currentToken.line, currentToken.position));
        advanceToken();
    }
    // Выражение в скобках
    else if (match(TokenType::LPAREN)) {
        // Открывающая скобка
        node->children.push_back(new ParseTreeNode("lparen", "(", currentToken.line, currentToken.position));
        advanceToken(); // Переходим к выражению внутри скобок

        // Выражение внутри скобок
        ParseTreeNode* exprNode = parseExpr();
        if (exprNode) {
            node->children.push_back(exprNode);
        }
        else {
            // Не удалось разобрать выражение в скобках
            delete node;
            return nullptr;
        }

        // Закрывающая скобка 
        if (match(TokenType::RPAREN)) {
            node->children.push_back(new ParseTreeNode("rparen", ")", currentToken.line, currentToken.position));
            advanceToken(); // Переходим к следующему токену
        }
        else {
            error("Ожидается ')'");
            delete node;
            return nullptr;
        }
    }
    // Ни один из вариантов не подошел - ошибка
    else {
        error("Ожидается идентификатор, константа или выражение в скобках");
        delete node;
        return nullptr;
    }

    return node; // Возвращаем узел простого выражения
}

// Condition ? Expr RelationOperator Expr (разбор условия: выражение оператор_отношения выражение)
ParseTreeNode* Parser::parseCondition() {
    ParseTreeNode* node = new ParseTreeNode("Condition");

    // Левое выражение
    ParseTreeNode* leftExpr = parseExpr();
    if (leftExpr) {
        node->children.push_back(leftExpr);
    }
    else {
        delete node;
        return nullptr; // Нельзя разобрать условие без левого выражения
    }

    // Оператор отношения (=, <>, >, <)
    if (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL) ||
        match(TokenType::GREATER) || match(TokenType::LESS)) {
        node->children.push_back(new ParseTreeNode("RelationOperator", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); // Переходим к правому выражению
    }
    else {
        error("Ожидается оператор отношения (=, <>, >, <)");
        delete node;
        return nullptr; // Нельзя разобрать условие без оператора
    }

    // Правое выражение
    ParseTreeNode* rightExpr = parseExpr();
    if (rightExpr) {
        node->children.push_back(rightExpr);
    }
    else {
        error("Ожидается выражение");
        delete node;
        return nullptr; // Нельзя разобрать условие без правого выражения
    }

    return node; // Возвращаем узел условия
}

// Основной метод синтаксического анализа (запускает разбор всей программы и выводит результаты)
bool Parser::parse() {
    // Разбор всей процедуры (строим дерево разбора)
    ParseTreeNode* root = parseProcedure();

    // Вывод всех обнаруженных ошибок (если есть)
    printErrors();

    // Вывод дерева разбора (если удалось что-то разобрать)
    if (root) {
        outputFile << "\nДЕРЕВО РАЗБОРА:\n";
        outputFile << std::string(30, '-') << std::endl;
        printTree(root); // Рекурсивный вывод дерева
        outputFile << std::string(30, '-') << std::endl;
        delete root;  // освобождаем память, выделенную для дерева
    }

    // Итоговый результат анализа
    outputFile << "\nРЕЗУЛЬТАТ: ";
    if (hasError) {
        outputFile << "Обнаружены синтаксические ошибки\n";
    }
    else {
        outputFile << "Синтаксических ошибок не обнаружено\n";
    }

    // Возвращаем true если ошибок нет, false если есть ошибки
    return !hasError;
}