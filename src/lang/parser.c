#include "util/string.h"
#include "util/varray.h"
#include "util/htable.h"
#include "lang.h"
#include "lexer.h"
#include "parser.h"

inline static struct ASTNode* new_node();
inline static struct LangVar new_literal(struct AST* ast, struct Token tok);

inline static void match_eq(struct AST* ast);
inline static String match_ident(struct AST* ast);
inline static struct LangVar match_literal(struct AST* ast);

static struct ASTNode* parse_expr(struct AST* ast);
static struct VArray*  parse_expr_list(struct AST* ast, isize count);
static struct ASTNode  parse_call(struct AST* ast);
static struct ASTNode  parse_assign(struct AST* ast, enum ASTType type);
static struct ASTNode  parse_fun(struct AST* ast);

#define check_error_internal(_tok, _type) _check_error_internal(_tok, _type, __func__)
inline static void _check_error_internal(struct Token token, enum TokenType type, const char* fn);
noreturn static void error_unexpected(const char* type, struct Token token);
noreturn static void error_expected(const char* expect, struct Token token);

struct AST parser_parse(struct Tokenizer tknz)
{
	intptr max_expr = 10;
	struct AST ast = {
		.tknz  = &tknz,
		.nodec = 0,
		.nodes = smalloc(max_expr*sizeof(struct ASTNode)),

		.lits = varray_new(PARSER_DEFAULT_LITERAL_COUNT , sizeof(struct LangVar)),
		.vars = varray_new(PARSER_DEFAULT_VARIABLE_COUNT, sizeof(struct LangVar)),
		.lit_table = htable_new(PARSER_DEFAULT_LITERAL_COUNT),
		.var_table = htable_new(PARSER_DEFAULT_VARIABLE_COUNT),
	};

