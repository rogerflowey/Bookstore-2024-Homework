#ifndef LOG_H
#define LOG_H
#include "storage.h"

extern bool TEST;
class FinanceLog {
  myVector<long long> finance_log;

public:
  FinanceLog() {
    finance_log.initialise("finance.log",TEST);
  }
  void show_finance(int n) {
    std::vector<long long> log;
    if(n==-1) {
      log=finance_log.read_all();
    } else {
      if(n==0) {
        std::cout<<std::endl;
        return;
      }
      if(n>finance_log.size()) {
        throw invalid_command();
      }
      for(int i=n-1;i>=0;--i) {
        log.push_back(finance_log.read(finance_log.size()-i));
      }
    }
    long long sum_p=0;
    long long sum_n=0;
    for(auto &l:log) {
      if(l>=0) {
        sum_p+=l;
      } else {
        sum_n-=l;
      }
    }
    std::cout<<"+ ";
    money_output(sum_p);
    std::cout<<" - ";
    money_output(sum_n);
    std::cout<<std::endl;
  };
  void push_back(long long value) {
    finance_log.write(value);
  };
};

struct string256 {
  std::string content;
};

struct Line {
  string256 line;
  std::string user;
  bool is_employee=false;
  bool is_finance=false;
};


class ActionLog {
  myVector<Line> actions;
public:
  ActionLog() {
    actions.initialise("action.log",TEST);
  }
  void report_finance(FinanceLog &log) {
    log.show_finance(-1);
    std::cout << "==================== 财务报告 ====================\n";
    auto all=actions.read_all();
    for(auto &l:all) {
      if(l.is_finance) {
        std::cout<<l.user<<" "<<l.line.content<<std::endl;
      }
    }
    std::cout << "====================================================\n";
  }
  void report_employee() {
    std::cout << "==================== 员工工作情况报告 ====================\n";
    auto all=actions.read_all();
    for(auto &l:all) {
      if(l.is_employee) {
        std::cout<<l.user<<" : "<<l.line.content<<std::endl;
      }
    }
    std::cout << "====================================================\n";
  }
  void log() {
    std::cout << "==================== 总日志 ====================\n";
    auto all=actions.read_all();
    for(auto &l:all) {
        std::cout<<l.user<<" : "<<l.line.content<<std::endl;
    }
    std::cout << "====================================================\n";
  }
  void record(Line &l) {
    actions.write(l);
  }
};

template<>
class IOType<string256> {
public:
  static int size() {
    return 256;
  }

  static void write(const string256 &v, std::fstream &out) {
    std::string value=v.content;
    int l = value.length();
    if (l > 256) {
      throw std::overflow_error("string size exceeds limit");
    }
    out.write(value.c_str(), l);
    if (l < 256) {
      out.write("\0", 256 - l);
    }
  }

  static void read(string256 &v, std::fstream &in) {
    std::string& value=v.content;
    value.resize(256);
    in.read(&value[0], 256);
    value.erase(value.find('\0'), std::string::npos);
  }
  static string256 ZERO(){throw NotComparable();};
  static string256 START(){throw NotComparable();};

  static bool less(string256 &t1,string256 &t2) {
    throw NotComparable();
  }
};

template<>
class IOType<Line> {
public:
  static int size() {
    return 322;
  }
  static void write(const Line& value, std::fstream& out) {
    IOType<string256>::write(value.line, out);
    IOType<std::string>::write(value.user, out);
    IOType<bool>::write(value.is_employee, out);
    IOType<bool>::write(value.is_finance, out);
  }

  static void read(Line& value, std::fstream& in) {
    IOType<string256>::read(value.line, in);
    IOType<std::string>::read(value.user, in);
    IOType<bool>::read(value.is_employee, in);
    IOType<bool>::read(value.is_finance, in);
  }

  static Line ZERO(){throw NotComparable("Line");}
  static Line START(){throw NotComparable("Line");}
};



#endif //LOG_H
