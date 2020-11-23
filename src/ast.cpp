#include "ast.hpp"

#include "error.hpp"

AstNode::AstNode(const Token* token) : origin(token) {}

void AstNode::addChild(AstNode::Ptr child) {
	children.push_back(std::move(child));
}

RootAstNode::RootAstNode() : AstNode(nullptr) {}

void RootAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

StructAstNode::StructAstNode(const Token* token) : name(token->value), AstNode(token) {}

void StructAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

MemberAstNode::MemberAstNode(const Token* type, const Token* name) : type(type->value), name(name->value), AstNode(type), nameToken(name) {}

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
			auto ptr = static_cast<StructAstNode*>(child.get());
			root->structs.push_back(ptr);
			root->addChild(std::move(child));
		} else {
			// Let error bubble up
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
		error::onToken("Expected identifier", tokens[current]);
		return nullptr;
	}

	auto struc = std::make_unique<StructAstNode>(name);

	if(getIf(TokenType::Is)) {
		while(1) {
			const Token* trait = getIf(TokenType::Identifier);
			if(!trait) {
				error::onToken("Expected trait name", tokens[current]);
				return nullptr;
			}
			struc->traits.push_back(trait->value);

			if(!getIf(TokenType::Comma)) {
				break;
			}
		}
		
	}
	
	if(!getIf(TokenType::LBrace)) {
		error::onToken("Expected '{'", tokens[current]);
		return nullptr;
	}

	while(!getIf(TokenType::RBrace) ) {
		auto member = buildMember();
		if(!member) {
			// Let error bubble up
			return nullptr;
		}
		struc->addChild(std::move(member));
		if(current >= last) {
			error::onToken("Expected '}'", tokens[current]);
		}
	}

	return struc;
}

AstNode::Ptr Parser::buildMember() {
	const Token* type = getIf(TokenType::Identifier);
	if(type == nullptr) {
		error::onToken("Expected identifier", tokens[current]);
		return nullptr;
	}

	const Token* name = getIf(TokenType::Identifier);
	if(name == nullptr) {
		error::onToken("Expected identifier", tokens[current]);
		return nullptr;
	}

	return std::make_unique<MemberAstNode>(type, name);
}


const Token* Parser::getIf(TokenType type) {
	if(current >= last || tokens[current].type != type) {
		return nullptr;
	}
	return &tokens[current++];
}
