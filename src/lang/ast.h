#ifndef LANG_AST_H
#define LANG_AST_H

enum ASTNodeType {
	ASTNODE_TYPE_NONE,
};

union ASTAttribute {
	int type;
};

struct ASTNode {
	enum ASTNodeType   type;
	union ASTAttribute attr;
	int symbol;
};

#endif
