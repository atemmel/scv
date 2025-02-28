#include "ast.hpp"

#include "error.hpp"
#include "pipeline.hpp"

#include <algorithm>
#include <iostream>

AstNode::AstNode(const Token* token) : origin(token) {}

void AstNode::addChild(AstNode::Ptr child) {
	children.push_back(std::move(child));
}

RootAstNode::RootAstNode() : AstNode(nullptr) {}

void RootAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

void RootAstNode::join(RootAstNode::Ptr& other) {
	structs.insert(structs.end(), other->structs.begin(), other->structs.end());
	traits.insert(traits.end(), other->traits.begin(), other->traits.end());
	addChild(std::move(other));
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
	visitor.visit(*this);
}

SegmentAstNode::SegmentAstNode(const Token* token) : AstNode(token) {}

void SegmentAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

MacroAstNode::MacroAstNode(const Token* token) : AstNode(token) {}

void MacroAstNode::accept(AstVisitor& visitor) {
	visitor.visit(*this);
}

Parser::Parser(const std::vector<Token>& tokens, const std::string& src, const std::string_view originFile) : tokens(tokens), src(src), originFile(originFile) {}

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
			TraitAstNode* trait = static_cast<TraitAstNode*>(child.get());
			root->traits.push_back(trait);
			root->addChild(std::move(child));
		} else if(buildRequire(root)) {
			continue;
		} else {
			// Let error bubble up
			if(error::empty()) {
				error::onToken("Unexpected token", eof() ? tokens.back() : tokens[current]);
			}
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

	auto code = buildCodeBlock();
	while(code) {
		trait->addChild(std::move(code));
		code = buildCodeBlock();
	}

	if(!getIf(TokenType::RBrace)) {
		error::onToken("Expected '}'", eof() ? tokens.back() : tokens[current]);
		return nullptr;
	}

	return trait;
}

AstNode::Ptr Parser::buildCodeBlock() {
	auto token = getIf(TokenType::Code);
	if(!token) {
		return nullptr;
	}

	if(!getIf(TokenType::LBrace)) {
		error::onToken("Expected '{'", tokens[current]);
		return nullptr;
	}

	auto code = std::make_unique<CodeAstNode>(token);

	size_t currentDepth = 0;

	while(1) {
		auto segment = buildSegment(currentDepth);
		if(!segment) {
			error::onToken("Code block never closed", *token);
			return nullptr;
		}

		if(getIf(TokenType::RBrace)) {
			auto asSegment = static_cast<SegmentAstNode*>(segment.get());
			auto view = asSegment->segment;
			auto firstNonSpaceOrNewline = [](const char c) {
				return !std::isspace(c) || c == '\n';
			};
			auto rtrim = std::find_if(view.rbegin(), view.rend(), firstNonSpaceOrNewline);
			asSegment->segment.remove_suffix(std::distance(view.rbegin(), rtrim));
			view = asSegment->segment;
			auto ltrim = std::find_if(view.begin(), view.end(), firstNonSpaceOrNewline);
			asSegment->segment.remove_prefix(std::distance(view.begin(), ltrim));
			code->children.push_back(std::move(segment));
			return code;
		}
		code->children.push_back(std::move(segment));

		auto macro = buildMacro();
		if(macro) {
			code->children.push_back(std::move(macro));
		}
	}

	// unreachable
	return nullptr;
}

AstNode::Ptr Parser::buildSegment(size_t &currentDepth) {
	auto segmentNode = std::make_unique<SegmentAstNode>(&tokens[current]);
	auto& firstToken = tokens[current];
	while(!eof()) {
		const Token& token = tokens[current];

		switch(token.type) {
			case TokenType::At:
				goto PARSING_DONE;
			case TokenType::LBrace:
				currentDepth++;
				break;
			case TokenType::RBrace:
				if(currentDepth == 0) {	// end of segment
					goto PARSING_DONE;
				}
				currentDepth--;
				break;
			default:
				break;
		}

		current++;
	}

	return nullptr;

PARSING_DONE:
	auto& lastToken = tokens[current];
	segmentNode->segment = std::string_view(
			src.c_str() + firstToken.index,
			lastToken.index - firstToken.index
		);
	return segmentNode;
}

AstNode::Ptr Parser::buildMacro() {
	if(!getIf(TokenType::At)) {
		return nullptr;
	}

	auto macro = std::make_unique<MacroAstNode>(&tokens[current - 1]);
	auto string = getIf(TokenType::Identifier);
	if(!string) {
		error::onToken("Expected macro identifier", tokens[current]);
		return nullptr;
	}

	macro->name = string->value;
	macro->children = std::move(buildMacroArgList());
	macro->optionalCode = std::move(buildCodeBlock());

	return macro;
}

bool Parser::buildRequire(RootAstNode::Ptr &root) {
	if(!getIf(TokenType::Requires)) {
		return false;
	}

	auto string = getIf(TokenType::Identifier);
	if(!string) {
		error::onToken("Expected file identifier", tokens[current]);
		return false;
	}

	auto dir = originFile;
	auto index = dir.find_last_of('/') + 1;
	if(index != dir.npos) {
		dir.remove_suffix(dir.size() - index);
	}

	std::string fileName = std::string(dir) + string->value + ".scv";
	if(Pipeline::hasProcessed(fileName)) {
		return true;
	}
	auto& src = Pipeline::readFile(fileName);
	auto otherRoot = Pipeline::buildRootFromSrc(src, fileName);
	otherRoot->origin = string;
	root->join(otherRoot);

	return true;
}

std::vector<AstNode::Ptr> Parser::buildMacroArgList() {
	if(!getIf(TokenType::LParens)) {
		return {};
	}

	std::vector<AstNode::Ptr> args;
	auto arg = buildMacro();
	while(arg) {
		args.push_back(std::move(arg));
		if(!getIf(TokenType::Comma)) {
			break;
		}
		arg = buildMacro();
	}

	if(!getIf(TokenType::RParens)) {
		error::onToken("Expected closing paranthesis ')'", tokens[current]);
		return {};
	}

	return args;
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
		if(!token->value.empty()) {
			join += token->value;
		} else {
			join += tokenStrings[static_cast<size_t>(token->type)];
		}
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
