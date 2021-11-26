#ifndef COMMAND_H
#define COMMAND_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <climits>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <dirent.h>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cassert>

#include <vector>
#include <string>
#include <algorithm>

#include "matcher.h"

enum error_code
{
  SUCCESS = 0,        // successful finish of function. // Can be interpreted as success flag 'true' value
  FAILURE,	      // function caused no errors, but did not manage to do its purpose.
                  // Can be interpreted as success flag 'false' value
  ERR_WRONG_INPUT,   // wrong input format
  ERR_FILE_OPEN,      // error while opening file
  ERR_FILE_DIR_EXIST  // file or directory does not exist
};

#define RESET "\033[0m" 
#define GREEN "\033[32m"

/* Enumeration for internal commands */
enum command_type
{
  CMD_OUT,  // either external command, or not command
  CMD_CD,   // changes directory
  CMD_PWD,  // shows present working directory
  CMD_TIME, // measures command work-time
  CMD_SET   // shows all shell-variables and environment variables
};

/* Splits string(text) into substrings(words) divided by given token */
void split_string_by_token(const std::string &text, char token, std::vector<std::string> &words);

/* Command obtaining class */
class command
{
  friend class pipeline;

private:
  std::string input_file_name, output_file_name;
  std::vector<std::string> command_name;
  command_type cmd_type = CMD_OUT;

public:
  /* Default class constructor */
  command() =default;

  /* Default class destructor */
  ~command()=default;

  command(const std::string &command_base, error_code &err_code){
    err_code = parse_command(command_base);

    if (err_code == SUCCESS)
      err_code = expand_command_path_params();
  }

  error_code parse_command(const std::string &command_base){
    if (command_base.empty()){
      std::cerr << "Wrong input format\n";
      return ERR_WRONG_INPUT;
    }

    std::vector<std::string> command_parts;
    split_string_by_token(command_base, ' ', command_parts);

    auto input_point_num  = std::count(command_parts.begin(), command_parts.end(), "<"),
         output_point_num = std::count(command_parts.begin(), command_parts.end(), ">");

    if (input_point_num > 1 || output_point_num > 1){
      std::cerr << "Wrong input format\n";
      return ERR_WRONG_INPUT;
    }

    auto  input_point  = std::find(command_parts.begin(), command_parts.end(), "<"),
          output_point = std::find(command_parts.begin(), command_parts.end(), ">");

    if (output_point < input_point){
      command_name.assign(command_parts.begin(), output_point);
      output_file_name = *(output_point + 1);

      if (input_point != command_parts.end())
        input_file_name = *(input_point +1);
    }
    else if (input_point < output_point){
      command_name.assign(command_parts.begin(), input_point);
      input_file_name = *(input_point + 1);
      
      if (output_point != command_parts.end())
        output_file_name = *(output_point + 1);
    }
    else 
      command_name = command_parts;
    
    if (!command_parts.empty())
      cmd_type = get_command_type(command_parts[0]);

    return SUCCESS;
  }

  static command_type get_command_type(const std::string &cmd_name){
    if (cmd_name.empty())
      return CMD_OUT;
    else if (cmd_name == "cd")
      return CMD_CD;
    else if (cmd_name == "pwd")
      return CMD_PWD;
    else if (cmd_name == "time"){
      return CMD_TIME;
    }
    else if (cmd_name == "set")
      return CMD_SET;
    else
      return CMD_OUT;
  }

  //Regular expression expansion functions
  //It's a TRYAP.
  static bool is_expansion_needed(const std::string &text){
    if (text.find('*') != std::string::npos || text.find('?') != std::string::npos)
      return true;
    
    return false;
  }

  error_code expand_command_path_params(void){
    for (size_t i = 0; i < command_name.size(); i++){
      if (!is_expansion_needed(command_name[i]))
        continue;
      
      std::vector<std::string> expanded_path_names = expand_path_regex(command_name[i]);

      if (expanded_path_names.empty())
        continue;
        
      command_name.erase(command_name.begin() + i);
      command_name.insert(command_name.begin() + i, expanded_path_names.begin(), expanded_path_names.end());
    }
    
    return SUCCESS;
  }

