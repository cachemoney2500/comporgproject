//
//  project.c
//  
//
//  Created by Courtney Moran on 4/21/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

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
    char instr[5];
    
    int j=0;
    while(str[j]!=' '){
        instr[j] = str[j];
        j++;
    }
    
    for (int i=0;i<10;i++){
        if (strcmp(instr,instructions[i])==0)
            return i;
    }
    return 100;
}

//perform the operation given in an instruction
//call when register values need to be updated
void operate(char * instr,int s[8],int t[10]){
    
    //assign a number to the instruction type
    int op = parse_instr(instr);
    //determine which register to store result in
    int s_or_t;
    int store = store_reg(instr,&s_or_t);
    
    int arg1; int arg2;
    //find first argument
    //skip past the instruction
    int j=0;
    while(instr[j]!=' ')
        j++;
    //skip past the store reg
    j+=6;
    if (instr[j]=='s')
        arg1 = s[instr[j+1]-'0'];
    else if (instr[j]=='t')
        arg1 = t[instr[j+1]-'0'];
    else
        arg1 = 0;
    //find second argument
    //skip ahead to the next argument
    while(instr[j]!=',')
        j++;
    j++;
    if (op%2==1){//if the instruction is 'immediate', arg2 is an integer
        //store the integer in an array
        char arg[10];
        int i=0;
        while(j<strlen(instr)){
            arg[i]=instr[j];
            i++; j++;
        }
        arg[i]='\0';
        //convert argument to an integer
        arg2 = atoi(arg);
    }else{
        j++; //skip past the '$'
        if (instr[j]=='s')
            arg2 = s[instr[j+1]-'0'];
        else if (instr[j]=='t')
            arg2 = t[instr[j+1]-'0'];
        else
            arg2 = 0;
    }
    
    //printf("arg1: %d\targ2: %d\n",arg1,arg2);
    
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
            
        default:
            break;
    }
    
}

//print out a specified number of dots
void print_dots(int dots){
    for(int i=0;i<dots;i++){
        printf("\t.");
    }
}

//print out asterisks for nop statements
void print_stars(int stars){
    for(int i=0;i<stars;i++){
        printf("\t*");
    }
}

//print out the stages
void print_stages(int stage,int repeat_stage,int repeat_cnt){
    for(int i=0;i<stage;i++){
        if (i+1==repeat_stage){
            for(int j=0;j<repeat_cnt;j++)
                printf("\t%s",stages[i]);
        }else
            printf("\t%s",stages[i]);
    }
}

//print the status of all the registers
void print_registers(int s[8],int t[10]){
    int i;
    for (i=0;i<8;i++){
        printf("$s%d = %d\t",i,s[i]);
        if(i%4==3)
            printf("\n");
    }
    for (i=0;i<10;i++){
        printf("$t%d = %d\t",i,t[i]);
        if(i%4==3||i==9)
            printf("\n");
    }
}


void simulation(char input[5][128],int len, bool forward){
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
    //check if nops are needed
    int nops[len];
    nops_check(input,len,nops);
    //keep track of which stage each instruction is on
    int status[len];
    status[0]=0;
    for(int x=1;x<len;x++)
        status[x]=-1;
    
    //print simulation steps
    int cycle = 0;
    int start_nops = 0;
    while (1){
        cycle++;
        printf("CPU Cycles ===>\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
        for(int i=0;i<MIN(cycle,len);i++){
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
            if (status[i]==5){
                operate(input[i],s,t);
            }
            
            //print out nops when needed
            if (start_nops){
                if(nops[i]&& !nops[i-1]){
                    for(int j=0;j<nops[i];j++){
                        printf("nop\t");
                        print_dots(i);
                        print_stages(2,0,0);
                        print_stars(MIN(3,cycle-2-i));
                        print_dots(7-i-MIN(3,cycle-2-i));
                        printf("\n");
                    }
                }
            }
            
            
            //print out each instruction w/ corresponding output stages
            printf("%s",input[i]);
            print_dots(i);
            switch (nops[i]) {
                case 1:
                    if (nops[i-1]==2){
                        if (status[i-2]<5){
                            print_stages(MIN(5,status[i]),1,cycle-i);
                            if (status[i]<1)
                                print_dots(9-i);
                            else if (status[i]==1 && cycle==i+1)
                                print_dots(9-i-1);
                            else
                                print_dots(9-MIN(5,status[i])-cycle+1);
                        }else{
                            print_stages(MIN(5,status[i]),1,3);
                            print_dots(9-MIN(5,status[i])-i-2);
                        }
                    }else{
                        if (status[i-2]<5){
                            print_stages(MIN(5,status[i]),2,cycle-i-1);
                            if (status[i]<1)
                                print_dots(9-i);
                            else if (status[i]==1 && cycle==i+1)
                                print_dots(9-i-1);
                            else
                                print_dots(9-MIN(5,status[i])-cycle+2);
                        }else{
                            print_stages(MIN(5,status[i]),2,2);
                            print_dots(9-MIN(5,status[i])-i-1);
                        }
                    }break;
                case 2:
                    if (status[i-1]<5){
                        print_stages(MIN(5,status[i]),2,cycle-i-1);
                        if (status[i]<1)
                            print_dots(9-i);
                        else if (status[i]==1 && cycle==i+1)
                            print_dots(9-i-1);
                        else
                            print_dots(9-MIN(5,status[i])-cycle+2);
                    }else{
                        print_stages(MIN(5,status[i]),2,3);
                        print_dots(9-MIN(5,status[i])-i-2);
                    }break;
                    
                    
                default:
                    print_stages(MIN(5,status[i]),0,0);
                    if (status[i]<0)
                        print_dots(9-i);
                    else
                        print_dots(9-MIN(5,status[i])-i);
                    break;
            }
            
            
            printf("\n");
        }
        printf("\n");
        print_registers(s,t);
        printf("----------------------------------------------------------------------------------\n");
        if(status[len-1]==5)
            break;
    }
    
    printf("END OF SIMULATION\n");
    
}


int main(int argc,char * argv[]){
    
    //read forwarding argument
    //store 1 for Forwarding, 0 for nonforwarding
    bool forward;
    if (argv[1][0]=='F') forward = true;
    else if (argv[1][0]=='N') forward = false;
    
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
    
    //run simulation
    simulation(str_in,i,forward);
    
    return EXIT_SUCCESS;
}

