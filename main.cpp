#include "storage.h"
#include "user.h"
#include "book_data.h"
#include "log.h"


bool TEST=true;

int main() {


  LoginStatus login_status;
  exit(0);
  while (true) {
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
        case hash("quit"):{
        }
        case hash("exit"): {
          exit(0);
        }
        default: {
          throw invalid_command();
        }
      }


    } catch (invalid_command &e) {
      std::cout<<"Invalid"<<std::endl;
    }
  }
}