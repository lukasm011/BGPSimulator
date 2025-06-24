#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLEN 100
#define MAXUPDT 256

typedef struct{
    char* network_address;
    int prefix_len;
} prefix;

typedef struct{
    prefix rte_pref;
    int* as_path;
    int as_pathcount;
    int as_pathcap;
} Route;

typedef struct{
    Route* rte_list;
    int rib_count;
    int rib_cap;
} RIB;

typedef struct{
    int number;
    int* neighbours;
    int neighbour_count;
    int neighbour_cap;
    RIB adj_rib;
    RIB loc_rib;
    RIB out_rib;
} AS;

typedef struct{
    int destination;
    Route r;
} Update;

typedef struct{
    Update* updates;
    int q_head;
    int q_tail;
} queue;

void add_as(AS** AS_list, int* as_list_count, int* as_list_cap){
    int destination;
    char flush;
    if(*as_list_count==*as_list_cap){
        *as_list_cap=*as_list_cap*2;
        *AS_list=realloc(*AS_list, *as_list_cap*sizeof(AS));
    }
    (*AS_list)[*as_list_count].neighbour_cap=1;
    (*AS_list)[*as_list_count].neighbours=malloc((*AS_list)[*as_list_count].neighbour_cap*sizeof(int));
    (*AS_list)[*as_list_count].neighbour_count=0;
    printf("Enter number of [%d]: AS", *as_list_count);
    scanf("%d", &((*AS_list)[*as_list_count].number));
    while (getchar() != '\n');
    while(1){
        printf("Enter destination of connection [%d]: AS", (*AS_list)[*as_list_count].neighbour_count);
        scanf("%d", &destination);
        if(destination==0){
            break;
        }
        else{
            if((*AS_list)[*as_list_count].neighbour_count==(*AS_list)[*as_list_count].neighbour_cap){
                (*AS_list)[*as_list_count].neighbour_cap=(*AS_list)[*as_list_count].neighbour_cap*2;
                (*AS_list)[*as_list_count].neighbours=realloc((*AS_list)[*as_list_count].neighbours, (*AS_list)[*as_list_count].neighbour_cap*sizeof(int));
            }
            (*AS_list)[*as_list_count].neighbours[(*AS_list)[*as_list_count].neighbour_count]=destination;
            (*AS_list)[*as_list_count].neighbour_count++;
        }
    }
    (*AS_list)[*as_list_count].adj_rib.rib_cap=1;
    (*AS_list)[*as_list_count].adj_rib.rib_count=0;
    (*AS_list)[*as_list_count].adj_rib.rte_list=malloc((*AS_list)[*as_list_count].adj_rib.rib_cap*sizeof(Route));
    (*AS_list)[*as_list_count].loc_rib.rib_cap=1;
    (*AS_list)[*as_list_count].loc_rib.rib_count=0;
    (*AS_list)[*as_list_count].loc_rib.rte_list=malloc((*AS_list)[*as_list_count].loc_rib.rib_cap*sizeof(Route));
    (*AS_list)[*as_list_count].out_rib.rib_cap=1;
    (*AS_list)[*as_list_count].out_rib.rib_count=0;
    (*AS_list)[*as_list_count].out_rib.rte_list=malloc((*AS_list)[*as_list_count].out_rib.rib_cap*sizeof(Route));
    (*as_list_count)++;
    while (getchar() != '\n');
}

void enqueue(queue* q, Update u){
    (*q).updates[(*q).q_tail].destination=u.destination;
    (*q).updates[(*q).q_tail].r.rte_pref.prefix_len=u.r.rte_pref.prefix_len;
    (*q).updates[(*q).q_tail].r.rte_pref.network_address=malloc((*q).updates[(*q).q_tail].r.rte_pref.prefix_len+1);
    strcpy((*q).updates[(*q).q_tail].r.rte_pref.network_address, u.r.rte_pref.network_address);
    (*q).updates[(*q).q_tail].r.as_pathcap=u.r.as_pathcap;
    (*q).updates[(*q).q_tail].r.as_pathcount=u.r.as_pathcount;
    (*q).updates[(*q).q_tail].r.as_path=realloc(u.r.as_path, (*q).updates[(*q).q_tail].r.as_pathcap*sizeof(int));
    (*q).q_tail++;
}

void make_prefix(prefix *new_prefix){
    char prefixinput[17];
    int cidr, i=0, a;
    printf("Enter prefix: ");
    scanf("%16s", prefixinput);
    cidr=prefixinput[strlen(prefixinput)-1]-48;
    a=(prefixinput[strlen(prefixinput)-2]-48);
    cidr=cidr+10*a;
    cidr=cidr/8;
    while(cidr){
        if(prefixinput[i]=='.'){
            cidr--;
        }
        i++;
    }
    (*new_prefix).prefix_len=i;
    (*new_prefix).network_address=malloc(i+1);
    strcpy((*new_prefix).network_address, prefixinput);
    (*new_prefix).network_address[i]='\0';
};

int main(){
    AS* AS_list;
    int as_list_count=0, as_list_cap=1;
    queue q;
    q.updates=malloc(2*MAXLEN*sizeof(Update));
    q.q_head=0;
    q.q_tail=0;
    Update origin;
    origin.r.as_pathcap=1;
    origin.r.as_pathcount=0;
    origin.r.as_path=malloc(origin.r.as_pathcap*sizeof(int));
    AS_list=malloc(as_list_cap*sizeof(AS));
    while(1){
        printf("===STATUS UPDATE===\nlist_count:%d\nlist_cap:%d\n", as_list_count, as_list_cap);
        if(getchar()=='a'){
            add_as(&AS_list, &as_list_count, &as_list_cap);
            origin.destination=AS_list[as_list_count-1].number;
            make_prefix(&(origin.r.rte_pref));
            enqueue(&q, origin);
            while (getchar() != '\n');
        }
        else{
            break;
        }
    }
    printf("%s", q.updates[2].r.rte_pref.network_address);
    return 0;
}
