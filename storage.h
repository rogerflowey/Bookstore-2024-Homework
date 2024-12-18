#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
#include <exception>
#include <optional>


using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template <typename T>
void write(const T& value, std::fstream& out);

template <typename T>
void write(const T& value, std::fstream& out) {
  out.write(reinterpret_cast<const char*>(&value), sizeof(T));
};

template <>
void write(const string& value, std::fstream& out) {
  int l=value.length();
  if(l>64) {
    throw std::overflow_error("string size exceed limit");
  }
  out.write(value.c_str(), l);
  if (l < 64) {
    out.write("\0", 64 - l);
  }
};

template <typename T>
void read(T& value, std::fstream& in);

template <typename T>
void read(T& value, std::fstream& in) {
  in.read(reinterpret_cast<char*>(&value), sizeof(T));
};

template<>
void read(string& value, std::fstream& in) {
  value.resize(64);
  in.read(&value[0], 64);
  value.erase(value.find('\0'), std::string::npos);
};

template <typename T>
int size() {
  return sizeof(T);
};
template<>
int size<std::string>() {
  return 64;
}


unsigned long long hash(const std::string& s) {
  unsigned long long hash = 0;
  for (char c: s) {
    hash += c;
    hash = (hash * 37);
  }
  if(hash==0) {//0被用作空值代表已删除
    hash=114514;
  }
  return hash;
}
template<class Key, class Value>
std::vector<std::pair<Key, Value>> strip(std::vector<std::pair<Key, Value>> &v) {
  std::vector<std::pair<Key, Value>> temp(v.size());
  for(auto &p:v) {
    if(p.first!=0) {
      temp.push_back(p);
    }
  }
  v=std::move(temp);
  return v;
}
template<class Key, class Value>
bool cmp(const std::pair<Key,Value>& lhs, const std::pair<Key,Value>& rhs) {
  return lhs.first < rhs.first;
}

template<class Key, class Value>
class BlockList {
private:
  fstream index_file;
  fstream file;
  string file_name;
  string index_name;
  int back=0;
  const int sizeofKey = size<Key>();
  const int sizeofValue = size<Value>();
  const int sizeofT = sizeofKey + sizeofValue;
  const int BLOCKSIZE = 4096;
  const int BLOCKCAP = 4096 / sizeofT - 1;


  //我把block包装成一个Vector
  //TODO:用有序方案？
  struct Block {
    friend BlockList;

  private:
    int begin_loc=114514;
    int size = 114514;
    int delete_cnt=114514;
    Key start;
    static BlockList *father;

    Key get_start() {
      return start;
    }

    void push_back(std::pair<Key, Value> data) {
      father->file.seekp(begin_loc + size * father->sizeofT, std::ios::beg);
      auto [tmp_key,tmp_value] = data;
      write(tmp_key,father->file);
      write(tmp_value,father->file);
      ++size;
      if (size >= father->BLOCKCAP) {
        throw std::overflow_error("Block capacity exceeded");
      }
    }

    int begin() {
      return begin_loc;
    }

    int end() {
      return begin_loc + (size) * father->sizeofT;
    }

    int next(int &t) {
      if (t != end()) {
        return t += father->sizeofT;
      }
      throw std::out_of_range("No next for end");
    }

    int prev(int &t) {
      if (t != begin()) {
        return t -= father->sizeofT;
      }
      throw std::out_of_range("No prev for begin");
    }

    std::pair<Key, Value> at(int n) {
      if (n < 0 || n >= size) {
        throw std::out_of_range(
          "Index " + std::to_string(n) + " is out of range, maximum size is " + std::to_string(size));
      }
      father->file.seekg(begin_loc + n * father->sizeofT, std::ios::beg);
      Key tmp_key;
      Value tmp_value;
      read(tmp_key,father->file);
      read(tmp_value,father->file);
      return {tmp_key, tmp_value};
    }

    void change(int n, std::pair<Key, Value> data) {
      if (n < 0 || n > size) {
        throw std::out_of_range(
          "Index " + std::to_string(n) + " is out of range, maximum size is " + std::to_string(size));
      }
      father->file.seekp(begin_loc + n * father->sizeofT, std::ios::beg);
      auto [tmp_key,tmp_value] = data;
      write(tmp_key,father->file);
      write(tmp_value,father->file);
    }

