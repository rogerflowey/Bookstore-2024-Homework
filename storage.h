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
#include <typeinfo>
#include <type_traits>
#include <utility>
#include "utils.h"


extern bool TEST;
template<typename Key, typename Value>
class MultiBlockList;


template<class Key, class Value>
std::vector<std::pair<Key, Value> > strip(std::vector<std::pair<Key, Value> > &v) {
  std::vector<std::pair<Key, Value> > temp(v.size());
  for (auto &p: v) {
    if (p.first != 0) {
      temp.push_back(p);
    }
  }
  v = std::move(temp);
  return v;
}

template<class Key, class Value>
bool cmp(const std::pair<Key, Value> &lhs, const std::pair<Key, Value> &rhs) {
  return less(lhs.first, rhs.first);
}

template<class Key, class Value>
class BlockList {
protected:
  std::fstream index_file;
  std::fstream file;
  std::string file_name;
  std::string index_name;
  int back = 0;
  const int sizeofKey;
  const int sizeofValue;
  const int sizeofT;
  const int BLOCKSIZE = 4096;
  const int BLOCKCAP;

  const Key ZERO_KEY;
  const Key START_KEY;
  //把block包装成一个Vector
  //TODO:用有序方案？
  struct Block {
    int begin_loc = 114514;
    int size = 114514;
    int delete_cnt = 114514;
    Key start;
    BlockList *father;


    Block() {
      father = nullptr;
    };

    explicit Block(BlockList *father): father(father) {
    };

    Key get_start() {
      return start;
    }

    void push_back(std::pair<Key, Value> data) {
      father->file.seekp(begin_loc + size * father->sizeofT, std::ios::beg);
      auto [tmp_key,tmp_value] = data;
      write(tmp_key, father->file);
      write(tmp_value, father->file);
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
      read(tmp_key, father->file);
      read(tmp_value, father->file);
      return {tmp_key, tmp_value};
    }

    void change(int n, std::pair<Key, Value> data) {
      if (n < 0 || n > size) {
        throw std::out_of_range(
          "Index " + std::to_string(n) + " is out of range, maximum size is " + std::to_string(size));
      }
      father->file.seekp(begin_loc + n * father->sizeofT, std::ios::beg);
      auto [tmp_key,tmp_value] = data;
      write(tmp_key, father->file);
      write(tmp_value, father->file);
    }

    void erase(int n) {
      if (n < 0 || n > size) {
        throw std::out_of_range(
          "Index " + std::to_string(n) + " is out of range, maximum size is " + std::to_string(size));
      }
      father->file.seekp(begin_loc + n * father->sizeofT, std::ios::beg);
      write(father->ZERO_KEY, father->file);
    }

    std::vector<std::pair<Key, Value> > read_block(bool strip = true) {
      std::vector<std::pair<Key, Value> > temp;
      temp.reserve(size);
      father->file.seekg(begin_loc, std::ios::beg);
      Key tmp_key;
      Value tmp_value;
      for (int i = 0; i < size; ++i) {
        read(tmp_key, father->file);
        read(tmp_value, father->file);
        if (!(tmp_key == father->ZERO_KEY && strip)) {
          temp.push_back({tmp_key, tmp_value});
        }
      }
      return temp;
    }


    //write_block会自动更新index
    void write_block(std::vector<std::pair<Key, Value> > data) {
      if (data.size() > father->BLOCKCAP) {
        throw std::length_error("Data size exceeds the block capacity of " + std::to_string(father->BLOCKCAP));
      }
      size = data.size();
      if (data.size() == 0) {
        throw std::runtime_error("writing no data");
      }
      if(start!=father->ZERO_KEY) {
        if(!father->index.erase(start)) {
          std::cerr<<"unable to erase block"<<std::endl;
        }

      }
      Key tmp = data.back().first;
      for (auto it = data.begin(); it != data.end(); ++it) {
        if (it->first < tmp) {
          tmp = it->first;
        }
      }
      if (tmp == father->ZERO_KEY) {
        throw std::runtime_error("Deleted value not cleared");
      }
      if (start != father->START_KEY) {
        start = tmp;
      }
      delete_cnt = 0;
      father->index.insert({start, *this});
      father->file.seekp(begin_loc, std::ios::beg);
      for (auto &[tmp_key,tmp_value]: data) {
        write(tmp_key, father->file);
        write(tmp_value, father->file);
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
      auto it = block_data.begin();
      for (; it != block_data.end(); ++it) {
        if (it->first == target) {
          break;
        }
      }
      if (it == block_data.end()) {
        return -1;
      }
      return std::distance(block_data.begin(), it);
    }

    void delete_block() {
      father->index.erase(start);
      size = 0;
      start = father->ZERO_KEY;
      delete_cnt = 0;
      father->empty_block.push_back(*this);
    }
  };

