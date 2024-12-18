#ifndef MYERROR_H
#define MYERROR_H

#include <exception>

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

#endif //MYERROR_H
