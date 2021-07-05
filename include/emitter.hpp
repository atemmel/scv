#pragma once

#include "ast.hpp"

#include <unordered_map>
#include <unordered_set>

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
	std::string doTypeMacro(const MacroAstNode& node);
	std::string doForMemberInMacro(const MacroAstNode& node);
	const std::string* findType(const std::string& str);
	const TraitAstNode* findTrait(const std::string& str);

	std::unordered_map<std::string, std::string> types;
	std::unordered_map<std::string, std::vector<std::string>> dependencies;
	std::unordered_map<std::string, bool> emitted;
	std::unordered_map<std::string, TraitAstNode*> traits;
	std::vector<std::string> collectedDependencies;
	std::unordered_set<std::string> usedRequirements;
	std::string output;
	std::string collected;
	const RootAstNode& root;
	const std::string& path;
	const std::string* activeStructName;
	const StructAstNode* activeStruct;
	uint32_t depth;
	uint32_t state;
	size_t currentMember;
	bool errorOccured = false;
	bool outputResult;
	
	enum {
		MappingTypes,
		MappingDeps,
		MappingMembers,
		MappingTraits,
		WritingTypes,
		WritingTraits,
	};
};
