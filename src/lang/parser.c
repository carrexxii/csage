#include "util/string.h"
#include "lexer.h"
#include "parser.h"

inline static bool is_terminal(enum ASTType type);
inline static struct ASTNode* new_node();
inline static struct ASTNode* new_literal(struct Token** token);
inline static struct ASTNode* new_ident(struct Token** token);
static inline void match_eq(struct Token** token);
static struct ASTNode* parse_expr(struct Token** token);
static struct ASTNode parse_assign(struct Token** token);

struct AST parser_parse(struct TokenList* tokens)
{
	intptr max_expr = 10;
	struct AST ast = {
		.nodec = 0,
		.nodes = smalloc(max_expr*sizeof(struct ASTNode)),
	};

	struct Token* token = tokens->tokens;
	do {
		switch (token->type) {
		case TOKEN_IDENT:
			switch ((token + 1)->type) {
			case TOKEN_EQ:
				ast.nodes[ast.nodec++] = parse_assign(&token);
				break;
			default:
				ERROR("[LANG] Expected <> got [%s (%s)]:%d:%d", STRING_OF_TOKEN(token->type),
				      token->lexeme.data, token->line, token->col);
			}
			break;
		case TOKEN_EOF:
			return ast;
		default:
			ERROR("[LANG] Unhandled token: [%s]:%d:%d", STRING_OF_TOKEN(token->type), token->line, token->col);
		}
	} while (token < tokens->tokens + tokens->tokenc);

	assert(0 && "Should have returned from TOKEN_EOF");
	return ast;
}

static void print_ast_rec(struct ASTNode* node, int spacec)
{
	for (int s = 0; s < spacec; s++)
		fprintf(stderr, "  ");
	fprintf(stderr, "[%s]\n", STRING_OF_NODE(node->type));
	if (is_terminal(node->type))
		return;

	switch (node->type) {
	case AST_BINARY:
	case AST_ASSIGN:
		print_ast_rec(node->left, spacec + 1);
		print_ast_rec(node->right, spacec + 1);
		break;
	case AST_PAREN:
		print_ast_rec(node->paren, spacec + 1);
		break;
	case AST_UNARY:
		print_ast_rec(node->node, spacec + 1);
		break;
	default:
		ERROR("[LANG] Could print node type \"%s\" (%d)", STRING_OF_NODE(node->type), node->type);
	}
}

void parser_print_ast(struct AST ast)
{
	struct ASTNode* node;
	for (int n = 0; n < ast.nodec; n++) {
		node = &ast.nodes[n];
		print_ast_rec(node, 0);
	}
}

/* -------------------------------------------------------------------- */

inline static bool is_terminal(enum ASTType type)
{
	return type == AST_INT  || type == AST_REAL  || type == AST_STR ||
	       type == AST_BOOL || type == AST_IDENT || type == AST_IDENT_LIST;
}

inline static struct ASTNode* new_node()
{
	return smalloc(sizeof(struct ASTNode));
}

/* -------------------------------------------------------------------- */

inline static struct ASTNode* new_literal(struct Token** token)
{
	struct Token* tk = *token;
	struct ASTNode* node = new_node();
	if (tk->type == TOKEN_NUMBER) {
		if (string_contains(tk->lexeme, '.')) {
			node->type = AST_REAL;
			node->real = atof(tk->lexeme.data);
		} else {
			node->type = AST_INT;
			node->real = atoi(tk->lexeme.data);
		}
	} else if (tk->type == TOKEN_STRING) {
		node->type   = AST_STR;
		node->string = string_new(tk->lexeme.data, tk->lexeme.len);
	} else {
		ERROR("[LANG] Token is not a valid literal: [%s (%s)]:%d:%d", STRING_OF_TOKEN(tk->type),
		      tk->lexeme.data, tk->line, tk->col);
	}

	(*token)++;
	return node;
}

inline static struct ASTNode* new_ident(struct Token** token)
{
	assert((*token)->type == TOKEN_IDENT);

	struct ASTNode* node = new_node();
	node->type  = AST_IDENT;
	node->ident = string_copy((*token)->lexeme);
	
	(*token)++;
	return node;
}

/* -------------------------------------------------------------------- */

static inline void match_eq(struct Token** token)
{
	assert((*token)->type == TOKEN_EQ);
	(*token)++;
}

/* -------------------------------------------------------------------- */

static struct ASTNode* parse_expr(struct Token** token)
{
	struct Token* tk = *token;
	if (tk->type > TOKEN_LITERAL_START && tk->type < TOKEN_LITERAL_END)
		return new_literal(token);

	ERROR("[LANG] Could not parse expression: [%s (%s)]:%d:%d", STRING_OF_TOKEN(tk->type),
	      tk->lexeme.data, tk->line, tk->col);
	return NULL;
}

static struct ASTNode parse_assign(struct Token** token)
{
	struct ASTNode node = {
		.type = AST_ASSIGN,
	};

	node.left = new_ident(token);
	match_eq(token);
	node.right = parse_expr(token);

	return node;
}
