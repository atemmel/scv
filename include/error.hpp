#pragma once

#include "token.hpp"

#include <string>

namespace error {

void set(const std::string& str);

void onToken(const std::string& str, const Token& tok);

const std::string& get();

bool empty();

}
