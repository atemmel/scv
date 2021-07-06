#pragma once

#include "token.hpp"

#include <memory>
#include <string>
#include <string_view>
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

struct StructAstNode : public AstNode {
	StructAstNode(const Token* token);
	void accept(AstVisitor& visitor) final;
	std::string name;

	std::vector<std::string> traits;
};

struct MemberAstNode : public AstNode {
	MemberAstNode(const Token* type, const Token* name);
	void accept(AstVisitor& visitor) final;
	std::string type;
	std::string name;
	const Token* nameToken;
};

struct TraitAstNode : public AstNode {
	TraitAstNode(const Token* token);
	void accept(AstVisitor& visitor) final;
	std::string name;
	std::vector<std::string> requirements;
};

struct CodeAstNode : public AstNode {
	CodeAstNode(const Token* token);
	void accept(AstVisitor& visitor) final;
};

// Represents "unimportant" which lacks macros
struct SegmentAstNode : public AstNode {
	SegmentAstNode(const Token* token);
	void accept(AstVisitor& visitor) final;
	std::string_view segment;
};

// Starts with an @, is optionally followed by a sequence of args
// e.g: @Macro1, @Macro2(arg1, arg2)
struct MacroAstNode : public AstNode {
	MacroAstNode(const Token* token);
	void accept(AstVisitor& visitor) final;
	AstNode::Ptr optionalCode;
	std::string name;
};

struct RootAstNode : public AstNode {
	using Ptr = std::unique_ptr<RootAstNode>;

	RootAstNode();
	void accept(AstVisitor& visitor) final;

	void join(RootAstNode::Ptr& other);

	std::vector<StructAstNode*> structs;
	std::vector<TraitAstNode*> traits;
};

class AstVisitor {
public:
	virtual ~AstVisitor() = default;
	virtual void visit(const RootAstNode& node) = 0;
	virtual void visit(const StructAstNode& node) = 0;
	virtual void visit(const MemberAstNode& node) = 0;
	virtual void visit(const TraitAstNode& node) = 0;
	virtual void visit(const CodeAstNode& node) = 0;
	virtual void visit(const SegmentAstNode& node) = 0;
	virtual void visit(const MacroAstNode& node) = 0;
};

class Parser {
public:
	Parser(const std::vector<Token>& tokens, const std::string& src, const std::string_view originFile);
	RootAstNode::Ptr operator()();
private:
	AstNode::Ptr buildStruct();
	AstNode::Ptr buildMember();
	AstNode::Ptr buildTrait();
	AstNode::Ptr buildCodeBlock();
	AstNode::Ptr buildSegment(size_t &currentDepth);
	AstNode::Ptr buildMacro();
	bool buildRequire(RootAstNode::Ptr& root);
	std::vector<AstNode::Ptr> buildMacroArgList();

	std::vector<std::string> buildRequirements();
	std::string joinTokenValuesUntilToken(TokenType delim);
	const Token* getIf(TokenType type);
	const Token* getIfNot(TokenType type);
	bool eof() const;

	const std::vector<Token>& tokens;
	const std::string& src;
	const std::string_view originFile;
	size_t current;
	size_t last;
};
