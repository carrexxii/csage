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
	printf("\n\t  [OP]\t[Operand]\n");
	struct Instruction* instr;
	for (int i = 0; i < code.instrs->len; i++) {
		instr = varray_get(code.instrs, i);
		printf("[%02d]\t%s\t ", i + 1, STRING_OF_OP(instr->op));
		// fprintf(stderr, "[%02d]\t%s\t ", i + 1, instr->op == OP_POP? "OP_POP": instr->op == OP_PUSH? "OP_PUSH": instr->op == OP_CALL? "OP_CALL": instr->op == OP_EOF? "OP_EOF": instr->op == OP_RET? "OP_RET": "<unknown op>");
		printf("%d\n", instr->operand);
	}
}

/* -------------------------------------------------------------------- */

inline static void push_expr(struct ByteCode* code, struct ASTNode* node)
{
	// DEBUG_VALUE(node->lexeme.data);
	// DEBUG_VALUE(node->literal.s64);
	struct Instruction instr = { 0 };
	switch (node->type) {
	case AST_INT:
	case AST_FLT:
	case AST_STR:
	case AST_INT_LITERAL:
		instr.op      = OP_PUSH,
		instr.type    = (enum LangType)node->type; /* Literal types match for the enums */
		instr.operand = node->literal.s64;
		break;
	case AST_CALL:
		instr.op      = OP_CALL;
		instr.type    = LANG_FUN;
		instr.operand = htable_get(code->var_table, node->lexeme);
		if (instr.operand == -1)
			ERROR("{LANG] Failed to find function index for \"%s\"", node->lexeme.data);
		break;
	default:
		ERROR("[LANG] Unhandled expression: %s (%d) \"%s\"",
		      STRING_OF_TYPE((enum LangType)node->type), node->type, node->lexeme.data);
	}

	write_instr(code, instr);
}

inline static void pop_var(struct ByteCode* code, String name)
{
	struct LangVar* var = varray_get(code->vars, htable_get(code->var_table, name));
	write_instr(code, (struct Instruction){
		.op      = OP_POP,
		.type    = var->type,
		.operand = var->val.s64,
	});
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
