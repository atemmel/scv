#include "argparser.hpp"

#include <iostream>
#include <algorithm>

ArgParser::ArgParser(int argc, char** argv) : args(argv + 1, argv + argc) {
}

void ArgParser::addBool(bool* var, std::string_view flag) {
	flags.insert({flag, {static_cast<void*>(var), VarPtr::Type::Bool} });
}

void ArgParser::addString(std::string* var, std::string_view flag) {
	flags.insert({flag, {static_cast<void*>(var), VarPtr::Type::String} });
}

const ArgParser::Args& ArgParser::unwind() {
	for(auto it = args.begin(); it != args.end(); it++) {
		auto hashIt = flags.find(*it);

		if(hashIt == flags.end() ) {
			unused.push_back(*it);
			continue;
		}
		auto& var = hashIt->second;

		int availableArgs = std::distance(std::next(it), args.end() );
		if(availableArgs < 1 && var.type != VarPtr::Type::Bool) {
			std::cerr << "Too few arguments for argument " << hashIt->first 
				<< ", expected " << 1 << ", recieved " << availableArgs 
				<< '\n';
			std::exit(EXIT_FAILURE);
		}

		switch(var.type) {
			case VarPtr::Type::Bool:
				*static_cast<bool*>(var.ptr) = true;
				break;
			case VarPtr::Type::String:
				static_cast<std::string*>(var.ptr)->assign(*std::next(it) );
				break;
		}

		if(var.type != VarPtr::Type::Bool) {
			std::advance(it, 1);
		}
	}
	return unused;
}
