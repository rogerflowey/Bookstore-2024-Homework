#ifndef UTILS_H
#define UTILS_H

#include <exception>
#include <list>
#include <set>
#include <variant>
#include <cmath>

template<int len>
struct string {
  char data[len] = {};

};

constexpr unsigned long long hash(const std::string &s) {
  unsigned long long hash = 0;
  for (char c: s) {
    hash += c;
    hash = (hash * 37);
  }
  if (hash == 0) {
    //0被用作空值代表已删除
    hash = 114514;
  }
  return hash;
}


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

class NotComparable : std::exception {
private:
  std::string message;

public:
  explicit NotComparable(const std::string &msg) : message(msg) {
  }

  NotComparable() = default;

  const char *what() const noexcept override {
    return message.c_str();
  }
};





unsigned long long strToInt(const std::string& value) {
  return std::stoull(value);
}
unsigned long long strToDec(const std::string& value) {
  double num = std::stod(value);

  int result = std::round(num * 100);

  return result;
}


void money_output(unsigned long long n) {
  std::cout<<n/100<<'.';
  if(n%100<10) {
    std::cout<<'0'<<n%100;
  } else {
    std::cout<<n%100;
  }
}



class Parser {
  std::list<std::string> tokens;

public:
  explicit Parser(std::string input,std::string seperator="") {
    std::string tmp = "";
    for (auto &c: input) {
      if (isgraph(c) && seperator.find(c)==std::string::npos) {
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
std::pair<std::string, std::string> parseParam(const std::string& input) {
  const std::vector<std::string> keys = {"-ISBN", "-name", "-author", "-keyword","-price"};

  for (const auto& key : keys) {
    if (input.find(key) == 0) {
      std::string value = input.substr(key.length());
      value.erase(0, 1);
      if (key == "-name" || key == "-author" || key == "-keyword") {
        if (value.front() == '"' && value.back() == '"') {
          value = value.substr(1, value.length() - 2);
        }
      }
      return {key.substr(1), value};
    }
  }
  return {"", ""};  // If no key is found, return an empty pair
}
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
  static T ZERO(){return T(0);}
  static T START(){return T(1);}

  static bool less(T &t1,T &t2) {
    return t1<t2;
  }
};

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

  static bool less(std::string &t1,std::string &t2) {
    return t1<t2;
  }
};


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
  static std::pair<T1, T2> ZERO(){return {IOType<T1>::ZERO(),IOType<T1>::ZERO()};}
  static std::pair<T1, T2> START(){return {IOType<T1>::ZERO(),IOType<T1>::START()};}

  static bool less(std::pair<T1, T2> &t1,std::pair<T1, T2> &t2) {
    if(t1.first==t2.first) {
      return t1.second<t2.second;
    }
    return t1.first<t2.first;
  }
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

  static bool less(Nothing &a,Nothing &b) {
    return false;
  }
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

template<typename T>
bool less(T &t1,T &t2) {
  return IOType<T>::less(t1,t2);
}


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


bool is_valid_ISBN(const std::string& isbn) {
  if (isbn.length() > 20) return false;
  for (char c : isbn) {
    if (!isgraph(c)) {
      return false;
    }
  }
  return true;
}

bool is_valid_name_or_author(const std::string& str) {
  if (str.length() > 60) return false;
  for (char c : str) {
    if (c == '"'||(!isgraph(c))) {
      return false;
    }
  }
  return true;
}

bool is_valid_keyword(const std::string& keyword) {
  if (keyword.length() > 60) return false;

  Parser kwd_p(keyword,"|");
  std::set<std::string> kwd_used;
  while(kwd_p.size()) {
    std::string kwd=kwd_p.next();
    if(kwd.empty()) {
      return false;
    }
    if(!kwd_used.insert(kwd).second) {
      return false;
    }
    for(auto &c:kwd) {
      if (c == '"'||!isgraph(c)) {
        return false;
      }
    }
  }
  return true;
}

bool is_valid_price(const std::string& value) {
  if (value.length() > 13) return false;  // Check length

  int dotCount = 0;
  bool hasDigitsAfterDot = false;

  for (char c : value) {
    if (c == '.') {
      dotCount++;
    } else if (!isdigit(c)) {
      return false;  // Invalid character
    }
  }

  if (dotCount > 1) return false;  // More than one dot is invalid
  if (dotCount == 1) {
    size_t dotPosition = value.find('.');
    if (value.length() - dotPosition - 1 > 2) {  // Ensure no more than 2 digits after the dot
      return false;
    }
  }

  return true;
}

bool is_valid_int(const std::string &value) {
  for(auto &c:value) {
    if(!(c>='0'&&c<='9')) {
      return false;
    }
  }
  return true;
}

bool is_valid_parameter(const std::pair<std::string,std::string> &param) {
  auto [key,value]=param;
  if (key == "ISBN") {
    return is_valid_ISBN(value);
  } else if (key == "name" || key == "author") {
    return is_valid_name_or_author(value);
  } else if (key == "keyword") {
    return is_valid_keyword(value);
  } else if(key=="price") {
    return is_valid_price(value);
  }
  return false;
}

#endif //UTILS_H
