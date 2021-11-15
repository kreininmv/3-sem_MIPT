#include "micr.h"

int signal_num;

static void sigint_handler(int sig_num){
    signal_num = sig_num;
}

error_code mikrosha::start(void){

    pipeline pipe{};
    std::string comm_line{};
    signal(SIGINT, sigint_handler);

    while(true){
        signal_num = 0;
        command::print_intro_line(std::cout);

        if (!getline(std::cin, comm_line)){
            std::cout << std::endl;
            break;
        }
        if (signal_num == SIGINT){
            continue;
        }

        pipe.reset_pipeline(comm_line);
        pipe.exec();
    }

    return SUCCESS;
}