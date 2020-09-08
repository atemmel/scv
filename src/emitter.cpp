#include "emitter.hpp"

#include "error.hpp"
#include "utils.hpp"

#include <fstream>

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
	output.append("\n\n");
	output.append(R"(#pragma once

#include <string>

)");
	errorOccured = false;

	// Map types
	mappingTypes = true;
	visit(root);

	if(errorOccured) {
		return false;
	}
	output.push_back('\n');

	// Write types
	mappingTypes = false;
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
	for(const auto& child : node.children) {
		child->accept(*this);
	}
}

void Emitter::visit(const StructAstNode& node) {
	output.append("struct ");
	output.append(node.name);

	if(mappingTypes) {
		output.append(";\n");
		auto shouldBeNull = findType(node.name);
		if(shouldBeNull != nullptr) {
			error::onToken("Type '" + node.name + "' already defined", *node.origin);
			errorOccured = true;
			return;
		} else {
			types[node.name] = node.name;
		}
	} else {
		output.append(" {\n");
		dig();
		for(const auto& child : node.children) {
			child->accept(*this);
		}
		rise();
		output.append("};\n\n");
	}
}

void Emitter::visit(const MemberAstNode& node) {
	auto type = findType(node.type);
	if(!type) {
		error::onToken("Type '" + node.type + "' not defined", *node.origin);
		errorOccured = true;
		return;
	}
	pad();
	output.append(*type);
	output.push_back(' ');
	auto name = findType(node.name);
	if(name) {
		error::onToken("Cannot name a member '" + node.name + "'", *node.nameToken);
		errorOccured = true;
		return;
	}
	output.append(node.name);
	output.append(";\n");
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
