#include "parser.h"
#include "bytecode.h"

#define BYTECODE_DEFAULT_SIZE    1024
#define BYTECODE_SIZE_MULTIPLIER 2

struct Bytecode {
	byte*  data;
	byte*  sp;
	intptr capacity;
};

inline static void write_op(struct Bytecode* code, void* data, intptr sz);
inline static void write_assign(struct Bytecode* code, struct ASTNode* node);

void bytecode_generate(struct AST ast)
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
}

/* -------------------------------------------------------------------- */

inline static void write_op(struct Bytecode* code, void* data, intptr sz)
{
	DEBUG(1, "Writing %ld bytes", sz);
	/* This fails if more than one resize is needed */
	intptr offset = code->sp - code->data;
	if (offset >= code->capacity) {
		code->capacity *= BYTECODE_SIZE_MULTIPLIER;
		code->data = srealloc(code->data, code->capacity);
		code->sp   = code->data + offset;
	}
	memcpy(code->sp, data, sz);
	code->sp += sz;
}

inline static void push_expr(struct Bytecode* code, struct ASTNode* node)
{
	write_op(code, (int[]){ OP_PUSH }, 1);
	switch (node->type) {
	case AST_INT : write_op(code, &node->integer   , sizeof(int64));    break;
	case AST_REAL: write_op(code, &node->real      , sizeof(double));   break;
	case AST_STR : write_op(code, node->string.data, node->string.len); break;
	default:
		ERROR("[LANG] Unexpected token: %s", STRING_OF_NODE(node->type));
	}
}

inline static void write_assign(struct Bytecode* code, struct ASTNode* node)
{
	push_expr(code, node->right);
	write_op(code, (int[]){ OP_POP }, 1);
	write_op(code, node->left->ident.data, node->left->ident.len); // !!
}
