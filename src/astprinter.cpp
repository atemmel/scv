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
