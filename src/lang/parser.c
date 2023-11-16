#include "lang/lang.h"
#include "util/string.h"
#include "util/varray.h"
#include "util/htable.h"
#include "lexer.h"
#include "parser.h"

inline static struct ASTNode* new_node();
inline static struct ASTNode* new_literal(struct Token** token);
inline static struct ASTNode* new_ident(struct Token** token);

inline static void   match_eq(struct Token** token);
inline static String match_ident(struct Token** token);
inline static union LangVal match_literal(struct Token** token, enum ASTType* type);

static struct ASTNode* parse_expr(struct Token** token);
static struct VArray*  parse_expr_list(struct Token** token, int count);
static struct ASTNode  parse_assign(struct Token** token);
static struct ASTNode  parse_call(struct Token** token);

static struct ASTNode parse_val(struct Token** token);
static struct ASTNode parse_var(struct Token** token);
static struct ASTNode parse_fun(struct Token** token);

#define check_error_internal(_tok, _type) _check_error_internal(_tok, _type, __func__)
inline static void _check_error_internal(struct Token* token, enum TokenType type, const char* fn);
noreturn static void error_unexpected(const char* type, struct Token* token);
noreturn static void error_expected(const char* expect, struct Token* token);

struct AST parser_parse(struct Tokenizer* tknz)
{
	intptr max_expr = 10;
	struct AST ast = {
		.nodec = 0,
		.nodes = smalloc(max_expr*sizeof(struct ASTNode)),

		.lits = varray_new(PARSER_DEFAULT_LITERAL_COUNT, sizeof(struct LangValTagged)),
		.lit_table = htable_new(PARSER_DEFAULT_LITERAL_COUNT),
		.var_table = htable_new(PARSER_DEFAULT_VARIABLE_COUNT),
		.fun_table = htable_new(PARSER_DEFAULT_FUNCTION_COUNT),
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

	ERROR("Should have returned from TOKEN_EOF, instead have: %s (%d)", STRING_OF_TOKEN(token->type), token->type);
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

inline static struct ASTNode* new_literal(struct Token** token)
{
	struct ASTNode* node = new_node();
	node->lexeme  = string_copy((*token)->lexeme);
	node->literal = match_literal(token, &node->type);



	// (*token)++; token is consumed by match_literal()
	return node;
}

inline static struct ASTNode* new_ident(struct Token** token)
{
	struct ASTNode* node = new_node();
	node->type   = AST_IDENT;
	node->lexeme = string_copy((*token)->lexeme);
	
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
	struct ASTNode* node;
	switch ((*token)->type) {
	case TOKEN_NUMBER:
	case TOKEN_STRING:
		return new_literal(token);
		break;
	case TOKEN_IDENT:
		node = new_node();
		node->type   = AST_CALL;
		node->lexeme = string_copy((*token)->lexeme);
		(*token)++;

		node->params = parse_expr_list(token, 1);
		return node;
		break;
	default:
		error_expected("expression", (*token));
	}

	return NULL;
}

static struct VArray* parse_expr_list(struct Token** token, int count)
{
	struct VArray* list = varray_new(count, sizeof(struct ASTNode));
	for (int i = 0; i < count; i++) {
		if ((*token)->type > TOKEN_VALUE_START && (*token)->type < TOKEN_KEYWORD_END) {
			varray_push(list, parse_expr(token));
		} else {
			error_expected("value for function", *token);
		}
	}

	return list;
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
	check_error_internal(*token, TOKEN_FUN);
	(*token)++;

	struct ASTNode node = {
		.type   = AST_FUN,
		.lexeme = string_copy(match_ident(token)),
	};
	// TODO: parse paramter list here
	match_eq(token);
	node.expr = parse_expr(token);

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
	exit(1);
}

noreturn static void error_expected(const char* expect, struct Token* token)
{
	ERROR("[LANG] Expected %s but got: \"%s\" on line %d:%d",
	      expect, token->lexeme.data, token->line, token->col);
	exit(1);
}
