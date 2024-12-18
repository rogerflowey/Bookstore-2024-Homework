#ifndef UTILS_H
#define UTILS_H

#include <exception>
#include <list>

class invalid_command : std::exception {
private:
  std::string message;

public:
  explicit invalid_command(const std::string& msg) : message(msg) {}

  invalid_command()=default;

  const char* what() const noexcept override {
    return message.c_str();
  }
};

bool is_user_str(std::string &s) {
  if(s.size()>30) {
    return false;
  }
  for(const auto &c:s) {
    if(!(isalnum(c)||c=='_')) {
      return false;
    }
  }
  return true;
}
bool is_user_name(std::string &s) {
  if(s.size()>30) {
    return false;
  }
  for(const auto &c:s) {
    if(!(isgraph(c))) {
      return false;
    }
  }
  return true;
}
int strToInt(std::string token) {
  if(token.empty()) {
    throw std::runtime_error("converting empty string to int");
  }
  int out=0;
  for(auto &c:token) {
    out*=10;
    out+=c-'0';
  }
  return out;
}


class Parser {
  std::list<std::string> tokens;
public:
  explicit Parser(std::string input) {
    std::string tmp="";
    for(auto &c:input) {
      if(isgraph(c)) {
        tmp.push_back(c);
      } else {
        if(!tmp.empty()) {
          tokens.push_back(tmp);
          tmp="";
        }
      }
    }
    tokens.push_back(tmp);
  }
  std::string next() {
    std::string tmp=tokens.front();
    tokens.pop_front();
    return tmp;
  }
  int size() {
    return tokens.size();
  }
};

#endif //UTILS_H
