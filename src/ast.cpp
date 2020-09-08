#include "ast.hpp"

#include <iostream>

void AstNode::addChild(AstNode::Ptr child) {
	children.push_back(std::move(child));
}

void RootAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

StructAstNode::StructAstNode(const std::string& name) : name(name) {}

void StructAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

MemberAstNode::MemberAstNode(const std::string& type, const std::string& name) : type(type), name(name) {}

void MemberAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

RootAstNode::Ptr Parser::operator()() {
	auto root = std::make_unique<RootAstNode>();
	current = 0;
	last = tokens.size();

	while(current < last) {
		if(auto child = buildStruct(); child) {
			root->addChild(std::move(child));
		} else {
			// TODO: This is an error
			return nullptr;
		}
	}

	return root;
}

AstNode::Ptr Parser::buildStruct() {
	if(!getIf(TokenType::Struct)) {
		return nullptr;
	}

	const Token* name = getIf(TokenType::Identifier);
	if(!name) {
		// TODO: This is an error
		return nullptr;
	}
	
	if(!getIf(TokenType::LBrace)) {
		// TODO: This is an error
		return nullptr;
	}

	auto struc = std::make_unique<StructAstNode>(name->value);

	while(!getIf(TokenType::RBrace) ) {
		auto member = buildMember();
		if(!member) {
			// TODO: This is an error
			return nullptr;
		}
		struc->addChild(std::move(member));
	}

	return struc;
}

AstNode::Ptr Parser::buildMember() {
	const Token* type = getIf(TokenType::Identifier);
	if(type == nullptr) {
		// TODO: This is an error
		return nullptr;
	}

	const Token* name = getIf(TokenType::Identifier);
	if(name == nullptr) {
		// TODO: This is an error
		return nullptr;
	}

	return std::make_unique<MemberAstNode>(type->value, name->value);
}


const Token* Parser::getIf(TokenType type) {
	if(current >= last || tokens[current].type != type) {
		return nullptr;
	}
	return &tokens[current++];
}
