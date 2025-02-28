#include "emitter.hpp"

#include "error.hpp"
#include "utils.hpp"

#include <fstream>
#include <iostream>

Emitter::Emitter(const RootAstNode& root, const std::string& path) : root(root), path(path), types(
{
	std::make_pair("int",    "int"),
	std::make_pair("i8",     "int8_t"),
	std::make_pair("i16",    "int16_t"),
	std::make_pair("i32",    "int32_t"),
	std::make_pair("i64",    "int64_t"),
	std::make_pair("u8",     "uint8_t"),
	std::make_pair("u16",    "uint16_t"),
	std::make_pair("u32",    "uint32_t"),
	std::make_pair("u64",    "uint64_t"),
	std::make_pair("byte",   "uint8_t"),
	std::make_pair("bool",   "bool"),
	std::make_pair("float",  "float"),
	std::make_pair("double", "double"),
	std::make_pair("f32",    "float"),
	std::make_pair("f64",    "double"),
	std::make_pair("string", "std::string"),
}) {}

bool Emitter::operator()() {
	depth = 0;
	output.reserve(64);
	output.append("// File autogenerated by scv on: ");
	output.append(getDate());
	output.append(R"(

#pragma once

#include <string>
)");
	errorOccured = false;
	emitted.reserve(root.structs.size());
	for(auto ptr : root.structs) {
		emitted[ptr->name] = false;
	}

	// Map deps
	state = MappingDeps;
	visit(root);

	if(errorOccured) {
		return false;
	}

	// Map types
	state = MappingTypes;
	visit(root);

	if(errorOccured) {
		return false;
	}

	state = MappingMembers;
	visit(root);

	if(errorOccured) {
		return false;
	}

	// Map traits
	state = MappingTraits;
	traits.reserve(root.traits.size());
	usedRequirements.reserve(root.traits.size());
	for(auto ptr : root.traits) {
		auto res = traits.try_emplace(ptr->name, ptr);
		if(!res.second) {
			error::onToken("Duplicate trait encountered", *ptr->origin);
			return false;
		}
	}
	visit(root);

	for(auto& req : usedRequirements) {
		output.append("#include ");
		output.append(req);
		output.append("\n");
	}

	output.append("\n");

	// Write types
	state = WritingTypes;
	visit(root);

	if(errorOccured) {
		return false;
	}

	// Write traits
	for(auto ptr : root.structs) {
		emitted[ptr->name] = false;
	}
	state = WritingTraits;
	outputResult = true;
	visit(root);

	if(errorOccured) {
		return false;
	}

	while(std::isspace(output.back())) {
		output.pop_back();
	}

	std::ofstream file(path.c_str());
	if(!file.is_open()) {
		error::set("Cannot open file '" + path + "'\n");
		return false;
	}

	file.write(output.c_str(), output.size());
	return true;
}

void Emitter::visit(const RootAstNode& node) {
	for(auto ptr : node.structs) {
		activeStructName = &ptr->name;
		visit(*ptr);
	}
}

void Emitter::visit(const StructAstNode& node) {
	const std::string* shouldBeNull;
	std::vector<std::string> deps;
	const TraitAstNode* trait;

	switch(state) {
		case MappingTypes:
			shouldBeNull = findType(node.name);
			if(shouldBeNull != nullptr) {
				error::onToken("Type '" + node.name + "' already defined", *node.origin);
				errorOccured = true;
				return;
			} else {
				types[node.name] = node.name;
			}
			break;
		case MappingDeps:
			for(const auto& child : node.children) {
				child->accept(*this);
			}
			dependencies.insert({node.name, std::move(collectedDependencies)});
			break;
		case MappingMembers:
			for(auto& child : node.children) {
				child->accept(*this);
			}
			break;
		case MappingTraits:
			for(auto& name : node.traits) {
				trait = findTrait(name);
				if(trait == nullptr) {
					error::onToken("Trait '" + name + "' requested is never defined", *node.origin);
					errorOccured = true;
					return;
				}

				for(auto& req : trait->requirements) {
					usedRequirements.insert(req);
				}
			}
			break;
		case WritingTypes:
			if(emitted[node.name]) {
				return;
			}

			deps = dependencies[node.name];
			for(const auto& dep : deps) {
				auto stru = findStruct(dep);
				if(stru->name == *activeStructName) {
					error::onToken("Cyclic dependency detected inside struct", *node.origin);
					errorOccured = true;
					return;
				}
				visit(*stru);
			}

			output.append("struct ");
			output.append(node.name);
			output.append(" {\n");
			dig();
			for(const auto& child : node.children) {
				child->accept(*this);
			}
			rise();
			output.append("};\n\n");
			emitted[node.name] = true;
			break;
		case WritingTraits:
			if(emitted[node.name]) {
				return;
			}
			emitted[node.name] = true;
			auto& deps = dependencies[node.name];
			for(auto& dep : deps) {
				if(!emitted[dep]) {
					auto stru = findStruct(dep);
					visit(*stru);
				}
			}

			activeStruct = &node;
			for(auto& name : node.traits) {
				trait = findTrait(name);
				visit(*trait);
			}
			break;
	}
}

