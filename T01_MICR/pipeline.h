#ifndef PIPELINE_H
#define PIPELINE_H

#include <iostream>
#include <fcntl.h>
#include <sys/times.h>
#include <sys/wait.h>

#include <string>
#include <vector>
#include <deque>
#include <iomanip>

#include "command.h"

#define READ_END 0
#define WRITE_END 1

class pipeline{
private:
    std::deque<command> command_queue;

public:
  pipeline(void) = default;
  ~pipeline(void){
      clear_pipeline();
  }
  
  error_code reset_pipeline(const std::string &command_line){
    clear_pipeline();
    std::vector<std::string> splitted_cmd_line;
    split_string_by_token(command_line, '|', splitted_cmd_line);
    
    //std::cerr << "pipeline.h: 34\n\n";
    if (check_cmd_line_IO_pattern(command_line, splitted_cmd_line) != SUCCESS){
      std::cerr << "Wrong input format\n";
      //std::cerr << "pipeline.h: 37\n\n";
      return ERR_WRONG_INPUT;
    }
    
    //std::cerr << "pipeline.h: 41\n\n";
    for (auto &curr_cmd_base : splitted_cmd_line){
      error_code err_code = SUCCESS;
      command_queue.emplace_back(curr_cmd_base, err_code);
    
      if (err_code != SUCCESS){
        clear_pipeline();
        return err_code;
      }
    }
    
    //std::cerr << "pipeline.h: 50\n\n";
    return SUCCESS;
  }

  error_code check_cmd_line_IO_pattern(const std::string &command_line, const std::vector<std::string> &splitted_command_line) {
    //std::cerr << "pipeline.h: 57\n\n";
    if (command_line.empty())
      return SUCCESS;
      
    //std::cerr << "pipeline.h: 61\n\n";
    auto
      input_points_num  = std::count(command_line.begin(), command_line.end(), '<'),
      output_points_num = std::count(command_line.begin(), command_line.end(), '>'),
      input_points_num_in_front = std::count(splitted_command_line.front().begin(), splitted_command_line.front().end(), '<'),
      output_points_num_in_back = std::count(splitted_command_line.back().begin() , splitted_command_line.back().end() , '>');
    
    //std::cerr << "pipeline.h: 68\n\n";
    if (!(input_points_num  == input_points_num_in_front && input_points_num <= 1 &&
          output_points_num == output_points_num_in_back && output_points_num <= 1))
      return FAILURE;
    
    //std::cerr << "pipeline.h: 73\n\n";
    return SUCCESS;
  }

  void clear_pipeline(void){
    command_queue.clear();
  }

  error_code exec(void){
    if (command_queue.empty())
      return SUCCESS;
    
    if (command_queue.front().cmd_type == CMD_TIME)
      return exec_with_time();
    
    auto &front_cmd = command_queue.front();
    
    if (front_cmd.cmd_type == CMD_CD)
      return front_cmd.exec();

    //creating pipes for pipeline
    std::vector<int[2]> pipe_array(command_queue.size() - 1);
    for (auto s : pipe_array){
      if (pipe2(s, O_CLOEXEC) != 0){
        std::cerr << "Can't open pipe\n";
        return FAILURE;
      }
    }
    
    //connect created pipes so as they constitute pipeline, create childprocess and execute command in this child process
    for (size_t i = 0; i < command_queue.size(); i++){
      pid_t pid = fork();

      if (pid == 0){
        // no pipeline is required, that is why no pipes were create. just exec one cmd
        if (command_queue.size() == 1 ){
          if (command_queue[i].exec() != SUCCESS)
            return FAILURE;
        }
        else if (i == 0){  
          dup2(pipe_array[i][WRITE_END], STDOUT_FILENO);
          close(pipe_array[i][READ_END]);
          if (command_queue[i].exec() != SUCCESS)
            return FAILURE;
        }
        else if (i > 0 && i < (command_queue.size() - 1)){
          dup2(pipe_array[i-1][READ_END], STDIN_FILENO);
          dup2(pipe_array[i][WRITE_END], STDOUT_FILENO);
          
          close(pipe_array[i-1][WRITE_END]);
          close(pipe_array[i][READ_END]);
          if (command_queue[i].exec() != SUCCESS)
            return FAILURE;
        }
        else {
          dup2(pipe_array[i-1][READ_END], STDIN_FILENO);
          close(pipe_array[i-1][WRITE_END]);
          if (command_queue[i].exec() != SUCCESS)
            return FAILURE;
        }
      }
      //pid != 0
      else {
        if (i == (command_queue.size() - 1)){
          for (auto &pipe : pipe_array){
            close(pipe[READ_END]);
            close(pipe[WRITE_END]);
          }

          for (size_t j = 0; j < command_queue.size(); j++)
            wait(nullptr);
        }
      }
    }
    
    return SUCCESS;
  }

  error_code exec_with_time(void){
    if (command_queue.empty() || command_queue.front().cmd_type != CMD_TIME)
      return ERR_WRONG_INPUT;
    
    auto &front_command = command_queue.front();
    front_command.command_name.erase(front_command.command_name.begin());
    front_command.cmd_type = (command_queue.empty()) ? CMD_OUT : command::get_command_type(front_command.command_name[0]);

    auto cps = sysconf(_SC_CLK_TCK);
    tms start{}, stop{};
    clock_t start_real_time = times(&start);
    exec();
    clock_t stop_real_time = times(&stop);

    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(3)
      << "real : " << get_time_in_sec(stop_real_time - start_real_time, cps) << "s" << std::endl
      << "user : " << get_time_in_sec((stop.tms_utime + stop.tms_cutime) - (start.tms_utime + start.tms_cutime), cps) << "s" << std::endl
      << "sys  : " << get_time_in_sec((stop.tms_stime + stop.tms_cstime) - (start.tms_stime + start.tms_cstime), cps) << "s" << std::endl;
  
    std::cout.unsetf(std::ios::fixed);

    return SUCCESS;
  }

  /* Returns time between 'start' and 'stop', which are given in clock_t, in seconds */
  static long double get_time_in_sec(clock_t time, long clocks_per_second)
  {
    return (long double)time / clocks_per_second;
  }
};
#endif