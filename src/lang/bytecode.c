#include "util/htable.h"
#include "util/varray.h"
#include "parser.h"
#include "bytecode.h"

inline static void check_resize(struct ByteCode* code);
inline static int  add_literal(struct ByteCode* code, struct ASTNode* node);

inline static void push_expr(struct ByteCode* code, struct ASTNode* node);
inline static void pop_var(struct ByteCode* code, String var);

inline static void write_instr(struct ByteCode* code, struct Instruction instr);
inline static void write_assign(struct ByteCode* code, struct ASTNode* node);
inline static void write_fun(struct ByteCode* code, struct ASTNode* node);
inline static void write_call(struct ByteCode* code, struct ASTNode* node);

noreturn static void error_expected(const char* expect, struct ASTNode* node);

struct ByteCode bytecode_generate(struct AST* ast)
{
	struct ByteCode code = {
		.instrs    = varray_new(BYTECODE_DEFAULT_SIZE, sizeof(struct Instruction)),
		.lits      = ast->lits,
		.vars      = ast->vars,
		.lit_table = ast->lit_table,
		.var_table = ast->var_table,
	};

	struct ASTNode* node;
	for (int n = 0; n < ast->nodec; n++) {
		node = &ast->nodes[n];
		switch (node->type) {
		case AST_VAL:
		case AST_VAR:
			write_assign(&code, node);
			break;
		case AST_FUN:
			write_fun(&code, node);
			break;
		case AST_CALL: // TODO: move
			write_call(&code, node);
			break;
		default:
			ERROR("[LANG] Bytecode generator: skipped node [%s]", STRING_OF_NODE(node->type));
			exit(70);
		}
	}

	write_instr(&code, (struct Instruction){ .op = OP_EOF });

	return code;
}

void bytecode_print(struct ByteCode code)
{
	fprintf(stderr, "--- ByteCode (%ld Instructions) ---\n", code.instrs->len);
	fprintf(stderr, "Literals:\n");
	union LangVal* lit;
	for (int i = 0; i < code.lits->len; i++) {
		lit = varray_get(code.lits, i);
		fprintf(stderr, "\t[%d]: %ld (%f)", i, lit->s64, lit->flt);
		fprintf(stderr, i % 2? "\n": "\t");
	}
	fprintf(stderr, "\n   ----------------------\n");

	fprintf(stderr, "Variables:\n");
	struct LangVar* var;
	for (int i = 0; i < code.vars->len; i++) {
		var = varray_get(code.vars, i);
		fprintf(stderr, "\t[%d]: %s%s", i, var->val.str->data, i % 2? "\n": "\t");
	}
	fprintf(stderr, "\n   ----------------------\n");

	fprintf(stderr, "\n\t  [OP]\t[Operand]\n");
	struct Instruction* instr;
	for (int i = 0; i < code.instrs->len; i++) {
		instr = varray_get(code.instrs, i);
		// fprintf(stderr, "%s\t", STRING_OF_OP(instr.op));
		fprintf(stderr, "[%02d]\t%s\t ", i + 1, instr->op == OP_POP? "OP_POP": instr->op == OP_PUSH? "OP_PUSH": instr->op == OP_CALL? "OP_CALL": instr->op == OP_EOF? "OP_EOF": instr->op == OP_RET? "OP_RET": "<unknown op>");
		fprintf(stderr, "%d\n", instr->operand);
	}
}

/* -------------------------------------------------------------------- */

inline static void push_expr(struct ByteCode* code, struct ASTNode* node)
{
	struct Instruction instr = {
		.op = OP_PUSH,
	};

	// TODO: Move this to the parser
	/* Integers that fit in INT16 are added as literals, anything else is added to the literals table */
	if (node->type == AST_INT && node->literal.s64 <= INT16_MAX && node->literal.s64 >= INT16_MIN) {
		instr.type    = LANG_INT_LITERAL;
		instr.operand = node->literal.s64;
	} else {
		int i;
		switch (node->type) {
		case AST_INT: instr.type = LANG_INT; [[fallthrough]];
		case AST_FLT: instr.type = LANG_FLT; [[fallthrough]];
		case AST_STR: instr.type = LANG_STR;
			i = htable_get(code->lit_table, node->lexeme);
			if (i == -1) {
				htable_insert(code->lit_table, node->lexeme, i);
				switch (node->type) {
				// case AST_INT: code->lits[i].val.s64 = node->literal.s64; break;
				// case AST_FLT: code->lits[i].val.flt = node->literal.flt; break;
				// case AST_STR: code->lits[i].val.str = string_new_ptr(node->lexeme.data, node->lexeme.len); break;
				default:
					assert(false && "Unreachable");
				}
				DEBUG(5, "\t[LANG] Added new literal: %s (%d)", node->lexeme.data, i);
			}

			instr.operand = i;
			break;
		case AST_CALL:
			i = htable_get(code->var_table, node->lexeme);
			if (i == -1) {
				ERROR("[LANG] Bytecode generator: could not find identifier \"%s\" in functions table", node->lexeme.data);
				exit(70);
			}
			break;
		default:
			error_expected("expression (push_expr)", node);
		}
	}

	write_instr(code, instr);
}

inline static void pop_var(struct ByteCode* code, String var)
{
	struct Instruction instr = {
		.op = OP_POP,
	};

	// int i = htable_get_or_insert(code->var_table, var, code->vars->len);
	// if (i == code->vars->len)
		// code->varc++;
	// instr.operand = i;

	write_instr(code, instr);
}

/* -------------------------------------------------------------------- */

inline static void write_instr(struct ByteCode* code, struct Instruction instr) {
	varray_push(code->instrs, &instr);
}

inline static void write_assign(struct ByteCode* code, struct ASTNode* node)
{
	push_expr(code, node->expr);
	pop_var(code, node->lexeme);
}

inline static void write_call(struct ByteCode* code, struct ASTNode* node)
{
	struct Instruction instr = {
		.op = OP_CALL,
	};

	for (int i = 0; i < node->params->len; i++)
		push_expr(code, varray_get(node->params, i));

	// int i = code->varc++;
	// htable_insert(code->var_table, node->lexeme, i);
	instr.operand = 0xFF;

	write_instr(code, instr);
}

inline static void write_fun(struct ByteCode* code, struct ASTNode* node)
{
	// htable_insert(code->var_table, node->lexeme, code->instrc);

	push_expr(code, node->expr);

	write_instr(code, (struct Instruction){ .op = OP_RET });
}

/* -------------------------------------------------------------------- */

noreturn static void error_expected(const char* expect, struct ASTNode* node)
{
	ERROR("[LANG] Bytecode generator: expected %s but got \"%s\"", expect, STRING_OF_NODE(node->type));
	exit(70);
}
