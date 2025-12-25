#include "SemanticAnalyzer.h"
#include <iostream>
#include <sstream>
#include <iomanip>

SemanticAnalyzer::SemanticAnalyzer(std::ofstream& out)
    : outputFile(out), hasError(false), currentProcedure(""), labelCounter(1) {
}

void SemanticAnalyzer::error(const std::string& message, int line, int position) {
    std::stringstream errorMsg;
    errorMsg << "Строка " << line << ", позиция " << position
        << ": " << message;

    errorMessages.push_back(errorMsg.str());
    hasError = true;
}

void SemanticAnalyzer::printErrors() {
    if (!errorMessages.empty()) {
        outputFile << "\nСЕМАНТИЧЕСКИЕ ОШИБКИ (всего: " << errorMessages.size() << "):\n";
        outputFile << std::string(50, '-') << std::endl;
        for (size_t i = 0; i < errorMessages.size(); ++i) {
            outputFile << i + 1 << ". " << errorMessages[i] << std::endl;
        }
        outputFile << std::string(50, '-') << std::endl;
    }
}

void SemanticAnalyzer::checkVariableDeclaration(const std::string& varName, int line, int position) {
    if (globalSymbolTable.find(varName) != globalSymbolTable.end()) {
        error("Повторное объявление переменной '" + varName + "'", line, position);
    }
}

void SemanticAnalyzer::checkVariableUsage(const std::string& varName, int line, int position) {
    if (globalSymbolTable.find(varName) == globalSymbolTable.end()) {
        error("Использование необъявленной переменной '" + varName + "'", line, position);
    }
    else {
        usedVariables.insert(varName);
    }
}

void SemanticAnalyzer::checkTypeCompatibility(const std::string& expected, const std::string& actual,
    int line, int position, const std::string& context) {
    if (expected != actual) {
        error("Несовместимость типов: ожидается " + expected + ", получен " + actual +
            (context.empty() ? "" : " в " + context), line, position);
    }
}

bool SemanticAnalyzer::analyze(ParseTreeNode* root) {
    if (!root) return false;

    // 1. ПЕРВЫЙ ПРОХОД: сбор информации об объявлениях
    for (auto child : root->children) {
        if (child->name == "Descriptions") {
            traverseDescriptions(child);
        }
    }

    // Очищаем для второго прохода
    postfixCode.clear();
    labelCounter = 1;

    // 2. ВТОРОЙ ПРОХОД: проверка использования и генерация кода
    for (auto child : root->children) {
        if (child->name == "Operators") {
            traverseOperators(child);
        }
    }

    printErrors();

    outputFile << "\nПОСТФИКСНАЯ ЗАПИСЬ:\n";
    outputFile << std::string(40, '-') << std::endl;
    outputFile << postfixCode << std::endl;
    outputFile << std::string(40, '-') << std::endl;

    outputFile << "\nРЕЗУЛЬТАТ: ";
    if (hasError) {
        outputFile << "Обнаружены семантические ошибки\n";
    }
    else {
        outputFile << "Семантических ошибок не обнаружено\n";
    }

    return !hasError;
}

void SemanticAnalyzer::traverseProcedure(ParseTreeNode* node) {
 
}

void SemanticAnalyzer::traverseDescriptions(ParseTreeNode* node) {
    if (!node) return;

    for (auto child : node->children) {
        if (child->name == "DescrList") {
            for (auto descr : child->children) {
                if (descr->name == "Descr") {
                    traverseDescr(descr);
                }
            }
        }
    }
}

void SemanticAnalyzer::traverseDescr(ParseTreeNode* node) {
    if (!node) return;

    std::string type = "integer";

    for (auto child : node->children) {
        if (child->name == "VarList") {
            traverseVarList(child, type);
        }
    }
}

void SemanticAnalyzer::traverseVarList(ParseTreeNode* node, const std::string& type) {
    if (!node) return;

    for (auto child : node->children) {
        if (child->name == "id") {
            std::string varName = child->value;
            int line = child->line;
            int position = child->position;

            // ВАЖНО: сначала проверка, потом добавление
            checkVariableDeclaration(varName, line, position);

            // Добавляем только если не было ошибки
            VariableInfo varInfo(varName, type, line, position);
            globalSymbolTable[varName] = varInfo;
            declaredVariables.insert(varName);
        }
    }
}

void SemanticAnalyzer::traverseOperators(ParseTreeNode* node) {
    if (!node) return;

    for (auto child : node->children) {
        traverseOp(child);
    }
}

void SemanticAnalyzer::traverseOp(ParseTreeNode* node) {
    if (!node) return;

    if (node->name == "Assignment") {
        traverseAssignment(node);
    }
    else if (node->name == "IfStatement") {
        traverseIfStatement(node);
    }
}

void SemanticAnalyzer::traverseAssignment(ParseTreeNode* node) {
    if (!node) return;

    std::string leftVarName;
    std::string leftType;
    int assignLine = 0;
    int assignPos = 0;

    // 1. Левая часть
    for (auto child : node->children) {
        if (child->name == "id") {
            leftVarName = child->value;
            assignLine = child->line;
            assignPos = child->position;

            checkVariableUsage(leftVarName, assignLine, assignPos);

            if (globalSymbolTable.find(leftVarName) != globalSymbolTable.end()) {
                leftType = globalSymbolTable[leftVarName].type;
            }
            break;
        }
    }

    // 2. Правая часть
    for (auto child : node->children) {
        if (child->name == "Expr") {
            traverseExpr(child);

            if (!typeStack.empty()) {
                std::string rightType = typeStack.top();
                typeStack.pop();

                if (!leftType.empty()) {
                    checkTypeCompatibility(leftType, rightType, assignLine, assignPos,
                        "операторе присваивания");
                }
            }

            generatePostfixForExpr(child);
            postfixCode += leftVarName + " := ; ";
            break;
        }
    }
}

