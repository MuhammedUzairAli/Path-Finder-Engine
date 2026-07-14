#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
/*
N is normal road, P is selected best path
B is blocked road or normal terrain
H is a hub from witch tranportation must occur and V is a supply deposit point
*/
enum terrain_types {B=0,P=1,N=2,H=3,V=4};
// a node is point on grid
typedef struct {

    int closest_x;
    int closest_y;
    
    int terrain_type;
    float cost;
    float risk; // also serves as traffic(though for that it is recomended to keep value low from 0 to 1.5)
    bool seen;
    
    int hubn;
    int viln;

} node;
// a default constructor
node constructor(){
    return (node){-1,-1,B,INFINITY,0,false,0,0};
}
// func for indexing my 1d array to map it to a 2d array
int indx(int y,int x,int max_x){
    return (y * max_x + x);
}

typedef struct{
    int x;
    int y;
    float cost;
}heapN; //heap node
//check assign map code
void assign_map(FILE* pterrain,node map[],int cols,int rows){
    
    int vilnum=0;
    int hubnum=0;
    int j=0;
    int i=0;
    int count=0;
    char buffer[300];
    while(fgets(buffer,300, pterrain) != NULL && i < rows){ //the i < rows is redundent its here to be safe
        j=0;
        count = 0;
        while(count < cols && buffer[j] != '\n' && buffer[j] != '\0'){
            if(buffer[j]>='0' && buffer[j]<='9'){//this also ignores negative signs wich are to be ignored
                char* pEnd;
                map[indx(i,count,cols)].risk += strtof(&buffer[j],&pEnd); //i arrived at this to be best for extracting number
                j=(pEnd-buffer);
            }
            switch (toupper(buffer[j])){
                case ',':
                    count++;
                    break;
                case 'N':
                    map[indx(i,count,cols)].terrain_type = N; 
                    break;
                case 'V':
                    map[indx(i,count,cols)].terrain_type = V;
                    vilnum++;
                    map[indx(i,count,cols)].viln = vilnum;
                    break;
                case 'H':
                    map[indx(i,count,cols)].terrain_type = H; 
                    hubnum++;
                    map[indx(i,count,cols)].hubn = hubnum;
                    break;
                case 'B':
                    map[indx(i,count,cols)].terrain_type = B; 
                    break;      
            }
            j++;
        }
        i++;
    }
}
void print_map(node map[],int cols,int rows){
    int i;
    int j;
    for(i=0;i<rows;i++){
        for(j=0;j<cols;j++){
            printf("%d",map[indx(i,j,cols)].terrain_type);
        }
    }
}

void swap_nodes(heapN* a,heapN* b){
    heapN temp = *b;
    *b = *a;
    *a = temp;
}
void push_heap(heapN heap[],int y, int x,int cost,int* size){
    heap[*size].x = x;
    heap[*size].y = y;
    heap[*size].cost = cost;
    int i = *size;
    (*size)++;
    while(i > 0 && heap[i].cost < heap[(i-1)/2].cost){
        swap_nodes(&heap[i],&heap[(i-1)/2]);
        i = (i-1)/2;
    }
}
heapN pop_heap (heapN heap[],int* size){
    heapN root;
    root.y = -1;
    root.x = -1;
    root.cost = INFINITY;
    if  (*size != 0){
        root = heap[0];
        heap[0] = heap[*size-1]; //size - 1 is last element
        (*size)--;
        int i = 0;
        int smaller; //smaller assumed left for now
        if((2*i + 1)<= *size-1){//for left
            smaller = 2*i + 1;//smaller assumed left for now
            if((2*i + 2)<= *size-1 && heap[2*i + 2].cost < heap[2*i + 1].cost){//heap to right less than heap to left
                smaller = 2*i + 2;
            }
        }
        while((2*i + 1)<= *size-1 && heap[i].cost > heap[smaller].cost){//atleast left must be within size if i has no left we are at a leaf in bin tree
            swap_nodes(&heap[i],&heap[smaller]);
            i = smaller;
            if((2*i + 1)<= *size-1){//for left
                smaller = 2*i + 1;//smaller assumed left for now
                if((2*i + 2)<= *size-1 && heap[2*i + 2].cost < heap[2*i + 1].cost){//heap to right less than heap to left
                    smaller = 2*i + 2;
                }
            }
            else{
                break;
            }
        }
    }
    return root;
}

