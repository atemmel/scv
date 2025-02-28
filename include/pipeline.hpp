#pragma once
#include "ast.hpp"
#include "token.hpp"

#include <list>
#include <string>
#include <string_view>
#include <set>
#include <vector>

class Pipeline {
public:
	Pipeline() = delete;
	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&) = delete;
	~Pipeline() = delete;

	static void full(const std::vector<std::string_view>& sv);

	static bool hasProcessed(const std::string_view sv);

	static RootAstNode::Ptr buildRootFromSrc(const std::string &src, const std::string_view origin);
	const static std::string& readFile(const std::string_view sv);
private:
	static std::set<std::string_view> previouslyProcessed;
	static std::list<std::string> storedSrcs;
};