    void erase(int n) {
      if (n < 0 || n > size) {
        throw std::out_of_range(
          "Index " + std::to_string(n) + " is out of range, maximum size is " + std::to_string(size));
      }
      father->file.seekp(begin_loc + n * father->sizeofT, std::ios::beg);
      write(Key(0),father->file);
    }

    std::vector<std::pair<Key, Value> > read_block(bool strip=true) {
      std::vector<std::pair<Key, Value> > temp;
      temp.reserve(size);
      father->file.seekg(begin_loc, std::ios::beg);
      Key tmp_key;
      Value tmp_value;
      for (int i = 0; i < size; ++i) {
        read(tmp_key,father->file);
        read(tmp_value,father->file);
        if(!(!tmp_key && strip)) {
          temp.push_back({tmp_key, tmp_value});
        }
      }
      return temp;
    }



    //write_block会自动更新index
    void write_block(std::vector<std::pair<Key, Value>> data) {

      if (data.size() > father->BLOCKCAP) {
        throw std::length_error("Data size exceeds the block capacity of " + std::to_string(father->BLOCKCAP));
      }
      size = data.size();
      if(data.size()==0) {
        throw std::runtime_error("writing no data");
      }
      Key tmp=data.back().first;
      for(auto it=data.begin();it!=data.end();++it) {
        if(it->first<tmp) {
          tmp=it->first;
        }
      }
      if(tmp==0) {
        throw std::runtime_error("Deleted value not cleared");
      }
      if(start!=1) {
        start=tmp;
      }
      delete_cnt=0;
      father->index.insert({start,*this});
      father->file.seekp(begin_loc, std::ios::beg);
      for (auto &[tmp_key,tmp_value]: data) {
        write(tmp_key,father->file);
        write(tmp_value,father->file);
      }
    }

    void flush() {
      write_block(read_block());
    }


    // //注意write_block 的性能开销(unused)
    // void insert(int n, std::pair<Key, Value> data) {
    //   std::vector<std::pair<Key, Value> > block_data = read_block(false);
    //   if (n < 0 || n > block_data.size()) {
    //     throw std::out_of_range("Index out of range");
    //   }
    //   block_data.insert(block_data.begin() + n, data);
    //
    //   write_block(strip(block_data));
    // }
    //
    // void erase(int n) {
    //   std::vector<std::pair<Key, Value> > block_data = read_block(false);
    //   if (n < 0 || n >= block_data.size()) {
    //     throw std::out_of_range("Index out of range");
    //   }
    //   block_data.erase(block_data.begin() + n);
    //   write_block(strip(block_data));
    // }


    int find(Key target) {
      std::vector<std::pair<Key, Value> > block_data = read_block(false);
      auto it=block_data.begin();
      for(;it!=block_data.end();++it) {
        if(it->first==target) {
          break;
        }
      }
      if(it==block_data.end()) {
        return -1;
      }
      return std::distance(block_data.begin(),it);
    }

    void delete_block() {
      father->index.erase(start);
      size=0;
      start=0;
      delete_cnt=0;
      father->empty_block.push_back(*this);
    }
  };
  std::map<Key,Block> index;
  std::vector<Block> empty_block;


private:
  void init_index(bool clear) {
    index_name = ".index_"+file_name;
    index_file.open(index_name, std::ios::in | std::ios::out | std::ios::binary);
    size_t index_size=0;
    if (!index_file || clear) {
      index_file.clear();
      index_file.open(index_name, std::ios::out | std::ios::binary);
      index_file.close();
      index_file.open(index_name, std::ios::in | std::ios::out | std::ios::binary);

      Block b;
      b.begin_loc = 0;
      b.size = 0;
      back = BLOCKSIZE;
      b.start = 1;
      b.delete_cnt = 0;
      index.insert({b.start, b});

    }else {
      index_file.seekg(0,std::ios::beg);
      index_file.read(reinterpret_cast<char *>(&index_size), sizeof(size_t));
      if (index_file.fail()) {
        throw std::runtime_error("read file failed, please clear all previous ones");
      }
      for (int i = 0; i < index_size; ++i) {
        int begin_loc, size , delete_cnt;
        Key start_key;
        index_file.read(reinterpret_cast<char *>(&(begin_loc)), sizeof(int));
        index_file.read(reinterpret_cast<char *>(&(size)), sizeof(int));
        index_file.read(reinterpret_cast<char *>(&(delete_cnt)), sizeof(int));
        read(start_key,index_file);
        back=std::max(back,begin_loc+BLOCKSIZE);
        Block b;
        if(begin_loc>100000000) {
          throw std::runtime_error("Data corrupted");
        }
        b.begin_loc=begin_loc;
        b.size=size;
        b.start=start_key;
        b.delete_cnt=delete_cnt;
        if(b.size==0) {
          empty_block.push_back(b);
        } else {
          index.insert({b.start,b});
        }
      }
    }
  }



