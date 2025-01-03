#include "storage.h"
#include "user.h"
#include "book_data.h"
#include "log.h"
#include "command.h"

bool TEST=true;

int main() {


  //freopen("../bookstore-testcases/advanced/testcase3/1.in","r",stdin);
  //freopen("my.out","w",stdout);

  int line_num=0;
  LoginStatus login_status;
  BookData book_data;
  FinanceLog finance_log;
  ActionLog action_log;
  while (std::cin) {
    ++line_num;
    std::cout.flush();
    bool success=true;
    Line temp;
    std::string line;
    try {
      getline(std::cin,line);
      Parser p(line);
      if(p.size()==0) {
        continue;
      }
      switch (hash(p.next())) {
        case hash("su"): {

          if(p.size()==2) {
            std::string user_id=p.next();
            std::string password=p.next();
            if(is_user_str(user_id)&& is_user_str(password)) {
              login_status.su(user_id,password);
              break;
            }
          }
          if(p.size()==1) {
            std::string user_id=p.next();
            if(is_user_str(user_id)) {
              login_status.su(user_id);
              break;
            }
          }
          throw invalid_command();
        }
        case hash("logout"): {
          if(login_status.get_privilege()<1) {
            throw invalid_command();
          }
          login_status.logout();
          break;
        }
        case hash("register"): {
          if(p.size()==3) {
            std::string user_id=p.next(),password=p.next(),username=p.next();
            if(is_user_str(user_id)&&is_user_str(password)&&is_user_name(username)) {
              login_status.reg(user_id,password,username);
              break;
            }
          }
          throw invalid_command();
        }
        case hash("passwd"): {
          if(login_status.get_privilege()<1) {
            throw invalid_command();
          }
          if(p.size()==3) {
            std::string user_id=p.next(),current_password=p.next(),new_password=p.next();
            if(is_user_str(user_id)&&is_user_str(current_password)&&is_user_str(new_password)) {
              login_status.passwd(user_id,new_password,current_password);
              break;
            }
          }
          if(p.size()==2) {
            std::string user_id=p.next(),new_password=p.next();
            if(is_user_str(user_id)&&is_user_str(new_password)) {
              login_status.passwd(user_id,new_password);
              break;
            }
          }
          throw invalid_command();
        }
        case hash("useradd"): {
          if(login_status.get_privilege()<3) {
            throw invalid_command();
          }
          if(p.size()==4) {
            std::string user_id=p.next();
            std::string password=p.next();
            int privilege=strToInt(p.next());
            std::string user_name=p.next();
            if(is_user_str(user_id)&&(privilege==0 || privilege==1 ||privilege==3||privilege==7)&& is_user_name(user_name)) {
              login_status.useradd(user_id,password,privilege,user_name);
              break;
            }
          }
          throw invalid_command();
        }
        case hash("delete"): {
          if(login_status.get_privilege()<7) {
            throw invalid_command();
          }
          if(p.size()==1) {
            std::string user_id=p.next();
            if(is_user_str(user_id)) {
              login_status.erase(user_id);
              break;
            }
          }
          throw invalid_command();
        }
        case hash("show"): {
          if(p.size()!=0) {
            std::string tmp=p.next();
            if(tmp=="finance") {
              if(login_status.get_privilege()<7) {
                throw invalid_command();
              }
              if(p.size()==0) {
                finance_log.show_finance(-1);
                break;
              }
              if(p.size()==1) {
                std::string n=p.next();
                if(!is_valid_int(n)) {
                  throw invalid_command();
                }
                finance_log.show_finance(strToInt(n));
                break;
              }
              throw invalid_command();
            } else {
              p.put(tmp);
            }
          }

          if(login_status.get_privilege()<1) {
            throw invalid_command();
          }
          if(p.size()==0) {
            book_data.show({"",""});
            break;
          }
          auto param=parseParam(p.next());
          if(param.second=="") {
            throw invalid_command();
          }
          if(param.first=="keyword") {
            Parser kwd_p(param.second,"|");
            if(kwd_p.size()!=1) {
              throw invalid_command();
            }
          }
          if(!is_valid_parameter(param)) {
            throw invalid_command();
          }
          book_data.show(param);
          break;
        }
        case hash("buy"): {
          temp.is_finance=true;
          if(login_status.get_privilege()<1) {
            throw invalid_command();
          }
          if(p.size()==2) {
            auto ISBN=p.next();
            auto quantity=p.next();
            if(!is_valid_ISBN(ISBN)||!is_valid_int(quantity)) {
              throw invalid_command();
            }
            book_data.buy(ISBN,strToInt(quantity),finance_log);
            break;
          }
          throw invalid_command();
        }
        case hash("select"): {
          if(login_status.get_privilege()<3) {
            throw invalid_command();
          }
          if(p.size()==1) {
            auto ISBN=p.next();
            if(!is_valid_ISBN(ISBN)) {
              throw invalid_command();
            }
            book_data.select(ISBN,login_status);
            break;
          }
          throw invalid_command();
        }
        case hash("modify"): {
          if(login_status.get_privilege()<3) {
            throw invalid_command();
          }
          BookData::Book tmp_book;
          while (p.size()) {
            auto param=parseParam(p.next());
            if(!is_valid_parameter(param)) {
              throw invalid_command();
            }
            if(param.first=="ISBN"&&tmp_book.ISBN.empty()) {
              tmp_book.ISBN=param.second;
            } else if(param.first=="name"&&tmp_book.book_name.empty()) {
              tmp_book.book_name=param.second;
            } else if(param.first=="author"&&tmp_book.author.empty()) {
              tmp_book.author=param.second;
            } else if(param.first=="keyword"&&tmp_book.keyword.empty()) {
              tmp_book.keyword=param.second;
            } else if(param.first=="price"&&tmp_book.price==0) {
              tmp_book.price=strToDec(param.second);
            } else {
              throw invalid_command();
            }
          }
          book_data.modify(tmp_book,login_status);
          break;
        }
        case hash("import"): {
          temp.is_finance=true;
          if(login_status.get_privilege()<3) {
            throw invalid_command();
          }
          if(p.size()==2) {
            auto quantity=p.next();
            auto total_cost=p.next();
            if(!is_valid_int(quantity)||!is_valid_price(total_cost)) {
              throw invalid_command();
            }
            book_data.import(strToInt(quantity),strToDec(total_cost),login_status,finance_log);
            break;
          }
          throw invalid_command();
        }

        case hash("report"):{
          if(login_status.get_privilege()<7) {
            throw invalid_command();
          }
          auto second=p.next();
          if(second=="finance") {
            action_log.report_finance(finance_log);
            break;
          } else if(second=="employee") {
            action_log.report_employee();
            break;
          }
        }
        case hash("log"): {
          if(login_status.get_privilege()<7) {
            throw invalid_command();
          }
          action_log.log();
          break;
        }

        case hash("quit"):{
        }
        case hash("exit"): {
          //login_status.user_data.find_block(hash("T35U0B7Kwv96WaJWafOkUSJOQ_T"));
          return 0;
        }



        default: {
          throw invalid_command();
        }
      }
    }
    catch (invalid_command &e) {
      std::cout<<"Invalid"<<std::endl;
      success=false;
    }
    if(success) {
      temp.user=login_status.get_user_id();
      temp.line.content=line;
      if(login_status.get_privilege()==3) {
        temp.is_employee=true;
      }
      action_log.record(temp);
    }
  }
  return 0;
}