  static std::vector<std::string> expand_path_regex(const std::string &path_regex){
    std::vector<std::string> splitted_path_regex;
    split_string_by_token(path_regex, '/', splitted_path_regex);

    std::vector<std::string> expanded_path_name;
    std::string path;
    bool flag = false;

    if (splitted_path_regex[0].empty()){
      path = "/";
      flag = true;
    }
    else 
      path = get_curr_dir() + "/";

    expanded_path_name.emplace_back("");

    bool is_dir = false;
    if (splitted_path_regex[splitted_path_regex.size() - 1].empty())
      is_dir = true;

    std::vector<std::string> reverse_names;

    for(auto it = splitted_path_regex.rbegin(); it != splitted_path_regex.rend(); it++){
      if (it->empty())
        continue;
      reverse_names.push_back(*it);
    }

    int args_ptr = reverse_names.size() - 1;

    while(args_ptr >= 0){
      std::vector<std::string> tmp_path_name;
      
      for (std::string& i : expanded_path_name){
        if (flag)
          i += '/';

        DIR* dir = opendir((path + i).c_str());
        if (dir != nullptr){
          for (dirent* d = readdir(dir); d != nullptr; d = readdir(dir)){
            if (d->d_name[0] == '.')
              continue;
            if (args_ptr > 0){
              matcher m(d->d_name, reverse_names[args_ptr].c_str());
              
              if (d->d_type == DT_DIR && m.match())
                tmp_path_name.push_back(i + d->d_name);
            }
            else {
              matcher m(d->d_name, reverse_names[args_ptr].c_str());
              if ((d->d_type == DT_DIR || (d->d_type == DT_REG && !is_dir)) && m.match())
                tmp_path_name.push_back(i + d->d_name);
            }
          }
        }
        closedir(dir);
      }
      expanded_path_name = tmp_path_name;
      args_ptr--;
      flag = true;
    }
    
    if (expanded_path_name.empty())
      return {};

    return expanded_path_name;
  }

  //command execution and related functions
  static std::string get_curr_dir(void){
    char dir_name_C[PATH_MAX];

    if (getcwd(dir_name_C, PATH_MAX) == nullptr)
      std::cerr << strerror(errno);

    return std::string(dir_name_C);
  }

  static std::string get_home_dir(void){
    const char *home_dir_name_C;

    if ((home_dir_name_C = getenv("HOME")) == nullptr)
      home_dir_name_C = getpwuid(getuid())->pw_dir;
    
    return std::string(home_dir_name_C);
  }

  //print current dir in given output stream
  static error_code print_intro_line(std::ostream &os){
    gid_t gid = getgid();
    errno = 0;
    std::string intro_line = get_curr_dir();
    
    if (errno != 0)
      return FAILURE;

    if (gid == 0)
      intro_line += "!";
    else
      intro_line += "> ";
    
    os << "\033[32m" << intro_line << "\033[0m";

    return SUCCESS;
  }

  error_code exec(void){
    switch(cmd_type){
      case CMD_CD:
        if (command_name.size() == 1)
          return exec_cd(get_home_dir());
        else if (command_name.size() == 2)
          return exec_cd(command_name[1]);
        
        return FAILURE;
      case CMD_PWD:
        return exec_pwd();
      case CMD_SET:
        return exec_set();
      case CMD_OUT:
        if (io_redirect() != SUCCESS){
          //std::cerr << "command.h:283\n";
          return FAILURE;
        }
          
        exec_bash_command(command_name);
        break;
      case CMD_TIME:
        std::cerr << "Wrong input format\n";
        break;
    }
    
    return SUCCESS;
  }

  error_code io_redirect(void){
    int fd_in = -1, fd_out = -1;
    
    //std::cerr << "I am in io_redirect\n\n";

    if (!output_file_name.empty()){
      fd_out = open(output_file_name.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IWRITE | S_IREAD);

      if (fd_out == -1){
        std::cerr << "Could not open file\n\n";
        return ERR_FILE_OPEN;
      }
      dup2(fd_out, STDOUT_FILENO);
    }

    if (!input_file_name.empty()){
      fd_in = open(input_file_name.c_str(), O_RDONLY);
      //std::cerr << "308:Could not open file\n\n";

      if (fd_in == -1){
        std::cerr << "Could not open file\n\n";
        return ERR_FILE_OPEN;
      }
      dup2(fd_in, STDIN_FILENO);
    }

    return SUCCESS;
  }

  static error_code exec_cd(const std::string& new_dir){

    if (chdir(new_dir.c_str()) == -1){
      std::cerr << "File or directory does not exist\n\n";
      return ERR_FILE_DIR_EXIST;
    }

    return SUCCESS;
  }

  static error_code exec_pwd(void){
    errno = 0;
    std::string cur_dir;
    cur_dir = get_curr_dir();
    if (errno != 0){
      kill(getpid(), SIGINT);
      return FAILURE;
    }

    std::cout << cur_dir << std::endl;

    return SUCCESS;
  }

  static error_code exec_set(void){
    for (int i = 0; environ[i] != nullptr; i++)
      std::cout << environ[i] << std::endl;

    return SUCCESS;
  }

  static void exec_bash_command(const std::vector<std::string> &command_name){
    errno = 0;
    std::vector<char*> v;
    
    v.reserve(command_name.size());
    for (const auto &s : command_name)
      v.push_back((char *)s.c_str());
    
    v.push_back(nullptr);
    execvp(v[0], &v[0]);
    perror(v[0]);
    kill(getpid(), SIGKILL);
  }  

};

#endif