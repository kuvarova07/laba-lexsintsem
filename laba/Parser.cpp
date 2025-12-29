#include "Parser.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Parser::Parser(Lexer& lex, std::ofstream& out)
    : lexer(lex), outputFile(out), hasError(false), parseTreeRoot(nullptr) {
    advanceToken();
}

void Parser::advanceToken() {
    currentToken = lexer.getNextToken(); 
}

bool Parser::match(enum TokenType expectedType) {
    return currentToken.type == expectedType; 
}

void Parser::error(const std::string& message) {
    std::stringstream errorMsg;
    errorMsg << "Stroka " << currentToken.line << ", position " << currentToken.position
        << ": " << message << " (poluchen '" << currentToken.value << "')";

    errorMessages.push_back(errorMsg.str());
    hasError = true;
}

void Parser::syncTo(const std::vector<TokenType>& syncTokens) {
    int skipped = 0; 
    while (currentToken.type != TokenType::END_OF_FILE) {
        for (TokenType syncToken : syncTokens) {
            if (match(syncToken)) {
                if (skipped > 0) {
                    outputFile << "  [Vosstanovleniye: propushcheno " << skipped << " tokenov]" << std::endl;
                }
                return;
            }
        }
        skipped++;
        advanceToken();
    }
}

void Parser::printTree(ParseTreeNode* node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) {
        outputFile << "  ";
    }

    if (!node->value.empty()) {
        outputFile << node->name << " [" << node->value << "]";
    }
    else {
        outputFile << node->name;
    }

    outputFile << std::endl;

    for (auto child : node->children) {
        printTree(child, depth + 1);
    }
}

void Parser::printErrors() {
    if (!errorMessages.empty()) {
        outputFile << "\nErrors found (total: " << errorMessages.size() << "):\n";
        for (size_t i = 0; i < errorMessages.size(); ++i) {
            outputFile << i + 1 << ". " << errorMessages[i] << std::endl;
        }
    }
}