	struct Token tok;
	while (1) {
		tok = lexer_next(&tknz);
		switch (tok.type) {
		case TOKEN_VAL: ast.nodes[ast.nodec++] = parse_assign(&ast, AST_VAL); break;
		case TOKEN_VAR: ast.nodes[ast.nodec++] = parse_assign(&ast, AST_VAR); break;
		case TOKEN_FUN: ast.nodes[ast.nodec++] = parse_fun(&ast); break;
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
		for (int i = 0; i < node->params->len; i++)
			print_ast_rec(varray_get(node->params, i), spacec + 1);
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
	struct LangVar* var;
	printf("Literals:\n");
	for (int i = 0; i < ast.lits->len; i++) {
		var = varray_get(ast.lits, i);
		switch (var->type) {
		case LANG_INT: printf("\t[%d]: %ld" , i, var->val.s64);       break;
		case LANG_FLT: printf("\t[%d]: %.3f", i, var->val.flt);       break;
		case LANG_STR: printf("\t[%d]: %s"  , i, var->val.str->data); break;
		default:
			ERROR("[LANG] Literal should not have type %s (%d)", STRING_OF_TYPE(var->type), var->type);
		}
		printf("%s", (i + 1) % 4? "\t": "\n");
	}

	printf("\nVariables:\n");
	struct HPair pair;
	int lnc = 1;
	for (int i = 0; i < ast.var_table->cap; i++) {
		pair = ast.var_table->pairs[i];
		if (pair.key.data) {
			var = varray_get(ast.vars, pair.val);
			printf("\t[%d]: %s ([%s (%d)]: %ld)", i, pair.key.data, STRING_OF_TYPE(var->type), var->type, var->val.s64);
			printf("%s", lnc++ % 4? "\t": "\n");
		}
	}

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

inline static struct ASTNode* new_ident(struct AST* ast, struct Token tok)
{
	struct ASTNode* node = new_node();
	node->type   = AST_IDENT;
	node->lexeme = string_copy(tok.lexeme);

	return node;
}

inline static struct LangVar new_literal(struct AST* ast, struct Token tok)
{
	int i = htable_get(ast->lit_table, tok.lexeme);
	if (i != -1) {
		struct LangVar* var = varray_get(ast->lits, i);
		return (struct LangVar){
			.type    = var->type,
			.val.s64 = i,
		};
	}

	struct LangVar lit;
	if (tok.type == TOKEN_NUMBER) {
		/* Floats */
		if (string_contains(tok.lexeme, '.') != -1) {
			lit = (struct LangVar){
				.type    = LANG_FLT,
				.val.flt = atof(tok.lexeme.data),
			};
		/* Integers */
		} else {
			int64 val = atoll(tok.lexeme.data);
			lit = (struct LangVar){
				.type    = (val <= INT16_MAX && val >= INT16_MIN)? LANG_INT_LITERAL: LANG_INT,
				.val.s64 = val,
			};
		}
	} else if (tok.type == TOKEN_STRING) {
		lit = (struct LangVar){
			.type    = LANG_STR,
			.val.str = string_new_ptr(tok.lexeme.data, tok.lexeme.len),
		};
	} else {
		error_expected("literal value", tok);
	}

	/* Integers that fit into INT16 are added as literals, anything else is added to the literals table */
	if (lit.type != LANG_INT_LITERAL) {
		i = ast->lits->len;
		htable_insert(ast->lit_table, tok.lexeme, i);
		varray_push(ast->lits, &lit);
		return (struct LangVar){
			.type = lit.type,
			.val  = i,
		};
	} else {
		return lit;
	}
}

/* -------------------------------------------------------------------- */

inline static void match_eq(struct AST* ast)
{
	struct Token tok = lexer_next(ast->tknz);
	if(strncmp(tok.lexeme.data, "=", 1))
		error_expected("an equals (=)", tok);
}

inline static String match_ident(struct AST* ast)
{
	struct Token tok = lexer_next(ast->tknz);
	if (tok.type != TOKEN_IDENT)
		error_expected("identifier", tok);
	return tok.lexeme;
}

/* -------------------------------------------------------------------- */

static struct ASTNode* parse_expr(struct AST* ast)
{
	struct Token tok = lexer_next(ast->tknz);
	struct ASTNode* node = new_node();
	switch (tok.type) {
	case TOKEN_NUMBER:
	case TOKEN_STRING:
		struct LangVar lit = new_literal(ast, tok);
		node->type    = (enum ASTType)lit.type;
		node->literal = lit.val;
		node->lexeme  = string_copy(tok.lexeme);
		return node;
	case TOKEN_IDENT:
		node->type   = AST_CALL;
		node->lexeme = string_copy(tok.lexeme);
		node->params = parse_expr_list(ast, 1); // TODO: find # of params from tables
		return node;
		break;
	default:
		error_expected("expression", tok);
	}

	return NULL;
}

static struct VArray* parse_expr_list(struct AST* ast, isize count)
{
	if (count <= 0)
		ERROR("[LANG] Passed %ld to `parse_expr_list()`", count);

	struct VArray* list = varray_new(count, sizeof(struct ASTNode));
	for (int i = 0; i < count; i++) {
		varray_push(list, parse_expr(ast));
	}

	return list;
}

static struct ASTNode parse_assign(struct AST* ast, enum ASTType type)
{
	struct ASTNode node = {
		.type   = type,
		.lexeme = string_copy(match_ident(ast))
	};
	match_eq(ast);
	node.expr = parse_expr(ast);

	if (htable_get(ast->var_table, node.lexeme) != -1)
		ERROR("[LANG] Identifier \"%s\" already exists", node.lexeme.data);
	htable_insert(ast->var_table, node.lexeme,
		varray_push(ast->vars, &(struct LangVar){
			.type   = LANG_VAR,
			.val    = ast->vars->len,
			.params = NULL,
		})
	);

	return node;
}

static struct ASTNode parse_fun(struct AST* ast)
{
	struct ASTNode node = {
		.type   = AST_FUN,
		.lexeme = string_copy(match_ident(ast)),
	};
	// TODO: parse paramter list here
	match_eq(ast);
	node.expr = parse_expr(ast);

	if (htable_get(ast->var_table, node.lexeme) != -1)
		ERROR("[LANG] Identifier \"%s\" already exists", node.lexeme.data);
	htable_insert(ast->var_table, node.lexeme,
		varray_push(ast->vars, &(struct LangVar){
			.type   = LANG_FUN,
			.val    = ast->vars->len,
			.params = NULL,
		})
	);

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