void Emitter::visit(const MemberAstNode& node) {
	const std::string* type;
	const std::string* name;

	switch(state) {
		case MappingDeps:
			type = findType(node.type);
			if(type == nullptr) {
				collectedDependencies.push_back(node.type);
			}

			return;
			break;
		case MappingMembers:
			type = findType(node.type);
			if(type == nullptr) {
				error::onToken("Type '" + node.type + "' not defined", *node.origin);
				errorOccured = true;
				return;
			}
			break;
		case WritingTypes:
			type = findType(node.type);
			if(type == nullptr) {
				error::onToken("Type '" + node.type + "' not defined", *node.origin);
				errorOccured = true;
				return;
			}
			pad();
			output.append(*type);
			output.push_back(' ');
			name = findType(node.name);
			if(name) {
				error::onToken("Cannot name a member '" + node.name + "'", *node.nameToken);
				errorOccured = true;
				return;
			}
			output.append(node.name);
			output.append(";\n");
			break;
		case WritingTraits:
			collected = node.name;
			break;
	}
}

void Emitter::visit(const TraitAstNode& node) {
	for(auto& child : node.children) {
		child->accept(*this);
	}
}

void Emitter::visit(const CodeAstNode& node) {
	std::string sum;
	for(auto& child : node.children) {
		child->accept(*this);
		if(!outputResult) {
			sum += collected;
		}
	}

	if(!outputResult) {
		collected = sum;
	} else {
		output.append("\n");
	}
}

void Emitter::visit(const SegmentAstNode& node) {
	if(outputResult) {
		output.append(node.segment);
	} else {
		collected = node.segment;
	}
}

void Emitter::visit(const MacroAstNode& node) {
	std::string result;
	if(node.name == "Type") {
		result = doTypeMacro(node);
	} else if(node.name == "ForMemberIn") {
		result = doForMemberInMacro(node);
	} else if(node.name == "Member") {
		activeStruct->children[currentMember]->accept(*this);
		result = collected;
		result += ' ';
	} else {
		error::onToken("Unrecognized macro: '" + node.name + "'", *node.origin);
	}

	if(outputResult) {
		output.append(result);
	} else {
		collected = result;
	}
}

std::string Emitter::doTypeMacro(const MacroAstNode& node) {
	return activeStruct->name;
}

std::string Emitter::doForMemberInMacro(const MacroAstNode& node) {
	if(node.children.size() != 1) {
		error::onToken("Macro of type 'ForMemberIn' requires exactly 1 argument, " + std::to_string(node.children.size()) + " provided", *node.origin);
		errorOccured = true;
		return "";
	}

	if(!node.optionalCode) {
		error::onToken("Macro of type 'ForMemberIn' requires a code block attached to it, none provided", *node.origin);
		errorOccured= true;
		return "";
	}

	auto prevState = outputResult;
	outputResult = false;
	node.children.front()->accept(*this);

	const StructAstNode* requested = nullptr;
	for(auto* struc : root.structs) {
		if(struc->name == collected) {
			requested = struc;
		}
	}

	if(requested == nullptr) {
		error::onToken("Can not find type with name '" + collected + "'", *node.children.front()->origin);
		errorOccured = true;
		return "";
	}

	auto prevActiveStruct = activeStruct;
	activeStruct = requested;

	currentMember = 0;
	std::string sum;
	for(; currentMember < requested->children.size(); currentMember++) {
		node.optionalCode->accept(*this);
		sum += collected;
	}
	outputResult = prevState;

	return sum;
}

StructAstNode* Emitter::findStruct(const std::string& str) {
	for(auto stru : root.structs) {
		if(stru->name == str) {
			return stru;
		}
	}
	return nullptr;
}

void Emitter::dig() {
	++depth;
}

void Emitter::rise() {
	--depth;
}

void Emitter::pad() {
	for(uint32_t i = 0; i < depth; i++) {
		output.push_back('\t');
	}
}

const std::string* Emitter::findType(const std::string& str) {
	auto it = types.find(str);
	if(it == types.cend()) {
		return nullptr;
	}
	return &it->second;
}

const TraitAstNode* Emitter::findTrait(const std::string& str) {
	auto it = traits.find(str);
	if(it == traits.cend()) {
		return nullptr;
	}
	return it->second;
}
