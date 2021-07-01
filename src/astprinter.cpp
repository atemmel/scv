#include "astprinter.hpp"

#include <iostream>

void AstPrinter::print(const RootAstNode& node) {
	visit(node);
}

void AstPrinter::visit(const RootAstNode& node) {
	pad();
	std::cout << "Root\n";
	dig();
	for(auto& child : node.children) {
		child->accept(*this);
	}
	rise();
}

void AstPrinter::visit(const StructAstNode& node) {
	pad();
	std::cout << "Struct: " << node.name;
	if(!node.traits.empty()) {
		std::cout << "\n";
		pad();
		std::cout << "traits (\n";
		dig();
		for(const auto& tr : node.traits) {
			pad();
			std::cout << tr << '\n';
		}
		rise();
		pad();
		std::cout << ")";
	}
	std::cout << '\n';
	dig();
	for(auto& child : node.children) {
		child->accept(*this);
	}
	rise();
}

void AstPrinter::visit(const MemberAstNode& node) {
	pad();
	std::cout << "Member: " << node.name << ", Type: " << node.type << '\n';
}

void AstPrinter::visit(const TraitAstNode& node) {
	pad();
	std::cout << "Trait: " << node.name << '\n';
	dig();
	for(auto& req : node.requirements) {
		pad();
		std::cout << "Requirement: " << req << '\n';
	}

	for(auto& child : node.children) {
		child->accept(*this);
	}

	rise();
}

void AstPrinter::visit(const CodeAstNode& node) {
	pad();
	std::cout << "Code:\n";
	dig();
	for(auto& child : node.children) {
		child->accept(*this);
	}
	rise();
}

void AstPrinter::visit(const SegmentAstNode& node) {
	pad();
	std::cout << "Segment: " << node.segment << '\n';
}

void AstPrinter::visit(const MacroAstNode& node) {
	pad();
	std::cout << "Macro: " << node.name << '\n';
	if(node.children.size() > 0) {
		dig();
		pad();
		std::cout << "Arguments:\n";
		dig();
		for(auto& child : node.children) {
			child->accept(*this);
		}
		rise();
		rise();
	}

	if(node.optionalCode) {
		pad();
		std::cout << "Optional code:\n";
		dig();
		node.optionalCode->accept(*this);
		rise();
	}
}


void AstPrinter::dig() {
	++depth;
}

void AstPrinter::rise() {
	--depth;
}

void AstPrinter::pad() const {
	for(uint32_t i = 0; i < depth; i++) {
		std::cout << "  ";
	}
}
