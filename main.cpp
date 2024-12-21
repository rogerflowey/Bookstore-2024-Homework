#include "storage.h"
#include "user.h"
#include "book_data.h"
#include "log.h"


bool TEST=true;

int main() {
  LoginStatus login_status;
  BookData book_data;
  while (std::cin) {
    try {
      std::string line;
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
          if(p.size()==2) {
            auto ISBN=p.next();
            auto quantity=p.next();
            if(!is_valid_ISBN(ISBN)||!is_valid_int(quantity)) {
              throw invalid_command();
            }
            book_data.buy(ISBN,strToInt(quantity));
            break;
          }
          throw invalid_command();
        }
        case hash("select"): {
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
          if(p.size()==2) {
            auto quantity=p.next();
            auto total_cost=p.next();
            if(!is_valid_int(quantity)||!is_valid_price(total_cost)) {
              throw invalid_command();
            }
            book_data.import(strToInt(quantity),strToDec(total_cost),login_status);
            break;
          }
          throw invalid_command();
        }
        case hash("quit"):{
        }
        case hash("exit"): {
          exit(0);
        }
        default: {
          throw invalid_command();
        }
      }


    }
    catch (invalid_command &e) {
      std::cout<<"Invalid"<<std::endl;
    }
  }
}