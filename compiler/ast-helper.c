#include "ast.h"
#include<stdlib.h>
#include <assert.h>




FuncDefNode* create_func_def_node(char* name, int nargs, char** argnames, void* body) {
    FuncDefNode* node = malloc(sizeof(FuncDefNode));
    node->type = FUNC_DEF;
    node->name = name;
    node->nargs = nargs;
    node->argnames = argnames;
    node->body = body;
    return node;
}

FuncCallNode* create_func_call_node(char* callee, void* args) {
    FuncCallNode* node = malloc(sizeof(FuncCallNode));
    node->type = FUNC_CALL;
    node->callee = callee;
    node->args = args;
    return node;
}

IfNode* create_if_node(void* expr, void* then_branch, void* else_branch) {
    IfNode* node = malloc(sizeof(IfNode));
    node->type = IF;
    node->expr = expr;
    node->then_branch = then_branch;
    node->else_branch = else_branch;
    return node;
}

WhileNode* create_while_node(void* expr, void* body) {
    WhileNode* node = malloc(sizeof(WhileNode));
    node->type = WHILE;
    node->expr = expr;
    node->body = body;
    return node;
}

AssgNode* create_assg_node(char* lhs, void* rhs) {
    AssgNode* node = malloc(sizeof(AssgNode));
    node->type = ASSG;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

ReturnNode* create_return_node(void* expr) {
    ReturnNode* node = malloc(sizeof(ReturnNode));
    node->type = RETURN;
    node->expr = expr;
    return node;
}

StmtListNode* create_stmt_list_node(struct DummyNode* stmt, struct StmtListNode* next) {
    StmtListNode* node = malloc(sizeof(StmtListNode));
    node->type = STMT_LIST;
    node->stmt = stmt;
    node->next = next;
    return node;
}

ExprListNode* create_expr_list_node(struct DummyNode* expr, struct ExprListNode* next) {
    ExprListNode* node = malloc(sizeof(ExprListNode));
    node->type = EXPR_LIST;
    node->expr = expr;
    node->next = next;
    return node;
}


IdentifierNode* create_identifier_node(char* name) {
    IdentifierNode* node = malloc(sizeof(IdentifierNode));
    node->type = IDENTIFIER;
    node->name = name;
    node->replaced = 0;
    node->replacedWith = NULL;
    return node;
}

IntConstNode* create_int_const_node(int value) {
    IntConstNode* node = malloc(sizeof(IntConstNode));
    node->type = INTCONST;
    node->value = value;
    node->replaced = 0;
    node->replacedWith = NULL;
    return node;
}

BinaryExprNode* create_binary_expr_node(NodeType type, void* operand1, void* operand2) {
    BinaryExprNode* node = malloc(sizeof(BinaryExprNode));
    node->type = type;
    node->operand1 = operand1;
    node->operand2 = operand2;
    node->replaced = 0;
    node->replacedWith = NULL;
    return node;
}

UnaryExprNode* create_unary_expr_node(void* operand) {
    UnaryExprNode* node = malloc(sizeof(UnaryExprNode));
    node->type = UMINUS;
    node->operand = operand;
    node->replaced = 0;
    node->replacedWith = NULL;
    return node;
}

NodeType ast_node_type(void* ptr) {
    DummyNode* dummy = (DummyNode*)ptr; // Cast to a dummy node
    assert(dummy != NULL);               // Ensure that the pointer is not NULL
    return dummy->type;                  // Return the type of the AST node
}



// TRAVERSALS


char* func_def_name(void* ptr) {
    FuncDefNode* node = (FuncDefNode*)ptr;
    return node->name;
}

int func_def_nargs(void* ptr) {
    FuncDefNode* node = (FuncDefNode*)ptr;
    return node->nargs;
}

char* func_def_argname(void* ptr, int n) {
    FuncDefNode* node = (FuncDefNode*)ptr;
    if (n >= 1 && n <= node->nargs) {
        return node->argnames[n - 1];
    }
    return NULL; // n is out of range
}

void* func_def_body(void* ptr) {
    FuncDefNode* node = (FuncDefNode*)ptr;
    return node->body;
}

char* func_call_callee(void* ptr) {
    FuncCallNode* node = (FuncCallNode*)ptr;
    return node->callee;
}

void* func_call_args(void* ptr) {
    FuncCallNode* node = (FuncCallNode*)ptr;
    return node->args;
}

void* stmt_list_head(void* ptr) {
    StmtListNode* node = (StmtListNode*)ptr;
    return node->stmt;
}

void* stmt_list_rest(void* ptr) {
    StmtListNode* node = (StmtListNode*)ptr;
    return node->next;
}

char* expr_id_name(void* ptr) {
    IdentifierNode* node = (IdentifierNode*)ptr;
    return node->name;
}

int expr_intconst_val(void* ptr) {
    IntConstNode* node = (IntConstNode*)ptr;
    return node->value;
}

void* expr_operand_1(void* ptr) {
    BinaryExprNode* node = (BinaryExprNode*)ptr;
    return node->operand1;
}

void* expr_operand_2(void* ptr) {
    BinaryExprNode* node = (BinaryExprNode*)ptr;
    return node->operand2;
}

void* stmt_if_expr(void* ptr) {
    IfNode* node = (IfNode*)ptr;
    return node->expr;
}

void* stmt_if_then(void* ptr) {
    IfNode* node = (IfNode*)ptr;
    return node->then_branch;
}

void* stmt_if_else(void* ptr) {
    IfNode* node = (IfNode*)ptr;
    return node->else_branch;
}

char* stmt_assg_lhs(void* ptr) {
    AssgNode* node = (AssgNode*)ptr;
    return node->lhs;
}

void* stmt_assg_rhs(void* ptr) {
    AssgNode* node = (AssgNode*)ptr;
    return node->rhs;
}

void* stmt_while_expr(void* ptr) {
    WhileNode* node = (WhileNode*)ptr;
    return node->expr;
}

void* stmt_while_body(void* ptr) {
    WhileNode* node = (WhileNode*)ptr;
    return node->body;
}

void* stmt_return_expr(void* ptr) {
    ReturnNode* node = (ReturnNode*)ptr;
    return node->expr;
}

void* expr_list_head(void* ptr) {
    ExprListNode* node = (ExprListNode*)ptr;
    return node->expr;
}

void* expr_list_rest(void* ptr) {
    ExprListNode* node = (ExprListNode*)ptr;
    return node->next;
}
