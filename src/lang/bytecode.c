#include "util/htable.h"
#include "parser.h"
#include "bytecode.h"

#define BYTECODE_DEFAULT_SIZE    1024
#define BYTECODE_SIZE_MULTIPLIER 2

inline static void check_resize(struct Bytecode* code);
inline static void write_operand(struct Bytecode* code, void* data);
inline static void write_op(struct Bytecode* code, enum BytecodeOp op);
inline static void write_assign(struct Bytecode* code, struct ASTNode* node);

struct Bytecode bytecode_generate(struct AST ast)
{
	struct Bytecode code;
	code.capacity = BYTECODE_DEFAULT_SIZE;
	code.data     = smalloc(code.capacity);
	code.sp       = code.data;

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

	return code;
}

void bytecode_print(struct Bytecode code)
{
	fprintf(stderr, "--- Bytecode (%ldB) ---\n", code.sp - code.data);
	for (byte* i = code.data; i < code.sp; i += 9) {
		// fprintf(stderr, "%s\t", STRING_OF_CODE(*i));
		fprintf(stderr, "\t%s\t", *i == OP_POP? "OP_POP": *i == OP_PUSH? "OP_PUSH": "<unknown op>");
		fprintf(stderr, "%ld\n", *(int64*)(i + 1));
	}
}

/* -------------------------------------------------------------------- */

inline static void check_resize(struct Bytecode* code)
{
	intptr offset = code->sp - code->data;
	if (offset + 8 >= code->capacity) {
		code->capacity *= BYTECODE_SIZE_MULTIPLIER;
		code->data = srealloc(code->data, code->capacity);
		code->sp   = code->data + offset;
	}
}

inline static void write_operand(struct Bytecode* code, void* data)
{
	DEBUG(1, "Writing 8 bytes");
	check_resize(code);
	memcpy(code->sp, data, 8);
	code->sp += 8;
}

inline static void write_op(struct Bytecode* code, enum BytecodeOp op)
{
	DEBUG(1, "Writing 1 byte");
	check_resize(code);
	memcpy(code->sp, (int8[]){ (int8)op }, 1);
	code->sp++;
}

inline static void push_expr(struct Bytecode* code, struct ASTNode* node)
{
	write_op(code, OP_PUSH);
	switch (node->type) {
	case AST_INT : write_operand(code, &node->integer    ); break;
	case AST_REAL: write_operand(code, &node->real       ); break;
	case AST_STR : write_operand(code, node->string->data); break;
	default:
		ERROR("[LANG] Unexpected token: %s", STRING_OF_NODE(node->type));
	}
}

inline static void write_assign(struct Bytecode* code, struct ASTNode* node)
{
	push_expr(code, node->right);
	write_op(code, OP_POP);
	write_operand(code, node->left->ident->data);
}
