
#include "shell_lib.c"
#define LINE_SIZE 10


int redirect_token_type(char * t){
    if(!strcmp(t,"<"))
        return 0;
    if(!strcmp(t,">"))
        return 1;
    if(!strcmp(t,">>"))
        return 2;
    return -1;

}

char ** make_tokens(char * input, char * lim){
    char * s=input;
    int index=0;

    char * * tokens;
    int tokens_count=0;
    char t;
    tokens = (char **)malloc(sizeof(char *) * strlen(input));
    while( s[0] && (lim - s >0)  ){
        t=s[0];
        if(t=='<' || t==';' || t=='|'){
            tokens[tokens_count] = (char *)malloc( sizeof(char) * (2) );
            tokens[tokens_count][0]=t;
            tokens[tokens_count][1]='\0';
            s++;
        }
        else if(s[0]=='&'){
            if(s[1]!='&'){
                tokens[tokens_count]= (char *)malloc( sizeof(char) * (2) );
                tokens[tokens_count][0]= '&';
                tokens[tokens_count][1]= '\0';
                s++;
            }
            else
            {
                tokens[tokens_count] = (char *)malloc( sizeof(char) * (3) );
                tokens[tokens_count][0]='&';
                tokens[tokens_count][1]='&';
                tokens[tokens_count][2]='\0';
                s+=2;
            }
            
        }
        else if(s[0]=='>'){
            if(s[1]== '>'){
                tokens[tokens_count] = (char *)malloc( sizeof(char) * (3) );
                tokens[tokens_count][0]='>';
                tokens[tokens_count][1]='>';
                tokens[tokens_count][2]='\0';
                s+=2;
            }
            else{
            tokens[tokens_count] = (char *)malloc( sizeof(char) * (2) );
            tokens[tokens_count][0]='>';
            tokens[tokens_count][1]='\0';
            s++;
            }

        }
        else{
            index=0;
            while(s[index] && s[index]!= '<' && s[index]!='>' && s[index]!='|' && s[index]!=';' && s[index]!='&' && s[index]!=' '  ){
                index++;
            }
            tokens[tokens_count]=(char *) malloc (sizeof(char) *(index+1)    ) ;
            for(int i=0;i<index;i++){
                tokens[tokens_count][i]= s[i];
            }
            tokens[tokens_count][index]='\0';
            s+=index;
        }
        tokens_count++;
    }
    tokens[tokens_count]=NULL;
    return tokens;
}

char ** tokenize(char * input_line){

    char * input = input_line;
    int line_size = strlen(input_line);
    char * * tokens= (char **) malloc( (line_size+1)* sizeof(char *) );
    int count=0;

    int i=0;
    char * * tokens_made;
    char * delimiter;
    while(*input && *input ==' ')
        input++;
    while( (delimiter = strchr(input,' '))){
        //*delimiter='\0';
        tokens_made = make_tokens(input,delimiter);
        i=0;
        while(tokens_made[i]){
            tokens[count++]=tokens_made[i++];
        }
        free(tokens_made);
        input = delimiter+1;
        while(*input && *input ==' ')
            input++;
    }
    if(input - input_line < line_size  ){
        tokens_made = make_tokens(input,input_line+line_size+1);
        i=0;
        while(tokens_made[i]){
            tokens[count++]=tokens_made[i++];
        }
        free(tokens_made);
    }
    tokens[count++]=NULL;
    return tokens;

}

void add_command(Cmd ** line,Cmd * c,int * i, int * size){
    if(*i == *size){
        *size = *size + 10;
        line = realloc(line, (*size) * sizeof(Cmd *));
    }
    line[*i]=c;
    *i = *i + 1;

}


void free_line(Cmd ** l){
    int i=0;
    while(l[i]){
        free_cmd(l[i++]);
    }
    free(l);
}
void print_parsed_line(Cmd ** l){
    int i=0;
    while(l[i]){
        printf("\n**********Command # %d: ****************\n",i+1);
        print_cmd(l[i]);
        printf("**********End of Command %d *****************\n",++i);
    }

}



Cmd * parse_command(char ** tokens,int * current_index){
    Cmd  * command = (Cmd *) malloc(sizeof(Cmd));
    init_cmd(command);

    int type_redirect=-1;
    while(tokens[*current_index]){
        char * current = tokens[*current_index];
        if(!strcmp(current,"&&") || !strcmp(current,"&") || !strcmp(current,"|") || !strcmp(current,";") )
            break;
        if(type_redirect != -1){
            add_redirect(command,current,type_redirect);
            type_redirect=-1;
            
        }
        else{
            type_redirect = redirect_token_type(current);
            if(type_redirect == -1)
                add_arg(command,current);
            else
                free(current);
        }
        *current_index = *current_index + 1;
    }
    add_arg(command,NULL);
    add_redirect(command,NULL,0);
    add_redirect(command,NULL,-1);
    return command;
}


Cmd ** parse_line(char ** tokens, int * current_index){
    
    int line_size = LINE_SIZE;
    int cmd_count=0;
    Cmd ** command_line = (Cmd **) malloc(line_size * sizeof(Cmd * ));
    Cmd * cmd;
    char * current_token;
    while(tokens[*current_index]){
        cmd = parse_command(tokens,current_index);
        add_command(command_line,cmd,&cmd_count,&line_size);
        current_token= tokens[*current_index];
        if(current_token && !strcmp(current_token,"|")){
            free(current_token);
            *current_index = *current_index + 1;continue;
        }
        break;
    }
    add_command(command_line,NULL,&cmd_count,&line_size);
    return command_line;
}




int parse_input(char * input, Cmd ** lines[]){
    char ** tokens = tokenize(input);
    int current_index=0;
    int lines_count=0;
    char * current_token;
    while(tokens[current_index]){
        Cmd ** line = parse_line(tokens,&current_index);
        lines[lines_count++]=line;
        current_token = tokens[current_index];
        if(!current_token) break;
        if(!strcmp(current_token,"&&") || !strcmp(current_token,";") ){
            free(current_token);
           
            current_index++;
        }
        else if(!strcmp(current_token,"&")){
            if(tokens[current_index+1]==NULL){
                 lines[lines_count]=NULL;
                 free(current_token);
                 free(tokens);
                return 1;
            }
            else{
                printf("Error : & operator must be at the end\n");
                 lines[lines_count]=NULL;
                 free(tokens);
                return -1;
            }
        }

    }
    lines[lines_count]=NULL;
    free(tokens);
    return 0;
}

