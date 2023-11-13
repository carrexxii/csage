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
	code.lits   = smalloc(code.lit_cap*sizeof(struct Literal));

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
		fprintf(stderr, "\t[%d]: ", i);
		switch (code.lits[i].type) {
		case LIT_INT : fprintf(stderr, "Integer: %ld", code.lits[i].integer);     break;
		case LIT_REAL: fprintf(stderr, "Real: %lf",    code.lits[i].real);        break;
		case LIT_STR : fprintf(stderr, "String:%s",    code.lits[i].string.data); break;
		default:
			ERROR("[LANG] %d is not a valid literal", code.lits[i].type);
		}
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

	struct Instruction instr;
	for (int i = 0; i < code.instrc; i++) {
		instr = code.instrs[i];
		// fprintf(stderr, "%s\t", STRING_OF_OP(instr.op));
		fprintf(stderr, "\t%s\t ", instr.op == OP_POP? "OP_POP": instr.op == OP_PUSH? "OP_PUSH": instr.op == OP_CALL? "OP_CALL": instr.op == OP_EOF? "OP_EOF": "<unknown op>");
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

	int i;
	switch (node->type) {
	case AST_INT:
	case AST_REAL:
	case AST_STR:
		i = htable_get(code->lit_table, node->lexeme);
		if (!i) {
			i = code->litc++;
			htable_insert(code->lit_table, node->lexeme, i);
			switch (node->type) {
			case AST_INT:
				code->lits[i].type    = LIT_INT;
				code->lits[i].integer = node->integer;
				break;
			case AST_REAL:
				code->lits[i].type = LIT_REAL;
				code->lits[i].real = node->real;
				break;
			case AST_STR:
				code->lits[i].type   = LIT_STR;
				code->lits[i].string = string_copy(node->lexeme);
				break;
			default:
				assert(false && "Unreachable");
			}
			DEBUG(5, "\t[LANG] Added new literal: %s (%d)", node->lexeme.data, i);
		}

		instr.operand = i;
		instr.is_var  = false;
		break;
	default:
		ERROR("[LANG] Unexpected token: %s", STRING_OF_NODE(node->type));
	}

	write_instruction(code, instr);
}

inline static void pop_var(struct ByteCode* code, struct ASTNode* node)
{
	assert(node->type == AST_IDENT);
	struct Instruction instr = {
		.op     = OP_POP,
		.is_var = true,
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
