#include "utils.hpp"

#include "error.hpp"

#include <chrono>
#include <fstream>
#include <iostream>

std::string consume(const char* path) {
	std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
	if(!file.is_open()) {
		error::set(std::string("Could not open file: ") + path + '\n');
		return std::string();
	}
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	if(size < 1) {
		return std::string();
	}
	std::vector<char> bytes(size);
	file.read(bytes.data(), size);
	return std::string(bytes.data(), size);
}

void dumpTokens(const std::vector<Token>& tokens) {
	for(auto& t : tokens) {
		size_t index = static_cast<size_t>(t.type);
		std::cout << t.row << ":" << t.column << ": type: "<< tokenStrings[index] << " value: " << t.value << '\n';
	}
}

void dieIfError() {
	if(!error::empty()) {
		std::cerr << error::get();
		std::exit(EXIT_FAILURE);
	}
}

std::string getDate() {
	auto result = std::time(nullptr);
	auto local = std::localtime(&result);
	std::string str;
	str.resize(6 + 2 + 2 + 2 + 2 + 2 + 6);
	int n = std::strftime(str.data(), str.size(), "%F %T", local);
	str.resize(n);
	return str;
}

std::string getFile(const std::string_view sv) {
	int n = sv.find_last_of('/');
	if(n == std::string::npos) {
		return std::string(sv.cbegin(), sv.cend());
	}
	return std::string(sv.cbegin() + n + 1, sv.cend());
}

std::string setStub(const std::string& path, const std::string& stub) {
	int n = path.find_last_of('.');
	if(n == std::string::npos) {
		return path + stub;
	}
	return path.substr(0, n + 1) + stub;
}

std::string joinPaths(const std::string& dir, const std::string& stub) {
	if(dir.empty()) {
		return stub;
	}

	if(dir.back() == '/') {
		return dir + stub;
	}

	return dir + '/' + stub;
}
