#include "util/htable.h"
#include "parser.h"
#include "bytecode.h"

inline static void check_resize(struct ByteCode* code);
inline static int  add_literal(struct ByteCode* code, struct ASTNode* node);
inline static void write_instruction(struct ByteCode* code, struct Instruction instr);
inline static void write_assign(struct ByteCode* code, struct ASTNode* node);

struct ByteCode bytecode_generate(struct AST ast)
{
	struct ByteCode code = {
		.instr_cap = BYTECODE_DEFAULT_SIZE,
		.instrs    = smalloc(code.instr_cap*sizeof(struct Instruction)),
		.lit_cap   = BYTECODE_LITERAL_COUNT,
		.lits      = smalloc(code.lit_cap*sizeof(struct Literal)),
		.lit_table = htable_new(BYTECODE_HTABLE_SIZE),
		.var_table = htable_new(BYTECODE_HTABLE_SIZE),
	};

	struct ASTNode* node;
	for (int n = 0; n < ast.nodec; n++) {
		node = &ast.nodes[n];
		switch (node->type) {
		case AST_ASSIGN:
			write_assign(&code, node);
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
	fprintf(stderr, "--- ByteCode (%dB) ---\n", code.instrc);
	struct Instruction instr;
	for (int i = 0; i < code.instrc; i++) {
		instr = code.instrs[i];
		// fprintf(stderr, "%s\t", STRING_OF_CODE(instr.op));
		fprintf(stderr, "\t%s\t", instr.op == OP_POP? "OP_POP": instr.op == OP_PUSH? "OP_PUSH": instr.op == OP_EOF? "OP_EOF": "<unknown op>");
		fprintf(stderr, "%d\n", instr.operand);
	}
}

void bytecode_free(struct ByteCode code)
{
	sfree(code.lits);
	sfree(code.instrs);
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

inline static int add_literal(struct ByteCode* code, struct ASTNode* node)
{
	assert(node->type == AST_INT || node->type == AST_REAL || node->type == AST_STR);
	check_resize(code);

	// htable_insert(&code->lit_table, node->lexeme, );
	int i = code->litc++;
	struct Literal* lit = &code->lits[i];
	switch (node->type) {
	case AST_INT : lit->integer = node->integer; break;
	case AST_REAL: lit->real    = node->real;    break;
	case AST_STR : lit->string  = string_copy(node->string); break;
	default:
		ERROR("[LANG] Invalid type for a literal: %s", STRING_OF_NODE(node->type));
	}

	return i;
}

inline static void write_instruction(struct ByteCode* code, struct Instruction instr)
{
	DEBUG(1, "[%d] Writing instruction: %d  %d (%s)", code->instrc, instr.op, instr.operand, STRING_TF(instr.is_var));
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
			DEBUG(5, "\t[LANG] Added new literal: %s", node->lexeme->data);
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
		.op = OP_POP,
	};

	int i = htable_get(code->var_table, node->ident);
	if (!i) {
		i = code->varc++;
		htable_insert(code->var_table, node->lexeme, i);
		DEBUG(5, "Adding variable: %s", node->lexeme->data);
	}
	instr.operand = i;

	write_instruction(code, instr);
}

inline static void write_assign(struct ByteCode* code, struct ASTNode* node)
{
	push_expr(code, node->right);
	pop_var(code, node->left);
}
