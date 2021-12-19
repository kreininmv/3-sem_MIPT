#include "psearch.h"

int psearch::make_queue(const std::string& current_path){
  
  //std::cout << "Current path: " << current_path << std::endl;
  
  DIR *dir = opendir(current_path.c_str());
  if (dir == nullptr)
    return 0;

  std::string new_path;
  
  for (auto rd = readdir(dir); rd != nullptr; rd = readdir(dir)){
    switch(rd->d_type){
      case DT_REG:
        file_names.emplace_back(std::string(rd->d_name));

        if (path[path.size() - 1] == '/')
          new_path = current_path + rd->d_name;
        else  
          new_path = current_path + "/" + rd->d_name;
        
        mut.lock();
        file_paths.push_back(new_path);
        mut.unlock();
        break;

      case DT_DIR:
        if ((strcmp(rd->d_name, ".") != 0) && (strcmp(rd->d_name, "..") != 0)){
          if (current_path[current_path.size() - 1] == '/')
            new_path = current_path + rd->d_name;
          else
            new_path = current_path + "/" + rd->d_name;

          make_queue(new_path);
        }
        break;
      
      default:
        break;
    }
  }
  
  closedir(dir);
  return 0;
}

int psearch::for_cur_dir_make_queue(void) {
  
  DIR *dir = opendir(path.c_str());
  
  if (dir == nullptr)
    return 0;
  
  for (auto rd = readdir(dir); rd != nullptr; rd = readdir(dir)) {
    if (rd->d_type == DT_REG) {
      file_names.emplace_back(std::string(rd->d_name));
      std::string new_path;
      if (path[path.size() - 1] == '/')
        new_path = path + rd->d_name;
      else 
        new_path = path + "/" + rd->d_name;

      mut.lock();
      file_paths.push_back(new_path);
      mut.unlock();
    }
  }


  closedir(dir);
  return 0;
}

void psearch::print(void){
  std::cout << "Path:" << path << std::endl;
  std::cout << "Pattern:" << pattern << std::endl;
  std::cout << "Num of threads:" << num_of_threads << std::endl;
  std::cout << "Current directory:" << flag_current_dir << std::endl << std::endl;
  std::cout << "File names size: " << file_names.size() << std::endl;
  std::cout << "File paths size: " << file_paths.size() << std::endl;
  std::cout << "Queue:" << std::endl;

  for (int i = 0; i < file_paths.size(); i++)
     std::cout << i << " " << file_paths[i] << "--> " << file_names[i] << std::endl;
}

void psearch::search(void){
  bool exit = false;

  while(true){
    
    //std::cerr << "I came to search\n";    
    
    mut.lock();
    std::string current_path;
    size_t current_queue_position;
    
    if (queue_position < file_paths.size()){
      //std::cerr << "I find file to find text in it\n";
      current_path = file_paths[queue_position];
      current_queue_position = queue_position;
      queue_position++;
    }
    else if (finish)
      exit = true;

    mut.unlock();
    std::ifstream input(current_path);
    
    if (input){
      std::string line;
      size_t i = 0;
      while(getline(input, line)){
    
        if (row_check(line)){
          std::cout << file_names[current_queue_position] << " " << i << " " << line << std::endl;
          is_printed = true;
        }
        i++;
      }
      input.close();
    }
    else
      usleep(30+30+30);
    
    if (exit)
      return;
  }
}


bool psearch::row_check(const std::string& row){
  size_t cur_state = 0;

  //std::cerr << "Compare: " << row << "\n";

  for (size_t i = 0; i < row.size(); i++){
    if ((row[i] < 128) && (row[i] > -1))
      cur_state = kmp_state[row[i]][cur_state];
    else
      cur_state = 0;
    
    if (cur_state == pattern.size())
      return true;
  }

  return false;
}

// Psearch class
int psearch::find(int argc, char** argv){
  
  if (!parsing(argc, argv))
    return 0;

  make_kmp();

  for (int i = 0; i < num_of_threads; i++)
    threads.emplace_back(std::thread(&psearch::search, this));

  if (flag_current_dir){
    //std::cout << "FIRST\n";
    for_cur_dir_make_queue();
  }
  else{
    //std::cout << "SECOND\n";
    make_queue(path);
  }
    
  mut.lock();
  finish = true;
  mut.unlock();
  //print();

  for (int i = 0; i < num_of_threads; i++)
    threads[i].join();


  if (!is_printed)
    std::cerr << "I can't find: " << pattern << std::endl;

  return 0;
}

void psearch::prefix_function(void){
  size_t num = pattern.size();
  prefix.resize(num);

  for (size_t i = 1; i < num; i++){
    size_t len_prev_pref = prefix[i-1];
    while((len_prev_pref > 0) && (pattern[i] != pattern[len_prev_pref]))
      len_prev_pref = prefix[len_prev_pref -1];

    if (pattern[i] == pattern[len_prev_pref])
      len_prev_pref++;

    prefix[i] = len_prev_pref;
  }
}

// KMP class
void psearch::make_kmp(void){
  prefix_function();

  kmp_state.resize(128, std::vector<size_t>(pattern.size() +1));

  size_t i = 0, j = 0;
  kmp_state[pattern[0]][0] = 1;

  for (i = 0; i < 128; i++){
    for (j = 0; j < pattern.size(); j++){
      if (i == pattern[j])
        kmp_state[i][j] = j + 1;
      else
        kmp_state[i][j] = kmp_state[i][prefix[j]];
    }
    kmp_state[i][j] = pattern.size();
  }
}

/// Parsing class
 bool psearch::parsing(int argc, char **argv){

   if (argc > 5){
     std::cerr << "Lol, you can't count. Too many arguments\n";
     return false;
   }
   if (argc < 2){
     std::cerr << "Lol, you can't count. Too few arguments\n";
     return false;
   }

   bool path_flag = true;
   int num_of_flags = 0;

   for (int i = 0; i < argc; i++){
     switch(argv[i][0]){
       case '-':
         if (num_of_flags < 2 && argv[i][1] == 't') {
              std::string thread_flag = argv[i] + 2;
              num_of_threads = stoi(thread_flag);
              num_of_flags++;
        }
        else if (num_of_flags < 2 && argv[i][1] == 'n'){
             flag_current_dir = 1;
             num_of_flags++;
        }
        else if (num_of_flags < 2){
             std::cerr << "Lol, wrong request, invalid parameter!\n"; 
             return false;
        }
        else {
          std::cerr << "Lol, you can't count. Too many parametres\n";
           return false;
         }
         break;

        case '/':
          if (path_flag){
            path = argv[i];
            path_flag = false;
          }
          else{
            std::cerr << "LOL, wrong request! Only single path!\n";
            return false;
          }
          break;
        
        case '.':
          if (i != 0 && !path_flag){
            std::cerr << "LOL, wrong request. Only single path\n";
            return false;
          }
          if (i != 0){
            path = argv[i];
            path_flag = false;
          }
          break;
        default:
          pattern = argv[i];
          break;
     }
   }

   if (pattern.empty()){
     std::cerr << "LOL, wrong request! You forget about indicate pattern\n";
     return false;
   }

   return true;
 }