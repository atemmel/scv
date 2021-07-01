#pragma once

#include "ast.hpp"

class AstPrinter : public AstVisitor {
public:
	void print(const RootAstNode& node);
	void visit(const RootAstNode& node) final;
	void visit(const StructAstNode& node) final;
	void visit(const MemberAstNode& node) final;
	void visit(const TraitAstNode& node) final;
	void visit(const CodeAstNode& node) final;
	void visit(const SegmentAstNode& node) final;
	void visit(const MacroAstNode& node) final;
private:
	void dig();
	void rise();
	void pad() const;
	uint32_t depth = 0;
};
