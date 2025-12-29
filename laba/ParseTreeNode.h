#ifndef PARSETREENODE_H
#define PARSETREENODE_H

#include <string>
#include <vector>

struct ParseTreeNode {
    std::string name;
    std::string value;
    std::vector<ParseTreeNode*> children;
    int line;
    int position;

    ParseTreeNode(const std::string& n, const std::string& v = "", int l = 0, int p = 0)
        : name(n), value(v), line(l), position(p) {
    }

    ~ParseTreeNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

#endif