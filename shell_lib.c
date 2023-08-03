#ifndef SHELL_LIB
#define SHELL_LIB

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include<sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#define FILE_PERMISSIONS S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH


int shell_waiting;



void print_cwd(){
    char * cwd = getcwd(NULL,0);
    printf("%s$ ",cwd);
    free(cwd);
}


int status_bg[1000];
char * line_bg[1000];
int pid_bg[1000];

void add_bg(char * line, int pid){
    for(int i=0;i<1000;++i)
        if(status_bg[i]== -1 ){
            status_bg[i]=1;
            line_bg[i]= (char *) malloc(strlen(line)*sizeof(char));
            strcpy(line_bg[i],line);
            char * end = strrchr(line_bg[i],'&');
            *end='\0';
            pid_bg[i]=pid;
            printf("[%d] %d\n",i+1,pid);
            break;
        }

}

void show_bg(){
    for(int i=0;i<1000;i++){
        if(status_bg[i]==1 ){
            printf("[%d]+   Running\t\t%s\n",pid_bg[i],line_bg[i]);
        }
        else if(status_bg[i]==0)
        {
            printf("[%d]-   Done\t\t%s\n",pid_bg[i],line_bg[i]);
        }
        
    }
}

void update_bg(){
    for(int i=0;i<1000;++i)
        if(status_bg[i]==0){
            printf("[%d]+   Done        %s\n",i+1,line_bg[i]);
            status_bg[i]=-1;
            free(line_bg[i]);
        }
}

void clear_bg(int pid){
    for(int i=0;i<1000;i++)
        if(pid_bg[i]==pid){
            status_bg[i]=0;
        }
}

void free_bg(){
    for(int i=0;i<1000;i++){
        if(status_bg[i]!=-1)
            free(line_bg[i]);
    }
}

char * readline(){
    shell_waiting=0;
    char * buffer=NULL;
    int size= 0;
    int i=0;
    char c;
    while(1){
        c=getchar();
        if(i==size){
            size+=BUFSIZ;
            buffer = realloc(buffer,size);
        }
        buffer[i++]=c;
        if(c=='\n'){
            buffer[i-1]='\0';break;
        }
    }

    return buffer;

}


//Command
typedef struct cmd{
    //Arguments
    char ** argv;
    int argv_size;
    int argv_count;

    //Redirects in
    int count_in_redirect;
    int size_in_redirect;
    char * * redirects_in_files;

    //Redirects out
    int count_out_redirect;
    int size_out_redirect;
    char * * redirects_out_files;
    int * redirect_out_type;

}Cmd;


void init_cmd(Cmd * c){
    c->argv_count=0;
    c->argv_size=0;
    c->argv=NULL;
    c->count_in_redirect=0;
    c->redirects_in_files=NULL;

    c->size_in_redirect=0;
    c->count_out_redirect=0;
    c->redirects_out_files=NULL;
    c->redirect_out_type=NULL;
    c->size_out_redirect=0;
}

void add_arg(Cmd * c , char * a){
    int i= c->argv_count;
    int size= c->argv_size;
    if(i== size ){
        size+=10;
        c->argv = realloc(c->argv,size* (sizeof(char *)));
        c->argv_size=size;
    }
    c->argv[i++]=a;
    if(a!=NULL)
        c->argv_count=i;

}

void add_redirect(Cmd * c,char * arg,int type){

    if(!type ){
        int i= c->count_in_redirect;
        int size = c->size_in_redirect;
        if(i== size ){
            size+=10;
            c->redirects_in_files = realloc(c->redirects_in_files,size*(sizeof(char *)));

        }
        c->redirects_in_files[i++]=arg;
        c->count_in_redirect=i;
        c->size_in_redirect=size;
    }
    else{
        int i= c->count_out_redirect;
        int size = c->size_out_redirect;
        if(i==size){
            size+=10;
            c->redirects_out_files= realloc(c->redirects_out_files,size*(sizeof(char *)));
            c->redirect_out_type = realloc(c->redirect_out_type,size*(sizeof(int)));
        }
        c->redirect_out_type[i]=type;
        c->redirects_out_files[i++]=arg;
        c->count_out_redirect=i;
        c->size_out_redirect=size;
    }




}

void print_cmd(Cmd * c ){
    int count = c->argv_count;
    printf("Command name: \n%s\n",c->argv[0]);
    printf("Arguments: \n");
    for(int i=1;i<count;i++){
        printf("%s\n",c->argv[i]);
    }
    printf("\n Redirects in:\n");
    int i=0;
    char * * tokens= c->redirects_in_files;
    while(tokens[i]){
        printf("%s\n",tokens[i++]);
    }
    printf("\nRedirects out:\n");
    i=0;
    tokens= c->redirects_out_files;
    while(tokens[i]){
        printf("%s ",tokens[i]);
        if(c->redirect_out_type[i++]==1)
            printf("of type >\n");
        else
            printf("of type >>\n");
    }
}

void free_cmd(Cmd * com){

    int i=0;
    while(com->redirects_in_files[i]){
        free(com->redirects_in_files[i++]);
    }
    free(com->redirects_in_files);

    i=0;
    while(com->redirects_out_files[i]){
        free(com->redirects_out_files[i++]);
    }
    free(com->redirects_out_files);

    i=0;
    while(com->argv[i]){
        free(com->argv[i++]);
    }
    free(com->argv);
    free(com->redirect_out_type);
    free(com);

}


void sigchld_handler(int signum){
    int pid= wait(NULL);
    clear_bg(pid);
}

void sigint_handler(int signum){
    if(shell_waiting)
        printf("\n");
}


#endif