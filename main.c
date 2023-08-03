#include "parsing.c"
#include "exec.c"

int shell_pid;


int main()
{

 
    //Signals
    signal(SIGCHLD,sigchld_handler);
    signal(SIGINT,sigint_handler);
    
    //Init
    char * input;
    Cmd ** lines[100];
    shell_pid =getpid();
    for(int i=0;i<1000;++i) status_bg[i]=-1;
    
    while(1){
    
    //Printing current directory
    print_cwd();
    
    //Input
    input = readline();
    if(!strlen(input)){
        update_bg(); 
        continue;
    }
    
    if(!strcmp(input,"exit"))
        break;



    //Parsing
    int bg = parse_input(input,lines);
    
  
    //Exec
    exec_input(lines, bg, input);
    update_bg();

    //Free memory
    for(int i=0;lines[i];++i){
        free_line(lines[i]);
    }
    free(input);
    
    }
    free_bg();
    free(input);
    return 0;
}
