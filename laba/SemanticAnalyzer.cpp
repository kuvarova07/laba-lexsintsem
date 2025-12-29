#include "SemanticAnalyzer.h"
#include <iostream>
#include <sstream>
#include <iomanip>

StringStack::StringStack() : topIndex(-1) {}

void StringStack::push(const std::string& value) {
    if (topIndex < 99) {
        data[++topIndex] = value;
    }
}

std::string StringStack::pop() {
    if (topIndex >= 0) {
        return data[topIndex--];
    }
    return "";
}

std::string StringStack::top() const {
    if (topIndex >= 0) {
        return data[topIndex];
    }
    return "";
}

bool StringStack::empty() const {
    return topIndex == -1;
}

void StringStack::clear() {
    topIndex = -1;
}

StringSet::StringSet() : count(0) {}

void StringSet::insert(const std::string& value) {
    for (int i = 0; i < count; i++) {
        if (data[i] == value) return;
    }

    if (count < 100) {
        data[count++] = value;
    }
}

bool StringSet::contains(const std::string& value) const {
    for (int i = 0; i < count; i++) {
        if (data[i] == value) return true;
    }
    return false;
}

void StringSet::clear() {
    count = 0;
}

ErrorList::ErrorList() : errorCount(0) {}

void ErrorList::add(const std::string& error) {
    if (errorCount < 50) {
        errors[errorCount++] = error;
    }
}

int ErrorList::size() const {
    return errorCount;
}

std::string ErrorList::get(int index) const {
    if (index >= 0 && index < errorCount) {
        return errors[index];
    }
    return "";
}

void ErrorList::clear() {
    errorCount = 0;
}

SemanticAnalyzer::SemanticAnalyzer(std::ofstream& out)
    : outputFile(out), hasError(false), currentProcedure(""), labelCounter(1) {
}

std::string SemanticAnalyzer::getVariableType(const std::string& varName) {
    int index = symbolTable.find(varName);
    if (index != -1) {
        return "integer";
    }
    return "";
}

void SemanticAnalyzer::markVariableInitialized(const std::string& varName) {
}

void SemanticAnalyzer::error(const std::string& message, int line, int position) {
    std::stringstream errorMsg;
    errorMsg << "Stroka " << line << ", position " << position
        << ": " << message;
    errorMessages.add(errorMsg.str());
    hasError = true;
}

void SemanticAnalyzer::printErrors() {
    if (errorMessages.size() > 0) {
        outputFile << "\nSemanticheskiye oshibki (vsego: " << errorMessages.size() << "):\n";
        for (int i = 0; i < errorMessages.size(); ++i) {
            outputFile << i + 1 << ". " << errorMessages.get(i) << std::endl;
        }
    }
}

void SemanticAnalyzer::checkVariableUsage(const std::string& varName, int line, int position) {
    if (symbolTable.find(varName) == -1) {
        error("Using an undeclared variable '" + varName + "'", line, position);
    }
    else {
        usedVariables.insert(varName);
    }
}

void SemanticAnalyzer::checkTypeCompatibility(const std::string& expected,
    const std::string& actual,
    int line, int position,
    const std::string& context) {
    if (expected != actual) {
        error("Nesovmestimost' tipov: ozhidayetsya " + expected +
            ", poluchen " + actual + (context.empty() ? "" : " в " + context),
            line, position);
    }
}

bool SemanticAnalyzer::analyze(ParseTreeNode* root) {
    if (!root) return false;

    hasError = false;
    currentProcedure = "";
    labelCounter = 1;
    postfixCode.clear();

    usedVariables.clear();
    declaredVariables.clear();
    typeStack.clear();
    errorMessages.clear();

    for (size_t i = 0; i < root->children.size(); i++) {
        if (root->children[i]->name == "Begin") {
            ParseTreeNode* beginNode = root->children[i];
            for (size_t j = 0; j < beginNode->children.size(); j++) {
                if (beginNode->children[j]->name == "ProcedureName") {
                    postfixCode += "PROCEDURE " + beginNode->children[j]->value + " ; ";
                    currentProcedure = beginNode->children[j]->value;
                    break;
                }
            }
            break;
        }
    }

    bool foundDescriptions = false;
    for (size_t i = 0; i < root->children.size(); i++) {
        if (root->children[i]->name == "Descriptions") {
            foundDescriptions = true;
            postfixCode += "VAR ";
            traverseDescriptions(root->children[i]);
            break;
        }
    }

    if (!foundDescriptions) {
        postfixCode += "VAR ";
    }

    postfixCode += "BEGIN ";
    for (size_t i = 0; i < root->children.size(); i++) {
        if (root->children[i]->name == "Operators") {
            traverseOperators(root->children[i]);
        }
        else if (root->children[i]->name == "End") {
            postfixCode += "END ";
        }
    }

    checkUnusedVariables();

    printErrors();

    outputFile << "\nPostfix notation:\n";
    outputFile << postfixCode << std::endl;

    outputFile << "\nResult: ";
    if (hasError) {
        outputFile << "Semantic errors found\n";
    }
    else {
        outputFile << "No semantic errors were found\n";
    }

    symbolTable.printToFile("semantic_symbols.txt");

    return !hasError;
}

void SemanticAnalyzer::checkUnusedVariables() {
}

