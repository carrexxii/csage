#include "util/string.h"
#include "lexer.h"
#include "parser.h"

inline static struct ASTNode* new_node();
inline static struct ASTNode* new_literal(struct Token** token);
inline static struct ASTNode* new_ident(struct Token** token);
static inline void match_eq(struct Token** token);
static struct ASTNode* parse_expr(struct Token** token);
static struct ASTNode parse_assign(struct Token** token);
static struct ASTNode parse_call(struct Token** token);

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
			/* Assignment: `IDENT =` */
			case TOKEN_SYMBOL:
				if (strncmp(token->lexeme.data, "=", 1))
					ast.nodes[ast.nodec++] = parse_assign(&token);
				break;
			/* Function call: `IDENT INT|REAL|STR|IDENT` */
			case TOKEN_NUMBER:
			case TOKEN_STRING:
			case TOKEN_IDENT:
				ast.nodes[ast.nodec++] = parse_call(&token);
				break;
			default:
				ERROR("[LANG] Unhandled token: [%s]:%d:%d", STRING_OF_TOKEN(token->type), token->line, token->col);
				exit(1);
			}
			break;
		case TOKEN_EOF:
			return ast;
		default:
			ERROR("[LANG] Unhandled token: [%s]:%d:%d", STRING_OF_TOKEN(token->type), token->line, token->col);
			exit(1);
		}
	} while (token < tokens->tokens + tokens->tokenc);

	assert(0 && "Should have returned from TOKEN_EOF");
	return ast;
}

static void print_ast_rec(struct ASTNode* node, int spacec)
{
	for (int s = 0; s < spacec; s++)
		fprintf(stderr, " --> ");
	fprintf(stderr, "[%s (%s)]\n", STRING_OF_NODE(node->type), node->lexeme.data);
	if (token_is_terminal(node->type))
		return;

	switch (node->type) {
	case AST_BINARY:
	case AST_ASSIGN:
		print_ast_rec(node->binary.left, spacec + 1);
		print_ast_rec(node->binary.right, spacec + 1);
		break;
	case AST_PAREN:
		print_ast_rec(node->paren, spacec + 1);
		break;
	case AST_CALL:
		// fprintf(stderr, "[%s (%s)]\n", STRING_OF_NODE(node->type), node->lexeme.data);
		for (int i = 0; i < node->params.len; i++)
			print_ast_rec(node->params.list[i], spacec + 1);
		break;
	case AST_UNARY:
		print_ast_rec(node->unary.node, spacec + 1);
		break;
	default:
		ERROR("[LANG] Could not print node type \"%s\" (%d)", STRING_OF_NODE(node->type), node->type);
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

inline static struct ASTNode* new_node()
{
	return smalloc(sizeof(struct ASTNode));
}

/* -------------------------------------------------------------------- */

inline static struct ASTNode* new_literal(struct Token** token)
{
	struct Token* tok = *token;
	struct ASTNode* node = new_node();
	if (tok->type == TOKEN_NUMBER) {
		if (string_contains(tok->lexeme, '.') != -1) {
			node->type = AST_REAL;
			node->literal.dbl = atof(tok->lexeme.data);
		} else {
			node->type = AST_INT;
			node->literal.s64 = atoi(tok->lexeme.data);
		}
	} else if (tok->type == TOKEN_STRING) {
		node->type = AST_STR;
	} else {
		ERROR("[LANG] Token is not a valid literal: [%s (%s)]:%d:%d", STRING_OF_TOKEN(tok->type),
		      tok->lexeme.data, tok->line, tok->col);
	}
	DEBUG_VALUE(tok->lexeme.data);

	node->lexeme = string_copy(tok->lexeme);
	(*token)++;
	return node;
}

inline static struct ASTNode* new_ident(struct Token** token)
{
	assert((*token)->type == TOKEN_IDENT);

	struct ASTNode* node = new_node();
	node->type   = AST_IDENT;
	node->lexeme = string_copy((*token)->lexeme);
	
	(*token)++;
	return node;
}

/* -------------------------------------------------------------------- */

static inline void match_eq(struct Token** token)
{
	assert(!strncmp((*token)->lexeme.data, "=", 1));
	(*token)++;
}

/* -------------------------------------------------------------------- */

static struct ASTNode* parse_expr(struct Token** token)
{
	struct Token* tk = *token;
	switch (tk->type) {
	case TOKEN_NUMBER:
	case TOKEN_STRING:
		return new_literal(token);
		break;
	default:
		ERROR("[LANG] Could not parse expression: [%s (%s)]:%d:%d", STRING_OF_TOKEN(tk->type),
		      tk->lexeme.data, tk->line, tk->col);
	}

	return NULL;
}

static struct ASTNode parse_assign(struct Token** token)
{
	struct ASTNode node = {
		.type = AST_ASSIGN,
	};

	node.binary.left = new_ident(token);
	node.lexeme = string_copy((*token)->lexeme);
	match_eq(token);
	node.binary.right = parse_expr(token);

	return node;
}

static struct ASTNode parse_call(struct Token** token)
{
	struct ASTNode node = {
		.type   = AST_CALL,
		.lexeme = string_copy((*token)->lexeme),
	};
	(*token)++;

	node.params.len = 1;
	node.params.list = smalloc(node.params.len*sizeof(struct ASTNode)); // TODO: fix
	node.params.list[0] = parse_expr(token);

	return node;
}
