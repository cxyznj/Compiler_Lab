#include "datastruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printTree(struct TreeNode* root, int layer) {
    if(root != NULL && root->ttype != EMPTY) {
        for (int i = 0; i < layer; i++)
            printf("  ");
        
        printf("%s", root->name);

        if (root->ttype == NOTERMINAL)
            printf(" (%d)", root->situation[0]);
        
        if (strcmp(root->name, "ID") == 0) {
            printf(": %s", root->val.strvalue);
        }
        else if (strcmp(root->name, "TYPE") == 0) {
            printf(": %s", root->val.strvalue);
        }
        else if (strcmp(root->name, "INT") == 0) {
            printf(": %d", root->val.intvalue);
        }
        else if (strcmp(root->name, "FLOAT") == 0) {
            printf(": %f", root->val.floatvalue);
        }

        printf("\n");

        struct TreeNode *cld = root->child;
        while (cld != NULL) {
            printTree(cld, layer + 1);
            cld = cld->sibling;
        }
        // printNode(root);
    }
}

struct TreeNode* CreatNode(int type, char* name, int line, int column, union Val v) {
    struct TreeNode* tn = (struct TreeNode*)malloc(sizeof(struct TreeNode));
    tn->ttype = type;
    strcpy(tn->name, name);
    tn->situation[0] = line; tn->situation[1] = column;
    tn->val = v;
    tn->sibling = tn->child = NULL;
    return tn;
}

int add_sibling(struct TreeNode* t1, struct TreeNode* t2) {
    if(t1->sibling == NULL)
        t1->sibling = t2;
    else {
        struct TreeNode* temp = t1->sibling;
        t1->sibling = t2;
        t2->sibling = temp;
    }
    return 1;
}

int add_child(struct TreeNode* t1, struct TreeNode* t2) {
    t1->child = t2;
    return 1;
}

/*void printNode(struct TreeNode* root) {
    printf("Type = %d, Name = %s, situation = %d, %d\n \
        val = %d, sibling = %d, child = %d\n", root->ttype, root->name, \
        root->situation[0], root->situation[1], root->val.intvalue, root->sibling, root->child);
}*/