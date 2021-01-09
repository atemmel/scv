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

TraitAstNode::TraitAstNode(const Token* token) : name(token->value), AstNode(token) {}

void TraitAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

CodeAstNode::CodeAstNode(const Token* token) : AstNode(token) {}

void CodeAstNode::accept(AstVisitor& visitor) {
	//TODO: This
	//visitor.visit(*this);
}

SegmentAstNode::SegmentAstNode(const Token* token) : AstNode(token) {}

void SegmentAstNode::accept(AstVisitor& visitor) {
	//TODO: This
	//visitor.visit(*this);
}

MacroAstNode::MacroAstNode(const Token* token) : AstNode(token) {}

void MacroAstNode::accept(AstVisitor& visitor) {
	//TODO: This
	//visitor.visit(*this);
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
		} else if(auto child = buildTrait(); child) {
			root->addChild(std::move(child));
		} else {
			// Let error bubble up
			error::onToken("Unrecognized token", tokens[current]);
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

AstNode::Ptr Parser::buildTrait() {
	if(!getIf(TokenType::Trait)) {
		return nullptr;
	}

	auto name = getIf(TokenType::Identifier);
	if(name == nullptr) {
		error::onToken("Expected identifier", tokens[current]);
		return nullptr;
	}

	auto trait = std::make_unique<TraitAstNode>(name);

	if(getIf(TokenType::Requires)) {
		trait->requirements = buildRequirements();
		if(!error::empty()) {
			return nullptr;
		}
	}

	if(!getIf(TokenType::LBrace)) {
		error::onToken("Expected '{'", tokens[current]);
		return nullptr;
	}

	// code block...
	auto code = buildCodeBlock();
	while(code) {
		trait->addChild(std::move(code));
		code = buildCodeBlock();
	}

	if(!getIf(TokenType::RBrace)) {
		error::onToken("Expected '}'", tokens[current]);
		return nullptr;
	}

	return trait;
}

AstNode::Ptr Parser::buildCodeBlock() {
	//TODO: This
	return nullptr;
}

std::vector<std::string> Parser::buildRequirements() {
	std::vector<std::string> requirements;
	const Token* token;
	std::string requirement;

PARSE_REQUEST:
	token = getIf(TokenType::Quote);
	if(token) {
		requirement += '\"';
		requirement += joinTokenValuesUntilToken(TokenType::Quote);
		requirement += '\"';
		if(eof()) {
			error::onToken("Quote never closed", *token);
			goto DONE;
		}
		current++;
		goto APPEND_REQUEST;
	}

	token = getIf(TokenType::Less);
	if(token) {
		requirement += '<';
		requirement += joinTokenValuesUntilToken(TokenType::Greater);
		requirement += '>';
		if(eof()) {
			error::onToken("Requirement directive never closed", *token);
			goto DONE;
		}
		current++;
		goto APPEND_REQUEST;
	}

	error::onToken("Expected requirement body, e.g \"header.hpp\" or <header.hpp>", tokens[current]);
	goto DONE;
	
APPEND_REQUEST:
	requirements.push_back(requirement);
	requirement.clear();
	if(getIf(TokenType::Comma)) {
		goto PARSE_REQUEST;
	}

DONE:
	return requirements;
}

std::string Parser::joinTokenValuesUntilToken(TokenType delim) {
	std::string join;
	for(auto token = getIfNot(delim); token; token = getIfNot(delim)) {
		join += token->value;
	}
	return join;
}

const Token* Parser::getIf(TokenType type) {
	if(current >= last || tokens[current].type != type) {
		return nullptr;
	}
	return &tokens[current++];
}

const Token* Parser::getIfNot(TokenType type) {
	if(current >= last || tokens[current].type == type) {
		return nullptr;
	}
	return &tokens[current++];
}

bool Parser::eof() const {
	return current >= last;
}
