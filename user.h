#ifndef USER_H
#define USER_H
#include <vector>
#include <string>
#include "storage.h"
#include "myError.h"



class LoginStatus {
  struct User {
    int privilege;
    std::string username;
    std::string user_id;
    unsigned long long password;
    int selected=0;

  };
  template <typename T>
  friend int size();
  template <typename T>
  friend void read(T& value, std::fstream& in);
  template <typename T>
  friend void write(const T& value, std::fstream& out);

  std::vector<User> login_stack;
  BlockList<unsigned long long,User> user_data;


public:
  LoginStatus() {
    if(user_data.initialise("user_data.dat")) {
      User root={7,"root","root",hash("sjtu")};
      user_data.insert({hash("root"),root});
    }
  }

  ~LoginStatus()=default;

  void su(const std::string &user_id,const std::string& password="") {
    auto tmp=user_data.find(hash(user_id));
    if(tmp) {
      throw invalid_command();
    }
    User tmp_user = tmp->second;
    if(password==""){
      if(get_privilege()<=tmp_user.privilege) {
        throw invalid_command();
      }
    }
    else {
      if(tmp_user.password!=hash(password)) {
        throw invalid_command();
      }
    }
    login_stack.push_back(tmp_user);
  }

  void logout() {
    if(get_privilege()==0) {
      throw invalid_command();
    }
    if(login_stack.empty()) {
      throw invalid_command();
    }
    login_stack.pop_back();
  }

  void reg(const std::string& user_id,const std::string& password,const std::string& username) {
    if(user_data.find(hash(user_id))) {
      throw invalid_command();
    }
    User new_user={1,username,user_id,hash(password)};
    user_data.insert({hash(user_id),new_user});
  }

  void passwd(const std::string& user_id,const std::string& new_passwd,const std::string& passwd="") {
    if(get_privilege()==0) {
      throw invalid_command();
    }
    auto tmp=user_data.find(hash(user_id));
    if(!tmp) {
      throw invalid_command();
    }
    if(passwd=="") {
      if(get_privilege()!=7) {
        throw invalid_command();
      }
    }
    User tmp_user=tmp->second;
    if(hash(passwd)!=tmp_user.password) {
      throw invalid_command();
    }
    user_data.erase(hash(tmp_user.user_id));
    tmp_user.password=hash(new_passwd);
    user_data.insert({hash(tmp_user.user_id),tmp_user});

  }

  void useradd(const std::string& user_id,const std::string& password, const int privilege, const std::string &username) {
    if(privilege>=get_privilege()) {
      throw invalid_command();
    }
    if(user_data.find(hash(user_id))) {
      throw invalid_command();
    }
    User new_user={privilege,username,user_id,hash(password),0};
    user_data.insert({hash(user_id),new_user});
  }

  void erase(const std::string& user_id) {
    if(!user_data.find(hash(user_id))) {
      throw invalid_command();
    }
    for(auto &user:login_stack) {
      if(hash(user.user_id)==hash(user_id)) {
        throw invalid_command();
      }
    }
    user_data.erase(hash(user_id));
  }

  int get_privilege() const {
    if(login_stack.empty()) {
      return 0;
    }
    return login_stack.back().privilege;
  }

};

template <>
void write(const LoginStatus::User& value, std::fstream& out) {
  write(value.privilege, out);
  write(value.username, out);
  write(value.user_id, out);
  write(value.password, out);
}

template <>
void read(LoginStatus::User& value, std::fstream& in) {
  read(value.privilege, in);
  read(value.username, in);
  read(value.user_id, in);
  read(value.password, in);
}
template <>
int size<LoginStatus::User>() {
  return 140;
}
#endif //USER_H