//
//  project.c
//  
//
//  Created by Courtney Moran on 4/21/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc,char * argv[]){
    
    //read forwarding argument
    //store 1 for Forwarding, 0 for nonforwarding
    int forward;
    if (argv[1][0]=='F') forward = 1;
    else if (argv[1][0]=='N') forward = 0;
    
    //read input file
    FILE* file;
    char str_in[10][128];
    file = fopen(argv[1],"r");
    int i = 0;
    while(fgets(str_in[i],128,file)){
        //get rid of trailing newline
        strtok(str_in[i],"\n");
        i++;
    }
    fclose(file);
    
    return EXIT_SUCCESS;
}
