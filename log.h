#ifndef LOG_H
#define LOG_H
#include "storage.h"


class FinanceLog {
  myVector<long long> finance_log;

public:
  FinanceLog() {
    finance_log.initialise("finance.log");
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

#endif //LOG_H
