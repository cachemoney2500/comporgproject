//
//  project.c
//
//
//  Created by Courtney Moran, 
//  Matthew Solomon, Megan Broga, 
//  and Christine Koulopoulos
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

const char *instructions[10] = {"add", "addi", "and", "andi", "or", "ori", "slt", "slti", "beq", "bne"};
const char *stages[5] = {"IF","ID","EX","MEM","WB"};

void print_registers(int s[8],int t[10]);

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
    //skip past the instruction
    int j=0;
    while(str[j]!=' ')
        j++;
    
    int s_or_t;
    //check which store register the previous instruction uses
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
void nops_check(char input[16][128],int len,int * nops){
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

//check if a string is a label
int r_u_a_label(char * str){
    int i=0;
    while(str[i]!=' '){
        if(str[i]==':')
            return 1;
        i++;
    }
    return 0;
}

//check which label a string is
int which_label(char str[128],char labels[5][128],int label_count){
    for(int i=0;i<label_count;i++){
        if(strcmp(str,labels[i])==0){
            return i;
        }
    }return 100;
}

//check if the input has any labels, if so store them & where they appear
//return the number of labels
int labels_check(char input[16][128],int len,char labels[5][128],int label_idx[5]){
    int j;
    int label_count = 0;
    for(int i=0;i<len;i++){
        if(r_u_a_label(input[i])){
            j=0;
            while(input[i][j]!=':'){
                labels[label_count][j] = input[i][j];
                j++;
            }
            labels[label_count][j] = '\0';
            label_idx[label_count]=i; //store the index of the label
            label_count++;
        }
    }return label_count;
}

//function to determine instruction type
int parse_instr(char * str){
    char instr[5];
    
    int j=0;
    while(str[j]!=' '){
        instr[j] = str[j];
        j++;
    }
    instr[j] = '\0';
    
    
    for (int i=0;i<10;i++){
        if (strcmp(instr,instructions[i])==0)
            return i;
    }
    return 100;
}

//perform the operation given in an instruction
//call when register values need to be updated
//return (1+label index) if a branch occurs, otherwise 0
int operate(char * instr,int s[8],int t[10],char labels[5][128],int label_count){
    
    //assign a number to the instruction type
    int op = parse_instr(instr);
    //determine which register to store result in
    int s_or_t;
    int store = store_reg(instr,&s_or_t);
    
    int arg1; int arg2;
    char label[128];
    int branch=0;
    //find first argument
    //skip past the instruction
    int j=0;
    while(instr[j]!=' ')
        j++;
    
    if(op>7){
        if(!s_or_t){
            arg1 = s[store];
        }else
            arg1 = t[store];
    }
    //skip past the store reg
    j+=6;
    if (instr[j]=='s'){
        if(op<8)
            arg1 = s[instr[j+1]-'0'];
        else
            arg2 = s[instr[j+1]-'0'];
    }else if (instr[j]=='t'){
        if(op<8)
            arg1 = t[instr[j+1]-'0'];
        else
            arg2 = t[instr[j+1]-'0'];
    }else{
        if(op<8)
            arg1 = 0;
        else
            arg2 = 0;
    }
    //find second argument
    //skip ahead to the next argument
    while(instr[j]!=',')
        j++;
    j++;
    if (op%2==1 || op>7){//if the instruction is 'immediate', arg2 is an integer
        //if its a branch instr, we want to store the label
        //store the integer in an array
        char arg[128];
        int i=0;
        while(j<strlen(instr)){
            arg[i]=instr[j];
            i++; j++;
        }
        arg[i]='\0';
        //convert argument to an integer
        if(op<8)
            arg2 = atoi(arg);
        else
            strcpy(label,arg);
    }else{
        j++; //skip past the '$'
        if (instr[j]=='s')
            arg2 = s[instr[j+1]-'0'];
        else if (instr[j]=='t')
            arg2 = t[instr[j+1]-'0'];
        else
            arg2 = 0;
    }
    
    //perform operation & store result in register
    switch (op/2) {
        case 0: //add operation
            if (!s_or_t)
                s[store] = arg1 + arg2;
            else
                t[store] = arg1 + arg2;
            break;
        case 1: //and operation
            if (!s_or_t)
                s[store] = arg1 & arg2;
            else
                t[store] = arg1 & arg2;
            break;
        case 2: //or operation
            if (!s_or_t)
                s[store] = arg1 | arg2;
            else
                t[store] = arg1 | arg2;
            break;
        case 3: //slt operation
            if (!s_or_t)
                s[store] = (arg1 < arg2);
            else
                t[store] = (arg1 < arg2);
            break;
        case 4://branch operation
            branch = (op%2) ? arg1==arg2 : arg1!=arg2;
            break;
            
        default:
            break;
    }
    if (branch){
        return 1+which_label(label,labels,label_count);
    }
    return 0;
}

//print out a specified number of dots
void print_dots(int dots,int before_or_after){
    for(int i=0;i<dots;i++){
        //printf("%d %d",dots,i);
        //printf("\n" );
        //printf("%d",i);
        //printf("\n");
        if (before_or_after == 1 && i == dots-1) {
            printf(".");
            return;
        }
        
        else {
            
            printf(".   ");
            
        }
    }
}

//print out asterisks for nop statements
void print_stars(int stars){
    for(int i=0;i<stars;i++){
        printf("*   ");
    }
}



//print out the stages
void print_stages(int stage,int repeat_stage,int repeat_cnt){
    for(int i=0;i<stage;i++){
        if (i+1==repeat_stage){
            for(int j=0;j<repeat_cnt;j++) {
                printf("%s",stages[i]);
                int spaces = 4 - strlen(stages[i]);
                for(int k=0;k<spaces;k++) {
                    printf(" ");
                }
            }
        }
        else {
            printf("%s",stages[i]);
            int spaces = 4 - strlen(stages[i]);
            for(int k=0;k<spaces;k++) {
                printf(" ");
            }
        }
    }
}

//print the status of all the registers
void print_registers(int s[8],int t[10]){
    int i;
    for (i=0;i<8;i++){
        if(i%4 == 3){
            printf("$s%d = %d",i,s[i]);
        }
        else{
            printf("$s%d = %-14d",i,s[i]);
        }
        if(i%4==3)
            printf("\n");
    }
    
    for (i=0;i<10;i++){
        if(i%4 == 3 || i==9){
            printf("$t%d = %d",i,t[i]);
        }
        else{
            printf("$t%d = %-14d",i,t[i]);
        }
        if(i%4==3||i==9)
            printf("\n");
    }
}

//make an array of only instructions, without labels
void extract_instructions(char input[16][128],char instr[16][128],int len,int label_idx[5]){
    int idx_count = 0;
    int instr_count = 0;
    for(int i=0;i<len;i++){
        if (i==label_idx[idx_count]){
            idx_count++;
        }else{
            strcpy(instr[instr_count],input[i]);
            instr_count++;
        }
    }
}



void simulation(char str_in[16][128],int len, int forward){
    printf("START OF SIMULATION ");
    if(!forward){
        printf("(no forwarding)\n");
    }
    else{
        printf("(forwarding)\n");
    }
    printf("----------------------------------------------------------------------------------\n");
    
    //initialize all register values
    int s[8] = {0,0,0,0,0,0,0,0};
    int t[10] = {0,0,0,0,0,0,0,0,0,0};
    
    //get an array of only instructions (no labels) & mark where the labels are
    char instr[16][128];
    char labels[5][128];
    int label_idx[5];
    label_idx[0]=100;
    int label_count = labels_check(str_in,len,labels,label_idx);
    extract_instructions(str_in,instr,len,label_idx);
    len = len - label_count;
    //create an array to store all instructions (including repeated ones)
    char all_instr[32][128];
    for(int x=0;x<len;x++){
        strcpy(all_instr[x],instr[x]);
    }
    
    //check if nops are needed
    int nops[32];
    //skip nops if using forwarding
    if(forward){
        for(int x=0;x<32;x++)
            nops[x]=0;
    }else
        nops_check(instr,len,nops);
    //keep track of which stage each instruction is on
    int status[32];
    for(int x=0;x<32;x++)
        status[x]=0;
    
    //print simulation steps
    int cycle = 0;
    int length = len;//length of all instr
    int start_nops = 0;
    while (1){
        cycle++;
        printf("CPU Cycles ===>     1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16\n");
        for(int i=0;i<MIN(cycle,length);i++){
            //handle incrementing which stage each intruction is on based how many nops
            switch (nops[i]) {
                case 1:
                    if (nops[i-1]==2){
                        if (status[i]==1){
                            if (status[i-2]==6)
                                status[i]++;
                        }else if (cycle>i-1) status[i]++;
                    }else if (status[i]==2){
                        start_nops = 1;
                        if (status[i-2]==6)
                            status[i]++;
                    }else{
                        if (cycle>i-1) status[i]++;
                    }
                    break;
                case 2:
                    if (status[i]==2){
                        start_nops = 1;
                        if (status[i-1]==6)
                            status[i]++;
                    }else{
                        if (cycle>i-1) status[i]++;
                    }break;
                    
                default:
                    if (cycle>i-1) status[i]++;
                    break;
            }
            
            //update register values if status is at 'WB'
            //see if a branch is needed
            if (status[i]==5){
                int branch = operate(all_instr[i],s,t,labels,label_count);
                if(branch){
                    for(int y=label_idx[branch-1]-branch+1;y<len;y++){
                        strcpy(all_instr[y+MIN(i+3,length)],instr[y]);
                    }
                    length += len-label_idx[branch-1]+branch-1-MAX(length-i-4,0);
                }
            }
            
            //print out nops when needed
            if (start_nops){
                if(nops[i]&& !nops[i-1]){
                    for(int j=0;j<nops[i];j++){
                        printf("nop                 ");
                        print_dots(i,0);
                        print_stages(2,0,0);
                        print_stars(MIN(3,cycle-2-i));
                        print_dots(14-i-MIN(3,cycle-2-i),1);
                        printf("\n");
                    }
                }
            }
            
            
            //print out each instruction w/ corresponding output stages
            printf("%s",all_instr[i]);
            int spaces = 20-strlen(all_instr[i]);
            for (int j=0;j<spaces;j++) {
                printf(" ");
            }
            print_dots(i,0);
            
            switch (nops[i]) {
                case 1:
                    if (nops[i-1]==2){
                        if (status[i-2]<5){
                            print_stages(MIN(5,status[i]),1,cycle-i);
                            if (status[i]<1)
                                print_dots(16-i,1);
                            else if (status[i]==1 && cycle==i+1)
                                print_dots(16-i-1,1);
                            else
                                print_dots(16-MIN(5,status[i])-cycle+1,1);
                        }else{
                            print_stages(MIN(5,status[i]),1,3);
                            print_dots(16-MIN(5,status[i])-i-2,1);
                        }
                    }else{
                        if (status[i-2]<5){
                            print_stages(MIN(5,status[i]),2,cycle-i-1);
                            if (status[i]<1)
                                print_dots(16-i,1);
                            else if (status[i]==1 && cycle==i+1)
                                print_dots(16-i-1,1);
                            else
                                print_dots(16-MIN(5,status[i])-cycle+2,1);
                        }else{
                            print_stages(MIN(5,status[i]),2,2);
                            print_dots(16-MIN(5,status[i])-i-1,1);
                        }
                    }break;
                case 2:
                    if (status[i-1]<5){
                        print_stages(MIN(5,status[i]),2,cycle-i-1);
                        if (status[i]<1)
                            print_dots(16-i,1);
                        else if (status[i]==1 && cycle==i+1)
                            print_dots(16-i-1,1);
                        else
                            print_dots(16-MIN(5,status[i])-cycle+2,1);
                    }else{
                        print_stages(MIN(5,status[i]),2,3);
                        print_dots(16-MIN(5,status[i])-i-2,1);
                    }break;
                    
                    
                default:
                    print_stages(MIN(5,status[i]),0,0);
                    if (status[i]<0)
                        print_dots(16-i,1);
                    else{
                        print_dots(16-MIN(5,status[i])-i,1);
                    }
                    break;
            }
            
            
            printf("\n");
        }
        printf("\n");
        print_registers(s,t);
        printf("----------------------------------------------------------------------------------\n");
        if(status[length-1]==5 || cycle==16){
            break;
        }
    }
    
    printf("END OF SIMULATION\n");
    
}


int main(int argc,char * argv[]){
    
    //read forwarding argument
    //store 1 for Forwarding, 0 for nonforwarding
    int forward;
    if (argv[1][0]=='F') forward = 1;
    else if (argv[1][0]=='N') forward = 0;
    
    //read input file
    FILE* file;
    char str_in[16][128];
    file = fopen(argv[2],"r");
    int i = 0;
    while(fgets(str_in[i],128,file)){
        //get rid of trailing newline
        strtok(str_in[i],"\n");
        i++;
    }
    fclose(file);
    
    //run simulation
    simulation(str_in,i,forward);
    
    return EXIT_SUCCESS;
}
