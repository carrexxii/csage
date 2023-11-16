#include "lang/lang.h"
#include "util/string.h"
#include "util/varray.h"
#include "util/htable.h"
#include "lexer.h"
#include "parser.h"

inline static struct ASTNode* new_node();
inline static struct LangVar new_literal(struct Token tok);

inline static void match_eq(struct Tokenizer* tknz);
inline static String match_ident(struct Tokenizer* tknz);
inline static struct LangVar match_literal(struct Tokenizer* tknz);

static struct ASTNode* parse_expr(struct Tokenizer* tknz);
static struct VArray*  parse_expr_list(struct Tokenizer* tknz, isize count);
static struct ASTNode  parse_call(struct Tokenizer* tknz);

static struct ASTNode parse_assign(struct Tokenizer* tknz, enum ASTType type);
static struct ASTNode parse_val(struct Tokenizer* tknz);
static struct ASTNode parse_var(struct Tokenizer* tknz);
static struct ASTNode parse_fun(struct Tokenizer* tknz);

#define check_error_internal(_tok, _type) _check_error_internal(_tok, _type, __func__)
inline static void _check_error_internal(struct Token token, enum TokenType type, const char* fn);
noreturn static void error_unexpected(const char* type, struct Token token);
noreturn static void error_expected(const char* expect, struct Token token);

struct AST parser_parse(struct Tokenizer tknz)
{
	intptr max_expr = 10;
	struct AST ast = {
		.nodec = 0,
		.nodes = smalloc(max_expr*sizeof(struct ASTNode)),

		.lits = varray_new(PARSER_DEFAULT_LITERAL_COUNT, sizeof(struct LangVar)),
		.lit_table = htable_new(PARSER_DEFAULT_LITERAL_COUNT),
		.var_table = htable_new(PARSER_DEFAULT_VARIABLE_COUNT),
		.fun_table = htable_new(PARSER_DEFAULT_FUNCTION_COUNT),
	};

	struct Token tok;
	while (1) {
		tok = lexer_next(&tknz);
		switch (tok.type) {
		case TOKEN_VAL: ast.nodes[ast.nodec++] = parse_assign(&tknz, AST_VAL); break;
		case TOKEN_VAR: ast.nodes[ast.nodec++] = parse_assign(&tknz, AST_VAR); break;
		case TOKEN_FUN: ast.nodes[ast.nodec++] = parse_fun(&tknz); break;
		case TOKEN_EOF:
			return ast;
		default:
			ERROR("[LANG] Unhandled token: [%s]:%d:%d", STRING_OF_TOKEN(tok.type), tok.line, tok.col);
			exit(70);
		}
	}

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
		for (int i = 0; i < node->params->len; i++) {
			// DEBUG_VALUE(((struct ASTNode*)varray_get(node->params, i))->expr);
			// DEBUG_VALUE(varray_get(node->params, i));
			// DEBUG_VALUE((struct ASTNode*)varray_get(node->params, i));
			print_ast_rec(varray_get(node->params, i), spacec + 1);
		}
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
	struct HPair* pair;
	printf("Literals:\n");
	for (int i = 0; i < ast.lit_table->cap; i++) {
		pair = &ast.lit_table->pairs[i];
		if (pair->key.data)
			printf("\t%s: %ld\n", pair->key.data, pair->val);
	}

	printf("Variables:\n");
	printf("Functions:\n");

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

inline static struct ASTNode* new_ident(struct Token tok)
{
	struct ASTNode* node = new_node();
	node->type   = AST_IDENT;
	node->lexeme = string_copy(tok.lexeme);

	return node;
}

inline static struct LangVar new_literal(struct Token tok)
{
	if (tok.type == TOKEN_NUMBER) {
		if (string_contains(tok.lexeme, '.') != -1)
			return (struct LangVar){
				.type = LANG_FLT,
				.val  = atof(tok.lexeme.data),
			};
		else
			return (struct LangVar){
				.type = LANG_INT,
				.val  = atoi(tok.lexeme.data),
			};
	} else if (tok.type == TOKEN_STRING) {
		return (struct LangVar){
			.type = LANG_STR,
			.val  = (union LangVal){ .str = string_new_ptr(tok.lexeme.data, tok.lexeme.len) },
		};
	} else {
		error_expected("literal value", tok);
	}
}

/* -------------------------------------------------------------------- */

inline static void match_eq(struct Tokenizer* tknz)
{
	struct Token tok = lexer_next(tknz);
	if(strncmp(tok.lexeme.data, "=", 1))
		error_expected("an equals (=)", tok);
}

inline static String match_ident(struct Tokenizer* tknz)
{
	struct Token tok = lexer_next(tknz);
	if (tok.type != TOKEN_IDENT)
		error_expected("identifier", tok);
	return tok.lexeme;
}

/* -------------------------------------------------------------------- */

static struct ASTNode* parse_expr(struct Tokenizer* tknz)
{
	struct Token tok = lexer_next(tknz);
	struct ASTNode* node = new_node();
	switch (tok.type) {
	case TOKEN_NUMBER:
	case TOKEN_STRING:
		struct LangVar lit = new_literal(tok);
		node->type   = (enum ASTType)lit.type;
		node->lexeme = string_copy(tok.lexeme);
		return node;
	case TOKEN_IDENT:
		node->type   = AST_CALL;
		node->lexeme = string_copy(tok.lexeme);

		node->params = parse_expr_list(tknz, 1); // TODO: find # of params from tables
		return node;
		break;
	default:
		error_expected("expression", tok);
	}

	return NULL;
}

static struct VArray* parse_expr_list(struct Tokenizer* tknz, isize count)
{
	if (count <= 0)
		ERROR("[LANG] Passed %ld to `parse_expr_list()`", count);

	struct VArray* list = varray_new(count, sizeof(struct ASTNode));
	for (int i = 0; i < count; i++) {
		varray_push(list, parse_expr(tknz));
	}

	return list;
}

static struct ASTNode parse_assign(struct Tokenizer* tknz, enum ASTType type)
{
	struct ASTNode node = {
		.type   = type,
		.lexeme = string_copy(match_ident(tknz))
	};
	match_eq(tknz);
	node.expr = parse_expr(tknz);

	return node;
}

static struct ASTNode parse_fun(struct Tokenizer* tknz)
{
	struct ASTNode node = {
		.type   = AST_FUN,
		.lexeme = string_copy(match_ident(tknz)),
	};
	// TODO: parse paramter list here
	match_eq(tknz);
	node.expr = parse_expr(tknz);

	return node;
}

/* -------------------------------------------------------------------- */

inline static void _check_error_internal(struct Token token, enum TokenType type, const char* fn)
{
	if (token.type != type) {
		ERROR("[LANG] %s should not be called with any other token than %s = %d (got: %s = %d)",
		      fn, STRING_OF_TOKEN(type), type, STRING_OF_TOKEN(token.type), token.type);
		exit(70);
	}
}

noreturn static void error_unexpected(const char* type, struct Token token) {
	ERROR("[LANG] Unexpected %s: \"%s\" on line %d:%d", type, token.lexeme.data, token.line, token.col);
	exit(1);
}

noreturn static void error_expected(const char* expect, struct Token token) {
	ERROR("[LANG] Expected %s but got: \"%s\" on line %d:%d", expect, token.lexeme.data, token.line, token.col);
	exit(1);
}
