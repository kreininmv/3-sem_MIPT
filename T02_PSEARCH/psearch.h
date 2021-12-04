#ifndef PSEARCH_H
#define PSEARCH_H

#include <iostream>
#include <vector>
#include <string>

#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cstring>
#include <fstream>
#include <thread>
#include <mutex>

class psearch{
private:  

  //parser
  std::string path = ".", pattern;
  size_t flag_current_dir = 0;
  std::vector<std::string> file_paths;
  std::vector<std::string> file_names;

  bool parsing(int argc, char** argv);
  int make_queue(const std::string& current_path);
  int for_cur_dir_make_queue(void);

  //KPM avtomat
  std::vector<size_t> prefix;
  std::vector<std::vector<size_t>> kmp_state;
  void make_kmp(void);
  void prefix_function(void);
  bool row_check(const std::string& row);

  //Thread search
  size_t num_of_threads = 1;
  std::mutex mut;
  std::vector<std::thread> threads;
  size_t queue_position = 0;
  void search(void);

  void print(void);
  bool is_printed = false;

  
public:
  psearch(void) = default;
  ~psearch() = default;

  int find(int argc, char** argv);
};

#endif