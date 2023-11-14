#include "util/string.h"
#include "lexer.h"
#include "parser.h"

inline static struct ASTNode* new_node();
inline static struct ASTNode* new_literal(struct Token** token);
inline static struct ASTNode* new_ident(struct Token** token);

inline static void   match_eq(struct Token** token);
inline static String match_ident(struct Token** token);
inline static union LangVal match_literal(struct Token** token, enum ASTType* type);

static struct ASTNode* parse_expr(struct Token** token);
static struct ASTNode  parse_assign(struct Token** token);
static struct ASTNode  parse_call(struct Token** token);

static struct ASTNode parse_val(struct Token** token);
static struct ASTNode parse_var(struct Token** token);
static struct ASTNode parse_fun(struct Token** token);

#define check_error_internal(_tok, _type) _check_error_internal(_tok, _type, __func__)
inline static void _check_error_internal(struct Token* token, enum TokenType type, const char* fn);
noreturn static void error_unexpected(const char* type, struct Token* token);
noreturn static void error_expected(const char* expect, struct Token* token);

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
		case TOKEN_VAL: ast.nodes[ast.nodec++] = parse_val(&token); break;
		case TOKEN_VAR: ast.nodes[ast.nodec++] = parse_var(&token); break;
		case TOKEN_FUN: ast.nodes[ast.nodec++] = parse_fun(&token); break;
		case TOKEN_EOF:
			return ast;
		default:
			ERROR("[LANG] Unhandled token: [%s]:%d:%d", STRING_OF_TOKEN(token->type), token->line, token->col);
			exit(70);
		}
	} while (token < tokens->tokens + tokens->tokenc);

	assert(0 && "Should have returned from TOKEN_EOF");
	return ast;
}

static void print_ast_rec(struct ASTNode* node, int spacec)
{
	for (int s = 0; s < spacec; s++)
		printf("--> ");
	if (node->type != AST_VAL && node->type != AST_VAR && node->type != AST_FUN)
		printf("[%s (%s)]\n", STRING_OF_NODE(node->type), node->lexeme.data);
	if (token_is_terminal(node->type))
		return;

	switch (node->type) {
	case AST_VAL:
	case AST_VAR:
	case AST_FUN:
		printf(node->type == AST_VAL? "val %s\n": node->type == AST_VAR? "var %s\n": "fun %s\n", node->lexeme.data);
		print_ast_rec(node->expr, spacec + 1);
		break;
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
	struct Token*   tok  = *token;
	struct ASTNode* node = new_node();
	node->lexeme  = string_copy(tok->lexeme);
	node->literal = match_literal(&tok, &node->type);

	*token = tok;
	return node;
}

inline static struct ASTNode* new_ident(struct Token** token)
{
	struct Token* tok = *token;
	check_error_internal(tok, TOKEN_IDENT);

	struct ASTNode* node = new_node();
	node->type   = AST_IDENT;
	node->lexeme = string_copy(tok->lexeme);
	
	(*token)++;
	return node;
}

/* -------------------------------------------------------------------- */

inline static void match_eq(struct Token** token)
{
	if(strncmp((*token)->lexeme.data, "=", 1))
		error_expected("an equals (=)", *token);
	(*token)++;
}

inline static String match_ident(struct Token** token)
{
	if ((*token)->type != TOKEN_IDENT)
		error_expected("identifier", *token);
	return ((*token)++)->lexeme;
}

inline static union LangVal match_literal(struct Token** token, enum ASTType* type)
{
	struct Token* tok = *token;
	if (tok->type == TOKEN_NUMBER) {
		if (string_contains(tok->lexeme, '.') != -1) {
			*type = AST_FLT;
			(*token)++;
			return (union LangVal){ .flt = atof(tok->lexeme.data) };
		} else {
			*type = AST_INT;
			(*token)++;
			return (union LangVal){ .s64 = atoi(tok->lexeme.data) };
		}
	} else if (tok->type == TOKEN_STRING) {
		*type = AST_STR;
		(*token)++;
		return (union LangVal){ .str = tok->lexeme.data };
	} else {
		error_expected("literal value", tok);
	}
}

/* -------------------------------------------------------------------- */

static struct ASTNode* parse_expr(struct Token** token)
{
	struct Token* tok = *token;
	struct ASTNode* node;
	switch (tok->type) {
	case TOKEN_NUMBER:
	case TOKEN_STRING:
		node = new_literal(&tok);
		*token = tok;
		return node;
		break;
	default:
		error_expected("expression", tok);
	}

	return NULL;
}

// static struct ASTNode parse_assign(struct Token** token)
// {
// 	struct ASTNode node = {
// 		.type = AST_ASSIGN,
// 	};

// 	node.binary.left = new_ident(token);
// 	node.lexeme = string_copy((*token)->lexeme);
// 	match_eq(token);
// 	node.binary.right = parse_expr(token);

// 	return node;
// }

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

static struct ASTNode parse_val(struct Token** token)
{
	struct Token* tok = *token;
	check_error_internal(tok, TOKEN_VAL);
	tok++;

	struct ASTNode node = {
		.type   = AST_VAL,
		.lexeme = string_copy(match_ident(&tok))
	};
	match_eq(&tok);
	node.expr = parse_expr(&tok);

	*token = tok;
	return node;
}

static struct ASTNode parse_var(struct Token** token)
{
	struct Token* tok = *token;
	check_error_internal(tok, TOKEN_VAR);
	tok++;

	struct ASTNode node = {
		.type   = AST_VAR,
		.lexeme = string_copy(match_ident(&tok))
	};
	match_eq(&tok);
	node.expr = parse_expr(&tok);

	*token = tok;
	return node;
}

static struct ASTNode parse_fun(struct Token** token)
{
	struct Token* tok = *token;
	check_error_internal(tok, TOKEN_FUN);
	tok++;

	struct ASTNode node = {
		.type   = AST_FUN,
		.lexeme = string_copy(match_ident(&tok)),
	};
	// TODO: parse paramter list here
	match_eq(&tok);
	node.expr = parse_expr(&tok);

	*token = tok;
	return node;
}

/* -------------------------------------------------------------------- */

inline static void _check_error_internal(struct Token* token, enum TokenType type, const char* fn)
{
	if (token->type != type) {
		ERROR("[LANG] %s should not be called with any other token than %s = %d (got: %s = %d)",
		      fn, STRING_OF_TOKEN(type), type, STRING_OF_TOKEN(token->type), token->type);
		exit(70);
	}
}

noreturn static void error_unexpected(const char* type, struct Token* token)
{
	ERROR("[LANG] Unexpected %s: \"%s\" on line %d:%d",
	      type, token->lexeme.data, token->line, token->col);
	exit(70);
}

noreturn static void error_expected(const char* expect, struct Token* token)
{
	ERROR("[LANG] Expected %s but got: \"%s\" on line %d:%d",
	      expect, token->lexeme.data, token->line, token->col);
	exit(70);
}
