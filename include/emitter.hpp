#pragma once

#include "ast.hpp"

#include <unordered_map>

class Emitter : public AstVisitor{
public:
	Emitter(const RootAstNode& root, const std::string& path);
	bool operator()();

	void visit(const RootAstNode& node) final;
	void visit(const StructAstNode& node) final;
	void visit(const MemberAstNode& node) final;
	void visit(const TraitAstNode& node) final;
	void visit(const CodeAstNode& node) final;
	void visit(const SegmentAstNode& node) final;
	void visit(const MacroAstNode& node) final;

private:
	StructAstNode* findStruct(const std::string& str);

	void dig();
	void rise();
	void pad();
	const std::string* findType(const std::string& str);

	std::unordered_map<std::string, std::string> types;
	std::unordered_map<std::string, std::vector<std::string>> dependencies;
	std::unordered_map<std::string, bool> emitted;
	std::vector<std::string> collectedDependencies;
	std::string output;
	const RootAstNode& root;
	const std::string& path;
	const std::string* activeStruct;
	uint32_t depth;
	bool mappingTypes = false;
	bool mappingDeps = false;
	bool errorOccured = false;
};