bool path(node map[],int cols,int rows,int x,int y){
    int next_x = map[indx(y,x,cols)].closest_x;
    int next_y = map[indx(y,x,cols)].closest_y;
    if(next_x != -1 || next_y != -1){
        map[indx(y,x,cols)].terrain_type = P;
    }
    else {
        return true;
    }
    if(!(path(map,cols,rows,next_x,next_y))){ //path isnt finished
        return false;
    }
    return true;
}
bool sub_router(node map[],int cols,int rows,int hubn,int viln,int hubx,int huby){
    
    heapN heap[cols*rows];
    map[indx(huby,hubx,cols)].cost = 0; //setting the current given hub to 0
    map[indx(huby,hubx,cols)].seen = true; //setting it to seen
    int size = 0;
    push_heap(heap,huby,hubx,0,&size);

    while(true){  
        heapN least = pop_heap(heap,&size);
        int lx=least.x;
        int ly=least.y;
        float leastcost = least.cost;

        if(lx == -1 || ly ==-1 ){ //in case it goes through map without findind path
            return false;
        }
        map[indx(ly,lx,cols)].seen = true; //what is popped from heap is seen
        
        if (map[indx(ly,lx,cols)].terrain_type == V && map[indx(ly,lx,cols)].viln == viln){
            //the path village node points to is given by closest x and closest y
            return path(map,cols,rows,map[indx(ly,lx,cols)].closest_x,map[indx(ly,lx,cols)].closest_y);
        }
        else{ // these checks are to make sure these neighbors arent outside grid
            if(!(ly-1 < 0 || lx-1<0)){
                if(!map[indx(ly-1,lx-1,cols)].seen && map[indx(ly-1,lx-1,cols)].terrain_type != B && map[indx(ly-1,lx-1,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly-1,lx-1,cols)].risk + 1.4){//if a tile was seen that means they already had lowest possible cost due to this being a grid
                    //this is a diagonal so 1.4 added
                    map[indx(ly-1,lx-1,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly-1,lx-1,cols)].risk + 1.4; //cost of prior plus risk of this plus travel cost
                    map[indx(ly-1,lx-1,cols)].closest_x = lx;
                    map[indx(ly-1,lx-1,cols)].closest_y = ly;
                    push_heap(heap,ly-1,lx-1,map[indx(ly-1,lx-1,cols)].cost,&size);
                }
            }
            if(!(ly-1 < 0)){
                if(!map[indx(ly-1,lx,cols)].seen && map[indx(ly-1,lx,cols)].terrain_type != B && map[indx(ly-1,lx,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly-1,lx,cols)].risk + 1){
                    map[indx(ly-1,lx,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly-1,lx,cols)].risk + 1;
                    map[indx(ly-1,lx,cols)].closest_x = lx;
                    map[indx(ly-1,lx,cols)].closest_y = ly;
                    push_heap(heap,ly-1,lx,map[indx(ly-1,lx,cols)].cost,&size);
                }
            }
            if(!(lx-1 < 0)){
                if(!map[indx(ly,lx-1,cols)].seen && map[indx(ly,lx-1,cols)].terrain_type != B && map[indx(ly,lx-1,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly,lx-1,cols)].risk + 1){
                    map[indx(ly,lx-1,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly,lx-1,cols)].risk + 1; 
                    map[indx(ly,lx-1,cols)].closest_x = lx;
                    map[indx(ly,lx-1,cols)].closest_y = ly;
                    push_heap(heap,ly,lx-1,map[indx(ly,lx-1,cols)].cost,&size);
                }
            }
            if(ly+1 < rows && lx+1 < cols){
                if(!map[indx(ly+1,lx+1,cols)].seen && map[indx(ly+1,lx+1,cols)].terrain_type != B && map[indx(ly+1,lx+1,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly+1,lx+1,cols)].risk + 1.4 ){
                    map[indx(ly+1,lx+1,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly+1,lx+1,cols)].risk + 1.4; //diagonal
                    map[indx(ly+1,lx+1,cols)].closest_x = lx;
                    map[indx(ly+1,lx+1,cols)].closest_y = ly;
                    push_heap(heap,ly+1,lx+1,map[indx(ly+1,lx+1,cols)].cost,&size);
                }
            }
            if(ly+1 < rows){
                if(!map[indx(ly+1,lx,cols)].seen && map[indx(ly+1,lx,cols)].terrain_type != B && map[indx(ly+1,lx,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly+1,lx,cols)].risk + 1){
                    map[indx(ly+1,lx,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly+1,lx,cols)].risk + 1; 
                    map[indx(ly+1,lx,cols)].closest_x = lx;
                    map[indx(ly+1,lx,cols)].closest_y = ly;
                    push_heap(heap,ly+1,lx,map[indx(ly+1,lx,cols)].cost,&size);
                }
            }
            if(lx+1 < cols){
                if(!map[indx(ly,lx+1,cols)].seen && map[indx(ly,lx+1,cols)].terrain_type != B && map[indx(ly,lx+1,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly,lx+1,cols)].risk + 1){
                    map[indx(ly,lx+1,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly,lx+1,cols)].risk + 1; 
                    map[indx(ly,lx+1,cols)].closest_x = lx;
                    map[indx(ly,lx+1,cols)].closest_y = ly;
                    push_heap(heap,ly,lx+1,map[indx(ly,lx+1,cols)].cost,&size);
                }
            }
            if(!(ly-1 < 0) && lx+1 < cols){
                if(!map[indx(ly-1,lx+1,cols)].seen && map[indx(ly-1,lx+1,cols)].terrain_type != B && map[indx(ly-1,lx+1,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly-1,lx+1,cols)].risk + 1.4){
                    map[indx(ly-1,lx+1,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly-1,lx+1,cols)].risk + 1.4; //diagonal
                    map[indx(ly-1,lx+1,cols)].closest_x = lx;
                    map[indx(ly-1,lx+1,cols)].closest_y = ly;
                    push_heap(heap,ly-1,lx+1,map[indx(ly-1,lx+1,cols)].cost,&size);
                }
            }
            if(ly+1 < rows && !(lx-1 < 0)){
                if(!map[indx(ly+1,lx-1,cols)].seen && map[indx(ly+1,lx-1,cols)].terrain_type != B && map[indx(ly+1,lx-1,cols)].cost > map[indx(ly,lx,cols)].cost + map[indx(ly+1,lx-1,cols)].risk + 1.4){
                    map[indx(ly+1,lx-1,cols)].cost = map[indx(ly,lx,cols)].cost + map[indx(ly+1,lx-1,cols)].risk + 1.4; //diagonal
                    map[indx(ly+1,lx-1,cols)].closest_x = lx;
                    map[indx(ly+1,lx-1,cols)].closest_y = ly;
                    push_heap(heap,ly+1,lx-1,map[indx(ly+1,lx-1,cols)].cost,&size);
                }
            }
        }
    }
} 
void router(node map[],node oldmap[],int cols,int rows,int hubs,int vils){
    int i;
    int j;
    int hubnum = 1;
    int vilnum;
    int hubx;
    int huby;
    bool hub_found;

    if (hubs == 0 || vils == 0) {
        return; //do nothing python can detect if hub 0 or vil 0
    }
    
    while (hubnum <= hubs){
        vilnum = 1;
        hubx = 0;
        huby = 0;
        hub_found = false;
        int x;
        for(i=0; i<rows ;i++){
            for(j=0; j<cols;j++){
                if(map[indx(i,j,cols)].terrain_type == H && map[indx(i,j,cols)].hubn == hubnum){
                    huby = i;
                    hubx = j;
                }
            }
        }
        
        while(vilnum <= vils){
            if(!(sub_router(map,cols,rows,hubnum,vilnum,hubx,huby))){
                printf("7%d%d",hubnum,vilnum); //error code to python
                /*log10(n) +1 number of digits if decimal part truncated wich int is used for 
                this is so the number of digits of hubnum and vil num is accounted for*/
                for(x=1+(int)(log10(hubnum)+1)+(int)(log10(vilnum)+1);x<cols*rows;x++){ 
                    printf("0"); //the rest of the map will be 0s
                }
                memcpy(map , oldmap ,cols * rows * sizeof(node));
            }
            else{
                print_map(map,cols,rows);
                memcpy(map , oldmap ,cols * rows * sizeof(node));
            }
            vilnum++;
        }
        hubnum++;
    }
}

