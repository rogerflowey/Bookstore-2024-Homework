#ifndef COMMAND_H
#define COMMAND_H
#include <regex>
#include <string>
#include <vector>


const std::regex quit_regex(R"(^\s*(quit|exit)\s*$)");

const std::regex su_regex(R"(^\s*su\s+([a-zA-Z0-9_]{1,30})(?:\s+([a-zA-Z0-9_]{1,30}))?\s*$)");
const std::regex register_regex(R"(^\s*register\s+([a-zA-Z0-9_]{1,30})\s+([a-zA-Z0-9_]{1,30})\s+([[:graph:]]{1,30})\s*$)");
const std::regex logout_regex(R"(^\s*logout\s*$)");
const std::regex passwd_regex(R"(^\s*passwd\s+([a-zA-Z0-9_]{1,30})\s+(?:([a-zA-Z0-9_]{1,30})\s+)?([a-zA-Z0-9_]{1,30})\s*$)");
const std::regex useradd_regex(R"(^\s*useradd\s+([a-zA-Z0-9_]{1,30})\s+([a-zA-Z0-9_]{1,30})\s+([0137])\s+([[:graph:]]{1,30})\s*$)");
const std::regex delete_regex(R"(^\s*delete\s+([a-zA-Z0-9_]{1,30})\s*$)");

const std::regex show_regex(R"(^\s*show\s+(-ISBN=[a-zA-Z0-9_]{1,20}|\-name="[^"\s]{1,60}"|\-author="[^"\s]{1,60}"|\-keyword="([^"\s]{1,60}?)+")?\s*$)");
const std::regex buy_regex(R"(^\s*buy\s+([a-zA-Z0-9_]{1,20})\s+(\d{1,10})\s*$)");
const std::regex select_regex(R"(^\s*select\s+([a-zA-Z0-9_]{1,20})\s*$)");
const std::regex modify_regex(R"(^\s*modify(?:\s+(?:(-ISBN=[a-zA-Z0-9_]{1,20})|(-name="[^"\s]{1,60}")|(-author="[^"\s]{1,60}")|(-keyword="(?:[^|"\s]{1,60}\|?)+")|(-price=(?:\d+(?:\.\d{1,2})?))))+\s*$)");

const std::regex ISBN_regex(R"(-ISBN=([a-zA-Z0-9_]{1,20}))");



#endif
