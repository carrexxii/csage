#ifndef LANG_PARSER_H
#define LANG_PARSER_H

#include "lang.h"
#include "lexer.h"

enum ASTType {
	AST_NONE = 0,

	AST_INT = LANG_INT,
	AST_FLT = LANG_FLT,
	AST_STR = LANG_STR,
	AST_BOOL,

	AST_IDENT,
	AST_VAL,
	AST_VAR,
	AST_FUN,

	AST_IDENT_LIST,
	AST_EXPR_LIST,
	AST_PAREN,
	AST_UNARY,
	AST_BINARY,
	AST_ASSIGN,
	AST_CALL,
	AST_COUNT,
};

struct ASTUnaryNode {
	struct ASTNode* node;
};
struct ASTBinaryNode {
	struct ASTNode* left;
	struct ASTNode* right;
};
struct ASTList {
	struct ASTNode** list;
	int len;
};

struct ASTNode {
	String lexeme;
	enum ASTType type;
	union {
		/* Terminal */
		union LangVal literal;
		/* Non-terminal */
		struct ASTNode* expr;
		struct ASTNode* paren;
		struct ASTUnaryNode  unary;
		struct ASTBinaryNode binary;
		struct ASTList params;
	};
};

struct AST {
	struct ASTNode* nodes;
	intptr nodec;
};

struct AST parser_parse(struct TokenList* tokens);
void parser_print_ast(struct AST ast);

inline static bool token_is_terminal(enum ASTType type) {
	return type == AST_INT  || type == AST_FLT || type == AST_STR ||
	       type == AST_BOOL || type == AST_IDENT;
}

#define STRING_OF_NODE(_e0) \
	(_e0) == AST_NONE?      "AST_NONE":      (_e0) == AST_INT?    "AST_INT":    (_e0) == AST_FLT?   "AST_REAL":  \
	(_e0) == AST_STR?       "AST_STR":       (_e0) == AST_BOOL?   "AST_BOOL":   (_e0) == AST_IDENT? "AST_IDENT": \
	(_e0) == AST_EXPR_LIST? "AST_EXPR_LIST": (_e0) == AST_PAREN?  "AST_PAREN":  (_e0) == AST_UNARY? "AST_UNARY": \
	(_e0) == AST_BINARY?    "AST_BINARY":    (_e0) == AST_ASSIGN? "AST_ASSIGN": (_e0) == AST_CALL?  "AST_CALL":  \
	"<Unknown value for enum \"ASTType\">"

#endif
