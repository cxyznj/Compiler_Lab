#ifndef _DATASTRUCT_H_

#define _DATASTRUCT_H_

enum { TERMINAL, NOTERMINAL, EMPTY };

union Val{
    unsigned int intvalue;
    float floatvalue;
    char strvalue[50];
};    
    
struct TreeNode{
    // Type of the Node: terminal or not terminal or EMPTY
    int ttype;
    // The name of the Node, like "Exp", "TYPE", "ID" and so on
    char name[50];
    // The situation of this Node: (line, column)
    int situation[2];
    // the type value of the Node (for INT/FLOAT/ID/TYPE/RELOP)
    union Val val;
    // The next sibling Node
    struct TreeNode *sibling;
    // The first child Node
    struct TreeNode *child;
};

void printTree(struct TreeNode* root, int layer);

struct TreeNode* CreatNode(int type, char* name, int line, int column, union Val v);

int add_sibling(struct TreeNode* t1, struct TreeNode* t2);

int add_child(struct TreeNode* t1, struct TreeNode* t2);

// void printNode(struct TreeNode* root);

#endif