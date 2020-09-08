#pragma once

#include "token.hpp"

#include <string>
#include <vector>

std::string consume(const char* path);

void dumpTokens(const std::vector<Token>& tokens);

void dieIfError();

std::string getDate();

std::string getFile(const std::string_view sv);

std::string setStub(const std::string& path, const std::string& stub);

std::string joinPaths(const std::string& dir, const std::string& stub);
