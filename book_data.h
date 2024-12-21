#ifndef BOOK_DATA_H
#define BOOK_DATA_H
#include <iomanip>

#include "utils.h"
#include "user.h"

extern bool TEST;

class BookData {
public:
  struct Book {
    int uid=0;
    std::string ISBN="";
    std::string author="";
    std::string book_name="";
    std::string keyword="";
    unsigned long long price=0;
    int storage=0;
    friend IOType<Book>;
  };
  friend IOType<Book>;

  myVector<Book> books;
  BlockList<unsigned long long,int> ISBN_index;
  MultiBlockList<unsigned long long,int> author_index;
  MultiBlockList<unsigned long long,int> book_name_index;
  MultiBlockList<unsigned long long,int> keyword_index;


public:

  BookData() {
    books.initialise("book.dat",TEST);
    ISBN_index.initialise("ISBN_index.dat",TEST);
    author_index.initialise("author_index.dat",TEST);
    book_name_index.initialise("book_name_index.dat",TEST);
    keyword_index.initialise("keyword_index.dat",TEST);
  }

  void select(const std::string &ISBN,LoginStatus &login_status) {
    auto tmp=ISBN_index.find(hash(ISBN));
    int uid;
    if(!tmp) {
      Book temp={books.size()+1,ISBN,"","","",0,0};
      uid=books.write(temp);
      ISBN_index.insert({hash(ISBN),uid});
    } else {
      uid=tmp->second;
    }
    login_status.select(uid);
  }

  void buy(const std::string &ISBN,int n) {
    auto tmp=ISBN_index.find(hash(ISBN));
    if(!tmp) {
      throw invalid_command();
    }
    if(n<=0) {
      throw invalid_command();
    }
    int uid=tmp->second;
    Book tmp_book=books.read(uid);
    if(tmp_book.storage<n) {
      throw invalid_command("undefined action,storage is less than buy amount");
    }
    tmp_book.storage-=n;
    money_output(tmp_book.price*n);
    books.update(tmp_book,uid);
  }

  void import(int quantity,unsigned long long total_cost, const LoginStatus &login_status) {
    int uid=login_status.get_select();
    if(quantity<=0) {
      throw invalid_command();
    }
    if(total_cost<=0) {
      throw invalid_command();
    }
    Book tmp_book=books.read(uid);
    tmp_book.storage+=quantity;
    books.update(tmp_book,uid);
  }

  void modify(const Book &target, const LoginStatus &login_status) {
    int uid=login_status.get_select();
    Book tmp_book=books.read(uid);
    if(target.ISBN!="") {
      if(target.ISBN==tmp_book.ISBN) {
        throw invalid_command();
      }
      if(!ISBN_index.erase(hash(tmp_book.ISBN))) {
        throw std::runtime_error("Data corrupted,cannot find ISBN");
      }
      tmp_book.ISBN=target.ISBN;
      ISBN_index.insert({hash(tmp_book.ISBN),uid});
    }
    if(target.book_name!="") {
      if(tmp_book.book_name!="") {
        if(!book_name_index.erase({hash(tmp_book.book_name),uid})) {
          throw std::runtime_error("Data corrupted in book name index");
        }
      }
      tmp_book.book_name=target.book_name;
      book_name_index.insert({hash(tmp_book.book_name),uid});
    }
    if(target.author!="") {
      author_index.erase({hash(tmp_book.author),uid});
      tmp_book.author=target.author;
      author_index.insert({hash(tmp_book.author),uid});
    }
    if(target.keyword!="") {
      Parser kwd_previous(tmp_book.keyword,"|");
      while (kwd_previous.size()) {
        std::string kwd=kwd_previous.next();
        keyword_index.erase({hash(kwd),uid});
      }
      Parser kwd_now(target.keyword,"|");
      while (kwd_now.size()) {
        std::string kwd=kwd_now.next();
        keyword_index.insert({hash(kwd),uid});
      }
      tmp_book.keyword=target.keyword;
    }
    if(target.price!=0) {
      tmp_book.price=target.price;
    }
    books.update(tmp_book,uid);
  }

  void show(const std::pair<std::string,std::string>& param) {
    std::vector<Book> result;
    if(param.first=="ISBN") {
      if(auto tmp=ISBN_index.find(hash(param.second))) {
        result.push_back(books.read(tmp->second));
      }
    } else if(param.first=="name") {
      auto tmp=book_name_index.find(hash(param.second));
      for(auto &p:tmp) {
        result.push_back(books.read(p.second));
      }
    } else if(param.first=="author") {
      auto tmp=author_index.find(hash(param.second));
      for(auto &p:tmp) {
        result.push_back(books.read(p.second));
      }
    } else if(param.first=="keyword") {
      auto tmp=keyword_index.find(hash(param.second));
      for(auto &p:tmp) {
        result.push_back(books.read(p.second));
      }
    } else{
      result=books.read_all();
    }
    std::sort(result.begin(), result.end(),&less<Book>);


    if(result.size()==0) {
      std::cout<<std::endl;
      return;
    }
    for(auto &b:result) {
      std::cout<<b.ISBN<<'\t'<<b.book_name<<'\t'<<b.author<<'\t'<<b.keyword<<'\t';
      money_output(b.price);
      std::cout<<'\t'<<b.storage<<std::endl;
    }
  }


};


template<>
class IOType<BookData::Book> {
public:
  static int size() {
    return 272;
  }

  static void write(const BookData::Book &value, std::fstream &out) {
    IOType<int>::write(value.uid,out);
    IOType<std::string>::write(value.ISBN,out);
    IOType<std::string>::write(value.author,out);
    IOType<std::string>::write(value.book_name,out);
    IOType<std::string>::write(value.keyword,out);
    IOType<unsigned long long>::write(value.price,out);
    IOType<int>::write(value.storage,out);
  }

  static void read(BookData::Book &value, std::fstream &in) {
    IOType<int>::read(value.uid,in);
    IOType<std::string>::read(value.ISBN,in);
    IOType<std::string>::read(value.author,in);
    IOType<std::string>::read(value.book_name,in);
    IOType<std::string>::read(value.keyword,in);
    IOType<unsigned long long>::read(value.price,in);
    IOType<int>::read(value.storage,in);
  }
  static BookData::Book ZERO(){throw NotComparable("Book");}
  static BookData::Book START(){throw NotComparable("Book");}

  static bool less(BookData::Book &a,BookData::Book &b) {
    return a.ISBN<b.ISBN;
  }
};


#endif //BOOK_DATA_H