void SemanticAnalyzer::traverseIfStatement(ParseTreeNode* node) {
    if (!node) return;

    std::string elseLabel = "L" + std::to_string(labelCounter++);
    std::string endLabel = "L" + std::to_string(labelCounter++);

    // 1. Условие
    for (auto child : node->children) {
        if (child->name == "Condition") {
            generatePostfixForCondition(child);
            break;
        }
    }

    postfixCode += elseLabel + " JZ ";

    // 2. Ветка THEN
    bool foundThen = false;
    for (auto child : node->children) {
        if (child->name == "Assignment" && !foundThen) {
            traverseAssignment(child);
            foundThen = true;
        }
    }

    postfixCode += endLabel + " JMP ";
    postfixCode += elseLabel + ": ";

    // 3. Ветка ELSE
    bool inElse = false;
    for (auto child : node->children) {
        if (child->name == "keyword" && child->value == "else") {
            inElse = true;
        }
        else if (child->name == "Assignment" && inElse) {
            traverseAssignment(child);
            break;
        }
    }

    postfixCode += endLabel + ": ";
}

void SemanticAnalyzer::traverseExpr(ParseTreeNode* node) {
    if (!node) return;

    std::string exprType = "unknown";

    for (auto child : node->children) {
        if (child->name == "SimpleExpr") {
            traverseSimpleExpr(child);

            if (!typeStack.empty()) {
                exprType = typeStack.top();
                typeStack.pop();
            }
        }
        else if (child->name == "operator") {
            std::string op = child->value;

            for (auto grandchild : node->children) {
                if (grandchild->name == "Expr" && grandchild != child) {
                    traverseExpr(grandchild);

                    if (!typeStack.empty()) {
                        std::string rightType = typeStack.top();
                        typeStack.pop();

                        checkTypeCompatibility(exprType, rightType, child->line, child->position,
                            "операции " + op);

                        typeStack.push(exprType);
                    }
                    break;
                }
            }
        }
    }

    if (exprType == "unknown") {
        exprType = "integer";
    }

    typeStack.push(exprType);
}

void SemanticAnalyzer::traverseSimpleExpr(ParseTreeNode* node) {
    if (!node) return;

    for (auto child : node->children) {
        if (child->name == "id") {
            std::string varName = child->value;
            checkVariableUsage(varName, child->line, child->position);

            if (globalSymbolTable.find(varName) != globalSymbolTable.end()) {
                typeStack.push(globalSymbolTable[varName].type);
            }
            else {
                typeStack.push("integer");
            }
        }
        else if (child->name == "const") {
            typeStack.push("integer");
        }
        else if (child->name == "Expr") {
            traverseExpr(child);
        }
    }
}

void SemanticAnalyzer::traverseCondition(ParseTreeNode* node) {
    if (!node) return;

    std::string leftType, rightType;

    for (auto child : node->children) {
        if (child->name == "Expr") {
            traverseExpr(child);

            if (!typeStack.empty()) {
                if (leftType.empty()) {
                    leftType = typeStack.top();
                    typeStack.pop();
                }
                else {
                    rightType = typeStack.top();
                    typeStack.pop();
                }
            }
        }
        else if (child->name == "RelationOperator") {
            if (!leftType.empty() && !rightType.empty()) {
                checkTypeCompatibility(leftType, rightType, child->line, child->position,
                    "условии (оператор " + child->value + ")");
            }
        }
    }
}

void SemanticAnalyzer::generatePostfixForExpr(ParseTreeNode* node) {
    if (!node) return;

    for (auto child : node->children) {
        if (child->name == "SimpleExpr") {
            generatePostfixForSimpleExpr(child);
        }
    }

    for (auto child : node->children) {
        if (child->name == "operator") {
            std::string op = child->value;

            for (auto grandchild : node->children) {
                if (grandchild->name == "Expr" && grandchild != child) {
                    generatePostfixForExpr(grandchild);
                }
            }

            postfixCode += op + " ";
        }
    }
}

void SemanticAnalyzer::generatePostfixForSimpleExpr(ParseTreeNode* node) {
    if (!node) return;

    for (auto child : node->children) {
        if (child->name == "id" || child->name == "const") {
            postfixCode += child->value + " ";
        }
        else if (child->name == "Expr") {
            generatePostfixForExpr(child);
        }
    }
}

void SemanticAnalyzer::generatePostfixForCondition(ParseTreeNode* node) {
    if (!node) return;

    bool firstExpr = true;
    for (auto child : node->children) {
        if (child->name == "Expr") {
            generatePostfixForExpr(child);

            if (firstExpr) {
                firstExpr = false;
            }
        }
    }

    for (auto child : node->children) {
        if (child->name == "RelationOperator") {
            if (child->value == "<>") {
                postfixCode += "!= ";
            }
            else {
                postfixCode += child->value + " ";
            }
            break;
        }
    }
}