void SemanticAnalyzer::traverseDescriptions(ParseTreeNode* node) {
    if (!node) return;

    for (size_t i = 0; i < node->children.size(); i++) {
        if (node->children[i]->name == "DescrList") {
            ParseTreeNode* descrList = node->children[i];
            for (size_t j = 0; j < descrList->children.size(); j++) {
                if (descrList->children[j]->name == "Descr") {
                    traverseDescr(descrList->children[j]);
                    if (j < descrList->children.size() - 1) {
                        postfixCode += " ; ";
                    }
                }
            }
        }
    }
}

void SemanticAnalyzer::traverseDescr(ParseTreeNode* node) {
    if (!node) return;

    std::string type = "integer";
    bool foundType = false;

    for (size_t i = 0; i < node->children.size(); i++) {
        if (node->children[i]->name == "Type") {
            type = node->children[i]->value;
            foundType = true;
            break;
        }
    }

    for (size_t i = 0; i < node->children.size(); i++) {
        if (node->children[i]->name == "VarList") {
            traverseVarList(node->children[i], type);
            break;
        }
    }
}

void SemanticAnalyzer::traverseVarList(ParseTreeNode* node, const std::string& type) {
    if (!node) return;

    std::string vars[20];
    int lines[20];
    int positions[20];
    int varCount = 0;

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "id") {
            if (varCount < 20) {
                vars[varCount] = child->value;
                lines[varCount] = child->line;
                positions[varCount] = child->position;
                varCount++;
            }
        }
    }

    for (int i = 0; i < varCount; i++) {
        for (int j = i + 1; j < varCount; j++) {
            if (vars[i] == vars[j] && !vars[j].empty()) {
                error("Povtornaya peremennaya '" + vars[i] + "' v odnom obyavlenii",
                    lines[j], positions[j]);
                vars[j] = "";
            }
        }
    }

    bool firstVar = true;
    for (int i = 0; i < varCount; i++) {
        if (vars[i].empty()) continue;

        if (symbolTable.find(vars[i]) != -1) {
            error("Povtornoye obyavleniye peremennoy '" + vars[i] + "'",
                lines[i], positions[i]);
        }
        else {
            Token varToken(TokenType::ID, vars[i], lines[i], positions[i]);
            varToken.dataType = type;
            varToken.isInitialized = false;

            int index = symbolTable.insert(varToken);

            if (index == -1) {
                error("Failed to add variable '" + vars[i] + "' to the symbol table",
                    lines[i], positions[i]);
            }
            else {
                declaredVariables.insert(vars[i]);

                if (!firstVar) {
                    postfixCode += ", ";
                }
                firstVar = false;
                postfixCode += vars[i];
            }
        }
    }

    if (!firstVar) {
        postfixCode += " : " + type;
    }
}

void SemanticAnalyzer::traverseOperators(ParseTreeNode* node) {
    if (!node) return;

    for (size_t i = 0; i < node->children.size(); i++) {
        traverseOp(node->children[i]);
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
    int assignLine = 0;
    int assignPos = 0;

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "id") {
            leftVarName = child->value;
            assignLine = child->line;
            assignPos = child->position;

            checkVariableUsage(leftVarName, assignLine, assignPos);
            break;
        }
    }

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "Expr") {
            traverseExpr(child);

            if (!typeStack.empty()) {
                std::string rightType = typeStack.top();
                typeStack.pop();

                std::string leftType = getVariableType(leftVarName);
                if (!leftType.empty()) {
                    checkTypeCompatibility(leftType, rightType, assignLine, assignPos,
                        "assignment operator");
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

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "Condition") {
            generatePostfixForCondition(child);
            break;
        }
    }

    postfixCode += elseLabel + " JZ ";

    bool foundThen = false;
    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "Assignment" && !foundThen) {
            traverseAssignment(child);
            foundThen = true;
        }
    }

    postfixCode += endLabel + " JMP ";
    postfixCode += elseLabel + ": ";

    bool inElse = false;
    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
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

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "SimpleExpr") {
            traverseSimpleExpr(child);

            if (!typeStack.empty()) {
                exprType = typeStack.top();
                typeStack.pop();
            }
        }
        else if (child->name == "operator") {
            std::string op = child->value;

            for (size_t j = 0; j < node->children.size(); j++) {
                ParseTreeNode* grandchild = node->children[j];
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

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "id") {
            std::string varName = child->value;

            checkVariableUsage(varName, child->line, child->position);

            std::string varType = getVariableType(varName);
            if (varType.empty()) {
                varType = "integer";
            }
            typeStack.push(varType);
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
    int exprCount = 0;

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "Expr") {
            traverseExpr(child);

            if (!typeStack.empty()) {
                if (exprCount == 0) {
                    leftType = typeStack.top();
                    typeStack.pop();
                }
                else {
                    rightType = typeStack.top();
                    typeStack.pop();
                }
                exprCount++;
            }
        }
        else if (child->name == "RelationOperator") {
            if (!leftType.empty() && !rightType.empty()) {
                checkTypeCompatibility(leftType, rightType, child->line, child->position,
                    "condition (operator " + child->value + ")");
            }
        }
    }
}

void SemanticAnalyzer::generatePostfixForExpr(ParseTreeNode* node) {
    if (!node) return;

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "SimpleExpr") {
            generatePostfixForSimpleExpr(child);
        }
    }

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "operator") {
            std::string op = child->value;

            for (size_t j = 0; j < node->children.size(); j++) {
                ParseTreeNode* grandchild = node->children[j];
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

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
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
    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
        if (child->name == "Expr") {
            generatePostfixForExpr(child);

            if (firstExpr) {
                firstExpr = false;
            }
        }
    }

    for (size_t i = 0; i < node->children.size(); i++) {
        ParseTreeNode* child = node->children[i];
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