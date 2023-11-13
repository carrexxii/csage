#include "util/htable.h"
#include "parser.h"
#include "bytecode.h"

inline static void check_resize(struct ByteCode* code);
inline static int  add_literal(struct ByteCode* code, struct ASTNode* node);
inline static void write_instruction(struct ByteCode* code, struct Instruction instr);
inline static void write_assign(struct ByteCode* code, struct ASTNode* node);
inline static void write_call(struct ByteCode* code, struct ASTNode* node);

struct ByteCode bytecode_generate(struct AST ast)
{
	struct ByteCode code = {
		.instr_cap = BYTECODE_DEFAULT_SIZE,
		.lit_cap   = BYTECODE_LITERAL_COUNT,
		.lit_table = htable_new(BYTECODE_HTABLE_SIZE),
		.var_table = htable_new(BYTECODE_HTABLE_SIZE),
	};
	code.instrs = smalloc(code.instr_cap*sizeof(struct Instruction));
	code.lits   = smalloc(code.lit_cap*sizeof(union LangVal));

	struct ASTNode* node;
	for (int n = 0; n < ast.nodec; n++) {
		node = &ast.nodes[n];
		switch (node->type) {
		case AST_ASSIGN:
			write_assign(&code, node);
			break;
		case AST_CALL: // TODO: move
			write_call(&code, node);
			break;
		default:
			ERROR("[LANG] Skipped node [%s]", STRING_OF_NODE(node->type));
		}
	}

	write_instruction(&code, (struct Instruction){ .op = OP_EOF });

	return code;
}

void bytecode_print(struct ByteCode code)
{
	fprintf(stderr, "--- ByteCode (%d Instructions) ---\n", code.instrc);
	fprintf(stderr, "Literals:\n");
	for (int i = 0; i < code.litc; i++) {
		fprintf(stderr, "\t[%d]: %ld (%f)", i, code.lits[i].s64, code.lits[i].dbl);
		fprintf(stderr, i % 2? "\n": "\t");
	}
	fprintf(stderr, "\n   ----------------------\n");

	fprintf(stderr, "Variables:\n");
	for (int i = 0; i < code.var_table->cap; i++) {
		char* v = code.var_table->pairs[i].key.data;
		if (!v)
			continue;
		fprintf(stderr, "\t[%d]: %s%s", i, v, i % 2? "\n": "\t");
	}
	fprintf(stderr, "\n   ----------------------\n");

	fprintf(stderr, "\n\t  [OP]\t[Operand]\n");
	struct Instruction instr;
	for (int i = 0; i < code.instrc; i++) {
		instr = code.instrs[i];
		// fprintf(stderr, "%s\t", STRING_OF_OP(instr.op));
		fprintf(stderr, "[%02d]\t%s\t ", i + 1, instr.op == OP_POP? "OP_POP": instr.op == OP_PUSH? "OP_PUSH": instr.op == OP_CALL? "OP_CALL": instr.op == OP_EOF? "OP_EOF": "<unknown op>");
		fprintf(stderr, "%d\n", instr.operand);
	}
}

void bytecode_free(struct ByteCode code)
{
	sfree(code.lits);
	sfree(code.instrs);
	htable_free(code.lit_table);
	htable_free(code.var_table);
}

/* -------------------------------------------------------------------- */

inline static void check_resize(struct ByteCode* code)
{
	if (code->instrc + 1 >= code->instr_cap) {
		code->instr_cap *= BYTECODE_SIZE_MULTIPLIER;
		code->instrs = srealloc(code->instrs, code->instr_cap);
	}

	if (code->litc + 1 >= code->lit_cap) {
		code->lit_cap *= BYTECODE_SIZE_MULTIPLIER;
		code->lits = srealloc(code->lits, code->lit_cap);
	}
}

inline static void write_instruction(struct ByteCode* code, struct Instruction instr)
{
	check_resize(code);
	code->instrs[code->instrc++] = instr;
}

inline static void push_expr(struct ByteCode* code, struct ASTNode* node)
{
	struct Instruction instr = {
		.op = OP_PUSH,
	};

	/* Integers that fit in INT16 are added as literals, anything else is added to the literals table */
	if (node->type == AST_INT && node->integer <= INT16_MAX && node->integer >= INT16_MIN) {
		instr.type    = LANG_INT_LITERAL;
		instr.operand = node->integer;
	} else {
		int i;
		switch (node->type) {
		case AST_INT : instr.type = LANG_INT; [[fallthrough]];
		case AST_REAL: instr.type = LANG_FLT; [[fallthrough]];
		case AST_STR : instr.type = LANG_STR;
			i = htable_get(code->lit_table, node->lexeme);
			if (!i) {
				i = code->litc++;
				htable_insert(code->lit_table, node->lexeme, i);
				switch (node->type) {
				case AST_INT:
					// code->lits[i].type    = LIT_INT;
					code->lits[i].s64 = node->integer;
					break;
				case AST_REAL:
					// code->lits[i].type = LIT_REAL;
					code->lits[i].dbl = node->real;
					break;
				case AST_STR:
					// code->lits[i].type   = LIT_STR;
					code->lits[i].str = string_copy(node->lexeme).data;
					break;
				default:
					assert(false && "Unreachable");
				}
				DEBUG(5, "\t[LANG] Added new literal: %s (%d)", node->lexeme.data, i);
			}

			instr.operand = i;
			break;
		default:
			ERROR("[LANG] Unexpected token: %s", STRING_OF_NODE(node->type));
			exit(1);
		}
	}

	write_instruction(code, instr);
}

inline static void pop_var(struct ByteCode* code, struct ASTNode* node)
{
	assert(node->type == AST_IDENT);
	struct Instruction instr = {
		.op     = OP_POP,
		// .is_var = true,
	};

	int i = htable_get(code->var_table, node->lexeme);
	if (!i) {
		i = code->varc++;
		htable_insert(code->var_table, node->lexeme, i);
		DEBUG(5, "\t[LANG] Adding variable: %s (%d)", node->lexeme.data, i);
	}
	instr.operand = i;

	write_instruction(code, instr);
}

inline static void write_assign(struct ByteCode* code, struct ASTNode* node)
{
	push_expr(code, node->right);
	pop_var(code, node->left);
}

inline static void write_call(struct ByteCode* code, struct ASTNode* node)
{
	struct Instruction instr = {
		.op = OP_CALL,
	};

	for (int i = 0; i < node->paramc; i++)
		push_expr(code, node->params[i]);

	// int i = code->varc++;
	// htable_insert(code->var_table, node->lexeme, i);
	instr.operand = 0xFF;

	write_instruction(code, instr);
}