ParseTreeNode* Parser::parseProcedure() {
    ParseTreeNode* node = new ParseTreeNode("Procedure");

    if (match(TokenType::PROCEDURE)) {
        ParseTreeNode* beginNode = parseBegin();
        if (beginNode) {
            node->children.push_back(beginNode);
        }
    }
    else {
        error("Ozhidayetsya 'procedure'");
        return node; 
    }

    if (match(TokenType::VAR)) { 
        ParseTreeNode* descrNode = parseDescriptions();
        if (descrNode) {
            node->children.push_back(descrNode);
        }
    }

    if (match(TokenType::BEGIN)) {
        node->children.push_back(new ParseTreeNode("keyword", "begin", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya 'begin'");
        syncTo({ TokenType::ID, TokenType::IF, TokenType::END });
    }

    ParseTreeNode* operatorsNode = parseOperators();
    if (operatorsNode) {
        node->children.push_back(operatorsNode);
    }

    if (match(TokenType::END)) {
        node->children.push_back(new ParseTreeNode("End", "end", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya 'end'"); 
    }

    return node; 
}

ParseTreeNode* Parser::parseBegin() {
    ParseTreeNode* node = new ParseTreeNode("Begin");

    node->children.push_back(new ParseTreeNode("keyword", "procedure", currentToken.line, currentToken.position));
    advanceToken(); 

    if (match(TokenType::ID)) {
        node->children.push_back(new ParseTreeNode("ProcedureName", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya imya protsedury");
    }

    if (match(TokenType::SEMICOLON)) {
        node->children.push_back(new ParseTreeNode("semicolon", ";", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya ';' posle imeni protsedury");
    }

    return node;
}

ParseTreeNode* Parser::parseDescriptions() {
    ParseTreeNode* node = new ParseTreeNode("Descriptions");
    node->children.push_back(new ParseTreeNode("keyword", "var", currentToken.line, currentToken.position)); 
    advanceToken(); 

    ParseTreeNode* descrListNode = parseDescrList();
    if (descrListNode) {
        node->children.push_back(descrListNode);
    }

    return node;
}

ParseTreeNode* Parser::parseDescrList() {
    ParseTreeNode* node = new ParseTreeNode("DescrList");

    while (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Identifikator") != std::string::npos)) {
        ParseTreeNode* descrNode = parseDescr();
        if (descrNode) {
            node->children.push_back(descrNode);
        }
        else {
            break;
        }
    }

    return node;
}

ParseTreeNode* Parser::parseDescr() {
    ParseTreeNode* node = new ParseTreeNode("Descr");

    ParseTreeNode* varListNode = parseVarList();
    if (varListNode) {
        node->children.push_back(varListNode);
    }
    else { 
        delete node;
        return nullptr;
    }

    // :
    if (match(TokenType::COLON)) {
        node->children.push_back(new ParseTreeNode("colon", ":", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya ':' posle spiska peremennykh");
        delete node;
        return nullptr;
    }

    if (match(TokenType::INTEGER)) {
        node->children.push_back(new ParseTreeNode("Type", "integer", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya 'integer'");
        delete node;
        return nullptr;
    }

    if (match(TokenType::SEMICOLON)) {
        node->children.push_back(new ParseTreeNode("semicolon", ";", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya ';' posle tipa");
        delete node;
        return nullptr;
    }

    return node;
}

ParseTreeNode* Parser::parseVarList() {
    ParseTreeNode* node = new ParseTreeNode("VarList");

    if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Identifikator") != std::string::npos)) {
        node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya identifikator v spiske peremennykh");
        delete node;
        return nullptr; 
    }

    while (match(TokenType::COMMA)) {
        node->children.push_back(new ParseTreeNode("comma", ",", currentToken.line, currentToken.position));
        advanceToken(); 

        if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Identifikator") != std::string::npos)) {
            node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
            advanceToken(); 
        }
        else {
            error("Ozhidayetsya identifikator posle zapyatoy");
            break; 
        }
    }

    return node;
}

ParseTreeNode* Parser::parseOperators() {
    ParseTreeNode* node = new ParseTreeNode("Operators");
    while (!match(TokenType::END) && currentToken.type != TokenType::END_OF_FILE) {
        ParseTreeNode* opNode = parseOp();
        if (opNode) {
            node->children.push_back(opNode);
        }
        else {
            outputFile << "  [Vosstanovleniye: propushchen token '" << currentToken.value << "']" << std::endl;
            advanceToken();

            syncTo({ TokenType::SEMICOLON, TokenType::END, TokenType::IF, TokenType::ID });

            if (match(TokenType::SEMICOLON)) {
                outputFile << "  [Propushchena ';']" << std::endl;
                advanceToken();
            }

            if (match(TokenType::END)) {
                break;
            }

        }
    }

    return node;
}


ParseTreeNode* Parser::parseOp() {
    ParseTreeNode* opNode = nullptr;

    if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Identifikator") != std::string::npos)) {
        opNode = parseAssignment();

        if (opNode) {
            if (match(TokenType::SEMICOLON)) {
                advanceToken(); 
            }
            else if (!match(TokenType::END) && !match(TokenType::ELSE)) {
                error("Ozhidayetsya ';' posle prisvaivaniya");

                syncTo({ TokenType::SEMICOLON, TokenType::ID, TokenType::IF, TokenType::END });
                if (match(TokenType::SEMICOLON)) {
                    advanceToken();
                }

                if (!match(TokenType::ID) && !match(TokenType::IF)) {
                    syncTo({ TokenType::SEMICOLON, TokenType::ID, TokenType::IF, TokenType::END });
                }
            }
        }
    }
    else if (match(TokenType::IF)) {
        opNode = parseIfStatement();
    }
    else {
        return nullptr;
    }

    return opNode;
}

void Parser::checkSemicolon() {
    if (!match(TokenType::END) && !match(TokenType::ELSE) && !match(TokenType::THEN)) {
        if (match(TokenType::SEMICOLON)) {
            advanceToken(); 
        }
        else {
            error("Ozhidayetsya ';' posle prisvaivaniya");
        }
    }
}

ParseTreeNode* Parser::parseAssignment() {
    ParseTreeNode* node = new ParseTreeNode("Assignment");

    node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
    advanceToken(); 

    if (match(TokenType::ASSIGN)) {
        node->children.push_back(new ParseTreeNode("assign", ":=", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya ':='");
        delete node;
        return nullptr; 
    }

    ParseTreeNode* exprNode = parseExpr();
    if (exprNode) {
        node->children.push_back(exprNode);
    }
    else {
        error("Ozhidayetsya vyrazheniye posle ':='");
        delete node;
        return nullptr;
    }

    return node; 
}


ParseTreeNode* Parser::parseIfStatement() {
    ParseTreeNode* node = new ParseTreeNode("IfStatement");

    node->children.push_back(new ParseTreeNode("keyword", "if", currentToken.line, currentToken.position));
    advanceToken(); 

    ParseTreeNode* conditionNode = parseCondition();
    if (conditionNode) {
        node->children.push_back(conditionNode);
    }
    else {
        error("Ozhidayetsya usloviye posle 'if'");
        delete node;
        return nullptr; 
    }

    if (match(TokenType::THEN)) {
        node->children.push_back(new ParseTreeNode("keyword", "then", currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya 'then'");
        syncTo({ TokenType::ID, TokenType::IF, TokenType::END, TokenType::SEMICOLON });
    }

    ParseTreeNode* thenOpNode = parseOp();
    if (thenOpNode) {
        node->children.push_back(thenOpNode);
    }

    if (match(TokenType::ELSE)) {
        node->children.push_back(new ParseTreeNode("keyword", "else", currentToken.line, currentToken.position));
        advanceToken();

        ParseTreeNode* elseOpNode = parseOp();
        if (elseOpNode) {
            node->children.push_back(elseOpNode);
        }
    }

    return node; 
}

ParseTreeNode* Parser::parseExpr() {
    ParseTreeNode* node = new ParseTreeNode("Expr");

    ParseTreeNode* simpleExprNode = parseSimpleExpr();
    if (simpleExprNode) {
        node->children.push_back(simpleExprNode);
    }
    else {
        delete node;
        return nullptr;
    }

    if (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = currentToken.value;
        node->children.push_back(new ParseTreeNode("operator", op, currentToken.line, currentToken.position));
        advanceToken(); 

        ParseTreeNode* exprNode = parseExpr();
        if (exprNode) {
            node->children.push_back(exprNode);
        }
    }

    return node;
}

ParseTreeNode* Parser::parseSimpleExpr() {
    ParseTreeNode* node = new ParseTreeNode("SimpleExpr");

    if (match(TokenType::ID) || (match(TokenType::ERROR) && currentToken.errorMessage.find("Identifikator") != std::string::npos)) {
        node->children.push_back(new ParseTreeNode("id", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else if (match(TokenType::CONST)) {
        node->children.push_back(new ParseTreeNode("const", currentToken.value, currentToken.line, currentToken.position));
        advanceToken();
    }
    else if (match(TokenType::LPAREN)) {
        node->children.push_back(new ParseTreeNode("lparen", "(", currentToken.line, currentToken.position));
        advanceToken(); 

        ParseTreeNode* exprNode = parseExpr();
        if (exprNode) {
            node->children.push_back(exprNode);
        }
        else {
            delete node;
            return nullptr;
        }

        if (match(TokenType::RPAREN)) {
            node->children.push_back(new ParseTreeNode("rparen", ")", currentToken.line, currentToken.position));
            advanceToken();
        }
        else {
            error("Ozhidayetsya ')'");
            delete node;
            return nullptr;
        }
    }
    else {
        error("Ozhidayetsya identifikator, konstanta ili vyrazheniye v skobkakh");
        delete node;
        return nullptr;
    }

    return node;
}

ParseTreeNode* Parser::parseCondition() {
    ParseTreeNode* node = new ParseTreeNode("Condition");

    ParseTreeNode* leftExpr = parseExpr();
    if (leftExpr) {
        node->children.push_back(leftExpr);
    }
    else {
        delete node;
        return nullptr; 
    }

    if (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL) ||
        match(TokenType::GREATER) || match(TokenType::LESS)) {
        node->children.push_back(new ParseTreeNode("RelationOperator", currentToken.value, currentToken.line, currentToken.position));
        advanceToken(); 
    }
    else {
        error("Ozhidayetsya operator otnosheniya (=, <>, >, <)");
        delete node;
        return nullptr; 
    }

    ParseTreeNode* rightExpr = parseExpr();
    if (rightExpr) {
        node->children.push_back(rightExpr);
    }
    else {
        error("Ozhidayetsya vyrazheniye");
        delete node;
        return nullptr; 
    }

    return node; 
}

bool Parser::parseForSemantic() {
    outputFile << "Sintaksicheskiy analiz (dlya semanticheskogo)\n";

    parseTreeRoot = parseProcedure();  

    printErrors();

    outputFile << "\nResult: ";
    if (hasError) {
        outputFile << "Syntax errors found\n";
    }
    else {
        outputFile << "No syntax errors found\n";
    }

    return !hasError;
}

bool Parser::parse() {
    outputFile << "Sintaksicheskiy analiz\n";

    ParseTreeNode* root = parseProcedure();

    printErrors();

    if (root) {
        outputFile << "\nDerevo razbora:\n";
        printTree(root);
        delete root;  
    }

    outputFile << "\nResult: ";
    if (hasError) {
        outputFile << "Syntax errors found\n";
    }
    else {
        outputFile << "No syntax errors found\n";
    }

    return !hasError;
}