  std::map<Key, Block> index;
  std::vector<Block> empty_block;

protected:
  void init_index(bool clear) {
    index_name = ".index_" + file_name;
    index_file.open(index_name, std::ios::in | std::ios::out | std::ios::binary);
    size_t index_size = 0;
    if (!index_file || clear) {
      //std::cerr<<"WARNING: Cannot find file:"<<index_name<<std::endl;
      index_file.clear();
      index_file.open(index_name, std::ios::out | std::ios::binary);
      index_file.close();
      index_file.open(index_name, std::ios::in | std::ios::out | std::ios::binary);

      Block b(this);
      b.begin_loc = 0;
      b.size = 0;
      back = BLOCKSIZE;
      b.start = START_KEY;
      b.delete_cnt = 0;
      index.insert({b.start, b});
    } else {
      index_file.seekg(0, std::ios::beg);
      index_file.read(reinterpret_cast<char *>(&index_size), sizeof(size_t));

      if (index_file.fail()) {
        throw std::runtime_error("read file failed, please clear all previous ones");
      }
      for (int i = 0; i < index_size; ++i) {
        int begin_loc, size, delete_cnt;
        Key start_key;
        index_file.read(reinterpret_cast<char *>(&(begin_loc)), sizeof(int));
        index_file.read(reinterpret_cast<char *>(&(size)), sizeof(int));
        index_file.read(reinterpret_cast<char *>(&(delete_cnt)), sizeof(int));
        read(start_key, index_file);
        back = std::max(back, begin_loc + BLOCKSIZE);
        Block b(this);
        if (begin_loc > 100000000) {
          throw std::runtime_error("Data corrupted");
        }
        b.begin_loc = begin_loc;
        b.size = size;
        b.start = start_key;
        b.delete_cnt = delete_cnt;
        if (b.size == 0) {
          empty_block.push_back(b);
        } else {
          index.insert({b.start, b});
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

  Block &new_block(std::vector<std::pair<Key, Value> > data) {
    Block temp(this);
    if (!empty_block.empty()) {
      temp = empty_block.back();
      empty_block.pop_back();
    } else {
      if (back > 100000000) {
        throw std::runtime_error("Data corrupted");
      }
      temp.begin_loc = back;
      back += BLOCKSIZE;
      temp.start = ZERO_KEY;
    }
    temp.size = 0;
    temp.delete_cnt = 0;
    temp.write_block(data);
    return index[temp.start];
  }


  void spilt(Block temp_block) {
    std::vector<std::pair<Key, Value> > tmp_v = temp_block.read_block();
    std::sort(tmp_v.begin(), tmp_v.end(), cmp<Key, Value>);
    int mid = tmp_v.size() / 2;
    std::vector<std::pair<Key, Value> > first_half(tmp_v.begin(), tmp_v.begin() + mid);
    std::vector<std::pair<Key, Value> > second_half(tmp_v.begin() + mid, tmp_v.end());
    temp_block.write_block(first_half);
    this->new_block(second_half);
  }

  void merge(Block temp_block) {
    auto next = index.upper_bound(temp_block.start);
    if (next == index.end()) {
      return;
    }
    Block &next_block = next->second;
    auto first_half = temp_block.read_block();
    auto second_half = next_block.read_block();
    first_half.insert(first_half.end(), second_half.begin(), second_half.end());
    temp_block.write_block(first_half);
    next_block.delete_block();
  }

public:
  BlockList(): sizeofKey(size<Key>()), sizeofValue(size<Value>()), sizeofT(sizeofKey + sizeofValue),
               BLOCKCAP(4096 / sizeofT - 1), ZERO_KEY(ZERO<Key>()), START_KEY(START<Key>()) {
  };

  bool initialise(std::string FN = "", bool clear = false) {
    bool return_val = false;
    if (FN != "") file_name = FN;
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);

    if (!file || clear) {
      //std::cerr<<"WARNING: Cannot find file:"<<FN<<std::endl;
      return_val = true;
      file.clear();
      file.open(file_name, std::ios::out | std::ios::binary);
      file.close();
      file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    }

    init_index(clear);
    return return_val;
  }

  void insert(std::pair<Key, Value> target) {
    Block &temp = find_block(target.first);
    temp.push_back(target);
    if (temp.size >= 0.8 * BLOCKCAP) {
      spilt(temp);
    }
  }

  bool erase(Key target) {
    Block &temp = find_block(target);
    int pos = temp.find(target);
    if (pos == -1) {
      return false;
    }
    temp.erase(pos);
    ++temp.delete_cnt;
    if (temp.delete_cnt > 0.4 * BLOCKCAP) {
      temp.flush();
    }
    if (temp.size - temp.delete_cnt < 0.1 * BLOCKCAP) {
      merge(temp);
    }
    return true;
  }

  std::optional<std::pair<Key, Value> > find(Key key) {
    std::vector<std::pair<Key, Value> > data = find_block(key).read_block();
    for (auto i = data.begin(); i != data.end(); ++i) {
      if (i->first == key) {
        return *i;
      }
    }
    return std::nullopt;
  }

  ~BlockList() {
    index_file.seekp(0, std::ios::beg);
    size_t size = index.size() + empty_block.size();
    index_file.write(reinterpret_cast<char *>(&(size)), sizeof(size_t));
    for (auto &b: index) {
      auto [begin_loc,size,delete_cnt,start,useless] = b.second;
      if (begin_loc > 100000000) {
        throw std::runtime_error("Data corrupted");
      }
      Key start_key = start;
      index_file.write(reinterpret_cast<char *>(&(begin_loc)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(size)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(delete_cnt)), sizeof(int));
      write(start_key, index_file);
    }
    for (auto &b: empty_block) {
      auto [begin_loc,size,delete_cnt,start,useless] = b;
      if (begin_loc > 100000000) {
        throw std::runtime_error("Data corrupted");
      }
      Key start_key = start;
      index_file.write(reinterpret_cast<char *>(&(begin_loc)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(size)), sizeof(int));
      index_file.write(reinterpret_cast<char *>(&(delete_cnt)), sizeof(int));
      write(start_key, index_file);
    }
    file.close();
    index_file.close();
  }
};

template<typename Key, typename Value>
class MultiBlockList : public BlockList<std::pair<Key, Value>, Nothing> {
public:
  void insert(std::pair<Key, Value> target) {
    BlockList<std::pair<Key, Value>, Nothing>::insert({target, NOTHING});
  }

  bool erase(std::pair<Key, Value> target) {
    return BlockList<std::pair<Key, Value>, Nothing>::erase(target);
  }

  std::vector<std::pair<Key, Value> > find(Key key) {
    std::vector<std::pair<Key, Value> > result;
    typename BlockList<std::pair<Key, Value>, Nothing>::Block target_copy = BlockList<std::pair<Key, Value>,Nothing>::find_block({key, 0});
    typename BlockList<std::pair<Key, Value>, Nothing>::Block target = target_copy;
    while (true) {
      std::vector<std::pair<std::pair<Key, Value>, Nothing> > data = target.read_block();
      for (auto i = data.begin(); i != data.end(); ++i) {
        if (i->first.first == key) {
          result.push_back(i->first);
        }
      }
      auto it = BlockList<std::pair<Key, Value>, Nothing>::index.upper_bound(target.start);
      if (it == BlockList<std::pair<Key, Value>, Nothing>::index.end()) {
        break;
      }
      target = it->second;
      if (target.start.first > key) {
        break;
      }
    }
    std::sort(result.begin(), result.end());
    return result;
  }
};


template<class T, int info_len = 1>
class myVector {
private:
  /* your code here */
  std::fstream file;
  std::string file_name;
  int sizeofT;
  int last;

public:
  myVector() : sizeofT(::size<T>()), last(0) {
  };

  explicit myVector(const std::string &file_name) : myVector() {
    this->file_name = file_name;
  }

  ~myVector() {
    write_info(last,1);
    file.close();
  }

  void initialise(std::string FN = "", bool clear = false) {
    if (FN != "") file_name = FN;
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);

    if (!file || clear) {
      file.clear();
      file.open(file_name, std::ios::out | std::ios::binary);
      file.close();
      file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
      last=0;
    }
    else {
      get_info(last,1);
    }
  }

  int size() {
    return last;
  }

  void get_info(int &tmp, int n) {
    if (n > info_len) return;
    /* your code here */
    file.seekg((n - 1) * sizeof(int), std::ios::beg);
    file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
  }

  //将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len) return;
    /* your code here */
    file.seekp((n - 1) * sizeof(int), std::ios::beg);
    file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
  }


  //index 是1-based
  int write(T &t) {
    /* your code here */
    file.seekp(info_len * sizeof(int) + last * sizeofT, std::ios::beg);
    ::write(t, file);
    return ++last;
  }

  //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
  void update(T &t, const int index) {
    /* your code here */
    file.seekp(info_len * sizeof(int) + (index - 1) * sizeofT, std::ios::beg);
    ::write(t, file);
  }

  //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
  T read(const int index) {
    /* your code here */
    T t;
    file.seekg(info_len * sizeof(int) + (index - 1) * sizeofT, std::ios::beg);
    ::read(t, file);
    return t;
  }

  std::vector<T> read_all() {
    std::vector<T> tmp;
    for (int i = 1; i <= size(); ++i) {
      tmp.push_back(read(i));
    }
    return tmp;
  }
};


#endif //STORAGE_H