int main(){
    
    int cols=0;
    int rows=0;
    int tempcols = 0;
    int hubs=0;
    int vils=0;

    char buffer[300]; //300 to account for commas and extra chars
    
    FILE *pterrain = fopen("terrain.csv","r");
    if (!pterrain){
        printf("failed to open file");
        return 1;
    }

    int i=0;
    int j=0;
    //code for getting cols and rows
    while (fgets(buffer,300,pterrain) != NULL && buffer[0] != '\n' ){
        tempcols = 0;
        for(i=0; buffer[i] != '\n' && buffer[i] != '\0' ; i++){
            if(buffer[i] == ','){ 
                tempcols ++;
            }
            else if(toupper(buffer[i]) == 'H'){
                hubs++;
            }
            else if(toupper(buffer[i]) == 'V'){
                vils++;
            }   
        }
        if (tempcols > cols){
            cols = tempcols;
        }
        j++;
    }
    rows = j;
     
    node *map = malloc(cols * rows * sizeof(node));
    if (map == NULL){
       printf("error");
       return 1; 
    }
    
    node *oldmap = malloc(cols * rows * sizeof(node)); // using 1d array that will be accessed like 2d for memory saving
    if (oldmap == NULL){
       printf("error");
       return 1; 
    }
    
    rewind(pterrain);
    for(i=0;i<cols*rows;i++){
        map[i]= constructor();
    }

    assign_map(pterrain,map,cols,rows);
    memcpy(oldmap , map ,cols * rows * sizeof(node)); //destinaion,source,size
    printf("%d,%d,%d,%d,",rows,cols,hubs,vils); //values we can give python instead of python reading from csv
    print_map(oldmap,cols,rows); //output the old_map for python to read instead of python reading csv
    router(map,oldmap,cols,rows,hubs,vils);
    
    fclose(pterrain);
    pterrain = NULL; //freeing the memory
    free(map);
    map = NULL;
    free(oldmap);
    oldmap = NULL;
}