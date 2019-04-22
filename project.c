//
//  project.c
//  
//
//  Created by Courtney Moran on 4/21/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *intructions[10] = {"add", "addi", "and", "andi", "or", "ori", "slt", "slti", "beq", "bne"};
const char *stages[5] = {"IF","ID","EX","MEM","WB"};

//function that returns the number of the store register in
//an instruction & indicates whether its an s or t register
int store_reg(char * str,int * s_or_t){
    int j=0;
    while(str[j]!=' '){
        j++;
    }
    
    //save the store register
    if(str[j+2]=='s'){
        *s_or_t = 0;
    }else if(str[j+2]=='t'){
        *s_or_t = 1;
    }
    return str[j+3]-'0';
}

//function that checks one of the argument registers is used
//as a store register in another instruction
int arg_reg(char* str,char* prev_str){
    
    int j=0;
    while(str[j]!=' ')
        j++;
    
    int s_or_t;
    int prev_store = store_reg(prev_str,&s_or_t);
    //skip past the store reg
    j+=3;
    int new_reg = 0;
    
    while(j<strlen(str)){
        j++;
        if (str[j]=='$'){
            new_reg = 1;
            continue;
        }
        
        if (new_reg){
            new_reg=0;
            
            if((str[j]=='s' && !s_or_t)||(str[j]=='t' && s_or_t==1)){
                if((str[j+1]-'0')==prev_store)
                    return 1;
            }j++;
        }
    }
    return 0;
}

//check if nops should be added
void nops_check(char input[5][128],int len,int * nops){
    //set first value to 0
    *nops = 0; nops++;
    //check if nops are needed in succeeding instructions
    for(int i=1;i<len;i++){
        if(arg_reg(input[i],input[i-1]))
            *nops = 2;
        else if(i>1 && arg_reg(input[i],input[i-2]))
            *nops = 1;
        else
            *nops = 0;
        nops++;
    }
}

//function to determine instruction type
int parse_instr(char * str){
    char instr[4];
    
    j=0;
    while(str[j]!=' '){
        instr[j] = str[j];
    }
    
    for (int i=0;i<10;i++){
        if (strcmp(instr,instructions[i])==0)
            return i;
    }
}


int main(int argc,char * argv[]){
    
    //read forwarding argument
    //store 1 for Forwarding, 0 for nonforwarding
    int forward;
    if (argv[1][0]=='F') forward = 1;
    else if (argv[1][0]=='N') forward = 0;
    
    //read input file
    FILE* file;
    char str_in[10][128];
    file = fopen(argv[2],"r");
    int i = 0;
    while(fgets(str_in[i],128,file)){
        //get rid of trailing newline
        strtok(str_in[i],"\n");
        i++;
    }
    fclose(file);
    
    return EXIT_SUCCESS;
}
