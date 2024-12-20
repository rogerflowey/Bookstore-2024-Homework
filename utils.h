#ifndef UTILS_H
#define UTILS_H

#include <exception>
#include <list>

class invalid_command : std::exception {
private:
  std::string message;

public:
  explicit invalid_command(const std::string &msg) : message(msg) {
  }

  invalid_command() = default;

  const char *what() const noexcept override {
    return message.c_str();
  }
};

bool is_user_str(std::string &s) {
  if (s.size() > 30) {
    return false;
  }
  for (const auto &c: s) {
    if (!(isalnum(c) || c == '_')) {
      return false;
    }
  }
  return true;
}

bool is_user_name(std::string &s) {
  if (s.size() > 30) {
    return false;
  }
  for (const auto &c: s) {
    if (!(isgraph(c))) {
      return false;
    }
  }
  return true;
}

int strToInt(std::string token) {
  if (token.empty()) {
    throw std::runtime_error("converting empty string to int");
  }
  int out = 0;
  for (auto &c: token) {
    out *= 10;
    out += c - '0';
  }
  return out;
}


class Parser {
  std::list<std::string> tokens;

public:
  explicit Parser(std::string input) {
    std::string tmp = "";
    for (auto &c: input) {
      if (isgraph(c)) {
        tmp.push_back(c);
      } else {
        if (!tmp.empty()) {
          tokens.push_back(tmp);
          tmp = "";
        }
      }
    }
    tokens.push_back(tmp);
  }

  std::string next() {
    std::string tmp = tokens.front();
    tokens.pop_front();
    return tmp;
  }

  int size() {
    return tokens.size();
  }
};

struct Nothing {
};

constexpr Nothing NOTHING;

template<typename T>
class IOType {
public:
  // 默认情况下，返回类型 T 的大小
  static int size() {
    if (std::is_class<T>::value) {
      std::cerr << "WARNING: USING DEFAULT SIZE FOR " << typeid(T).name() << std::endl;
    }
    return sizeof(T);
  }

  // 默认情况下，写入类型 T 到文件
  static void write(const T &value, std::fstream &out) {
    out.write(reinterpret_cast<const char *>(&value), size());
  }

  // 默认情况下，从文件读取类型 T
  static void read(T &value, std::fstream &in) {
    in.read(reinterpret_cast<char *>(&value), size());
  }
  static T ZERO(){return T(0);};
  static T START(){return T(1);};
};

// 特化 std::string 类型
template<>
class IOType<std::string> {
public:
  static int size() {
    return 64; // 根据实际需求，可能返回 std::string 对象的大小
  }

  static void write(const std::string &value, std::fstream &out) {
    int l = value.length();
    if (l > 64) {
      throw std::overflow_error("string size exceeds limit");
    }
    out.write(value.c_str(), l);
    if (l < 64) {
      out.write("\0", 64 - l); // 填充剩余的空白
    }
  }

  static void read(std::string &value, std::fstream &in) {
    value.resize(64);
    in.read(&value[0], 64);
    value.erase(value.find('\0'), std::string::npos); // 删除填充的空字符
  }
  static std::string ZERO(){return "";};
  static std::string START(){return "\0";};
};

// 特化 std::pair 类型
template<typename T1, typename T2>
class IOType<std::pair<T1, T2> > {
public:
  static int size() {
    return IOType<T1>::size() + IOType<T2>::size();
  }

  static void write(const std::pair<T1, T2> &value, std::fstream &out) {
    IOType<T1>::write(value.first, out);
    IOType<T2>::write(value.second, out);
  }

  static void read(std::pair<T1, T2> &value, std::fstream &in) {
    IOType<T1>::read(value.first, in);
    IOType<T2>::read(value.second, in);
  }
  static std::pair<T1, T2> ZERO(){return {IOType<T1>::ZERO(),IOType<T1>::ZERO()};};
  static std::pair<T1, T2> START(){return {IOType<T1>::ZERO(),IOType<T1>::START()};};
};

template<>
class IOType<Nothing> {
public:
  static int size() {
    return 0;
  }

  static void write(const Nothing &value, std::fstream &out) {
    return;
  }

  static void read(Nothing &value, std::fstream &in) {
    return;
  }
  static Nothing ZERO(){return NOTHING;};
  static Nothing START(){return NOTHING;};
};

// 使用模板函数调用 TypeSize 类的静态函数
template<typename T>
int size() {
  return IOType<T>::size();
}

template<typename T>
void write(const T &value, std::fstream &out) {
  IOType<T>::write(value, out);
}

template<typename T>
void read(T &value, std::fstream &in) {
  IOType<T>::read(value, in);
}

template<typename T>
constexpr T ZERO() {
  return IOType<T>::ZERO();
}
template<typename T>
constexpr T START() {
  return IOType<T>::START();
}

#endif //UTILS_H
