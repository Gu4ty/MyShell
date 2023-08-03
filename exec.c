
 #include "shell_lib.c"

int Build_in_cmd(Cmd * c){
    //TODO
    if(!strcmp(c->argv[0],"cd") ){
        
        if(chdir(c->argv[1]) == -1){
            perror("Error -> ");
        }
        return 1;
    }
    if(!strcmp(c->argv[0],"jobs")){
        show_bg();
        return 1;
    }
    if(!strcmp(c->argv[0],"fg")){
        errno=0;
        char * endptr;
        char * str = c->argv[1];
        long bg_index = strtol(str,&endptr,10);
        if ((errno == ERANGE && (bg_index == LONG_MAX || bg_index == LONG_MIN)) 
                                    || (errno != 0 && bg_index == 0)) {
            perror("Error -> ");
               
        }
        else if (endptr == str) {
            printf("No digits were found\n");
            
        }
        bg_index--;
        int pid = pid_bg[bg_index];
        status_bg[bg_index]=-1;
        free(line_bg[bg_index]);
        waitpid(pid,NULL,0);
        return 1;
    }
    return 0;
}

void exec_cmd(Cmd * c){

    char * redirect;
    int i=0, fd;
 
    while(c->redirects_in_files[i]){
        redirect = c->redirects_in_files[i];
        fd = open(redirect,O_RDWR,FILE_PERMISSIONS);
        if(fd==-1){
            perror("Error -> ");
           
            return;
        }
        dup2(fd,STDIN_FILENO);
        close(fd);
        i++;
    }
    i=0;
    while(c->redirects_out_files[i]){
        redirect = c->redirects_out_files[i];
        if(c->redirect_out_type[i]==1)
            fd=open(redirect,O_CREAT |O_RDWR | O_TRUNC,FILE_PERMISSIONS);
        else
            fd=open(redirect,O_CREAT | O_RDWR | O_APPEND,FILE_PERMISSIONS);
        if(fd==-1){
            perror("Error -> ");
            return;
        }
        dup2(fd,STDOUT_FILENO);
        close(fd);
        i++;
    }

    if(Build_in_cmd(c))
        return ;

    if(!strcmp(c->argv[0],"ls") || !strcmp(c->argv[0],"grep")   ){
        int j=0;
        char * arg = (char *) malloc(sizeof(char)*20);
        strcpy(arg,"--color=auto");
        add_arg(c,arg);
    }
    if(execvp(c->argv[0],c->argv) ==-1){
        
        printf("%s: command not found\n",c->argv[0]);
        exit(0);
    }


}

void exec_line(Cmd ** line){

    if(!line[1]){
        exec_cmd(line[0]);
        exit(0);
    }
    
    int i=0;
    int pid;
    Cmd * cmd;
    int pipefd[2];
    int fdin = dup(STDIN_FILENO);
     while(line[i]){
        pipe(pipefd);
        pid= fork();
        if(pid==0){
            signal(SIGINT,sigint_handler);
            close(pipefd[0]);
            if(line[i+1] != NULL)
                dup2(pipefd[1],STDOUT_FILENO);
            cmd= line[i];
            exec_cmd(cmd);
            exit(0);
        }
        else{
            close(pipefd[1]);
            dup2(pipefd[0],STDIN_FILENO);
            wait(NULL) ;
            i++;
        }
    }
    dup2(fdin,STDIN_FILENO);
    return ;
}


void execute(Cmd ** line, int bg, char * s){
    if(!bg && !line[1] && Build_in_cmd(line[0]))
        return;
    int pid= fork();
    if(pid==-1){
        printf("Could not create a new process...\n");
        return;
    }
    else if(!pid){
        signal(SIGINT,sigint_handler);
        exec_line(line);
        exit(0);
    }
    if(!bg){
        shell_waiting=1;
           
        waitpid(pid,NULL,0); 
        return;
    }
    char * bg_line;
    char * delim1=NULL;
    char * delim2=NULL;
    delim1 = strrchr(s,';');
    char * tempdelim2;
    while( (tempdelim2 =strstr(s,"&&") ) ) delim2=tempdelim2;
    if(delim1 == NULL) delim1=s;
    if(delim2 == NULL) delim2=s;
    if(delim1-s > delim2-s) 
        bg_line=delim1+1;
    else if(delim1-s < delim2-s)
        bg_line=delim2+1;
    else
        bg_line=s;
    add_bg(bg_line,pid);
    
    
    return ;
}

void exec_input(Cmd ** lines[],int bg, char * input){
    int i=0;
    while(lines[i]){
        if(lines[i+1] != NULL)
            execute(lines[i],0,input);
        else
        {
            execute(lines[i],bg,input);
        }
        
       i++;
    }
}