  Block &find_block(Key target) {
    auto it = --index.upper_bound(target);
    if (it == index.end()) {
      throw std::out_of_range("Block not found");
    }
    return it->second;
  }

  Block& new_block(std::vector<std::pair<Key,Value>> data) {
    Block temp;
    if(!empty_block.empty()) {
      temp=empty_block.back();
      empty_block.pop_back();
    } else {
      if(back>100000000) {
        throw std::runtime_error("Data corrupted");
      }
      temp.begin_loc = back;
      back+=BLOCKSIZE;
      temp.start=0;
    }
    temp.size = 0;
    temp.delete_cnt=0;
    temp.write_block(data);
    return index[temp.start];
  }



  void spilt(Block &temp_block) {
    std::vector<std::pair<Key, Value> > tmp_v=temp_block.read_block();
    std::sort(tmp_v.begin(), tmp_v.end(),cmp<Key,Value>);
    int mid = tmp_v.size() / 2;
    std::vector<std::pair<Key, Value>> first_half(tmp_v.begin(), tmp_v.begin() + mid);
    std::vector<std::pair<Key, Value>> second_half(tmp_v.begin() + mid, tmp_v.end());
    temp_block.write_block(first_half);
    Block new_block=this->new_block(second_half);
  }

  void merge(Block &temp_block) {
    auto next=index.upper_bound(temp_block.start);
    if(next==index.end()) {
      return;
    }
    Block& next_block=next->second;
    auto first_half=temp_block.read_block();
    auto second_half=next_block.read_block();
    first_half.insert(first_half.end(),second_half.begin(),second_half.end());
    temp_block.write_block(first_half);
    next_block.delete_block();
  }

public:
  bool initialise(string FN = "",bool clear=false) {
    bool return_val=false;
    Block::father = this;
    if (FN != "") file_name = FN;
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);

    if (!file || clear) {
      return_val=true;
      file.clear();
      file.open(file_name, std::ios::out | std::ios::binary);
      file.close();
      file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    }

    init_index(clear);
    return return_val;
  }
  void insert(std::pair<Key,Value> target) {
    Block& temp = find_block(target.first);
    temp.push_back(target);
    if(temp.size>=0.8*BLOCKCAP) {
      spilt(temp);
    }
  }

  bool erase(Key target) {
    Block &temp= find_block(target);
    int pos = temp.find(target);
    if (pos == -1) {
      return false;
    }
    temp.erase(pos);
    ++temp.delete_cnt;
    if(temp.delete_cnt>0.4*BLOCKCAP) {
      temp.flush();
    }
    if(temp.size-temp.delete_cnt<0.1*BLOCKCAP) {
      merge(temp);
    }
    return true;
  }
  std::optional<std::pair<Key, Value>> find(Key key) {
    std::vector<std::pair<Key,Value>> data = find_block(key).read_block();
    for(auto i=data.begin();i!=data.end();++i) {
      if(i->first==key) {
        return *i;
      }
    }
    return std::nullopt;
  }
  ~BlockList() {
    index_file.seekp(0,std::ios::beg);
    size_t size = index.size()+empty_block.size();
    index_file.write(reinterpret_cast<char *>(&(size)), sizeof(size_t));
    for (auto &b: index) {

      auto [begin_loc,size,delete_cnt,start] = b.second;
      if(begin_loc>100000000) {
        throw std::runtime_error("Data corrupted");
      }
      Key start_key=start;
      index_file.write(reinterpret_cast<char *>(&(begin_loc)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(size)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(delete_cnt)), sizeof(int));
      write(start_key,index_file);
    }
    for(auto &b:empty_block) {
      auto [begin_loc,size,delete_cnt,start] = b;
      if(begin_loc>100000000) {
        throw std::runtime_error("Data corrupted");
      }
      Key start_key=start;
      index_file.write(reinterpret_cast<char *>(&(begin_loc)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(size)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(delete_cnt)), sizeof(int));
      write(start_key,index_file);
    }
    file.close();
    index_file.close();
  }
};

template<class Key, class Value>
BlockList<Key, Value>* BlockList<Key, Value>::Block::father = nullptr;


#endif //STORAGE_H
