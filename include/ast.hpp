#pragma once

#include "token.hpp"

#include <memory>
#include <string>
#include <vector>

class AstVisitor;

struct AstNode {
	AstNode(const Token* token);
	using Ptr = std::unique_ptr<AstNode>;

	virtual ~AstNode() = default;
	virtual void accept(AstVisitor& visitor) = 0;
	void addChild(Ptr child);
	std::vector<Ptr> children;
	const Token* origin;
};

struct RootAstNode : public AstNode {
	using Ptr = std::unique_ptr<RootAstNode>;

	RootAstNode();
	void accept(AstVisitor& visitor) final;
};

struct StructAstNode : public AstNode {
	StructAstNode(const Token* token);
	void accept(AstVisitor& visitor) final;
	std::string name;
};

struct MemberAstNode : public AstNode {
	MemberAstNode(const Token* type, const Token* name);
	void accept(AstVisitor& visitor) final;
	std::string type;
	std::string name;
	const Token* nameToken;
};

class AstVisitor {
public:
	virtual ~AstVisitor() = default;
	virtual void visit(const RootAstNode& node) = 0;
	virtual void visit(const StructAstNode& node) = 0;
	virtual void visit(const MemberAstNode& node) = 0;
};

class Parser {
public:
	Parser(const std::vector<Token>& tokens);
	RootAstNode::Ptr operator()();
private:
	AstNode::Ptr buildStruct();
	AstNode::Ptr buildMember();
	const Token* getIf(TokenType type);

	const std::vector<Token>& tokens;
	size_t current;
	size_t last;
};
