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

private:
	void dig();
	void rise();
	void pad();
	const std::string* findType(const std::string& str);

	std::unordered_map<std::string, std::string> types;
	std::string output;
	const RootAstNode& root;
	const std::string& path;
	uint32_t depth;
	bool mappingTypes;
	bool errorOccured;
};
