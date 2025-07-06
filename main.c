#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLEN 100
#define MAXUPDT 256
#define ANSI_RED     "\x1b[31m"
#define ANSI_RESET   "\x1b[0m"

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
    int i=0;
    (*q).updates[(*q).q_tail].destination=u.destination;
    (*q).updates[(*q).q_tail].r.rte_pref.prefix_len=u.r.rte_pref.prefix_len;
    (*q).updates[(*q).q_tail].r.rte_pref.network_address=malloc((*q).updates[(*q).q_tail].r.rte_pref.prefix_len+1);
    strcpy((*q).updates[(*q).q_tail].r.rte_pref.network_address, u.r.rte_pref.network_address);
    (*q).updates[(*q).q_tail].r.as_pathcap=u.r.as_pathcap;
    (*q).updates[(*q).q_tail].r.as_pathcount=u.r.as_pathcount;
    (*q).updates[(*q).q_tail].r.as_path=malloc((*q).updates[(*q).q_tail].r.as_pathcap*sizeof(int));
    while(i<(*q).updates[(*q).q_tail].r.as_pathcap){
        (*q).updates[(*q).q_tail].r.as_path[i]=u.r.as_path[i];
        i++;
    }
    (*q).q_tail++;
}

int queue_is_empty(queue q){
    if(q.q_head==q.q_tail){
        return 1;
    }
    else{
        return 0;
    }
}

void dequeue(queue* q, Update* u){
    int i=0;
    (*u).destination=(*q).updates[(*q).q_head].destination;
    (*u).r.as_pathcap=(*q).updates[(*q).q_head].r.as_pathcap;
    (*u).r.as_pathcount=(*q).updates[(*q).q_head].r.as_pathcount;
    (*u).r.as_path=malloc((*u).r.as_pathcap*sizeof(int));
    while(i<(*u).r.as_pathcount){
        (*u).r.as_path[i]=(*q).updates[(*q).q_head].r.as_path[i];
        i++;
    }
    (*u).r.rte_pref.prefix_len=(*q).updates[(*q).q_head].r.rte_pref.prefix_len;
    (*u).r.rte_pref.network_address=malloc((*u).r.rte_pref.prefix_len+1);
    strcpy((*u).r.rte_pref.network_address, (*q).updates[(*q).q_head].r.rte_pref.network_address);
    (*q).q_head++;
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

int as_indexfinder(int number,AS* AS_list, int as_list_count){
    int i=0;
    while(i<as_list_count){
        if(AS_list[i].number==number){
            break;
        }
        i++;
    }
    return i;
}

void load_route(Route* from, Route* to){
    int i=0;
    (*to).as_pathcap=(*from).as_pathcap;
    (*to).as_pathcount=(*from).as_pathcount;
    (*to).rte_pref.prefix_len=(*from).rte_pref.prefix_len;
    (*to).rte_pref.network_address=malloc((*to).rte_pref.prefix_len+1);
    strcpy((*to).rte_pref.network_address, (*from).rte_pref.network_address);
    (*to).as_path=malloc((*to).as_pathcount*sizeof(int));
    while(i<(*to).as_pathcount){
        (*to).as_path[i]=(*from).as_path[i];
        i++;
    }
}

void delete_route(RIB* rib, int index){
    int i=index+1;
    //if the route is the last one in the RIB there is nothing to shift down.
    if(index+1==(*rib).rib_count){
        free((*rib).rte_list[index].rte_pref.network_address);
        (*rib).rte_list[index].rte_pref.prefix_len=0;
        (*rib).rte_list[index].as_path=realloc((*rib).rte_list[index].as_path, sizeof(int));
        (*rib).rte_list[index].as_pathcount=0;
        (*rib).rte_list[index].as_pathcap=1;
        (*rib).rib_count--;
    }
    //if not, shift everything down.
    else{
        while(i<(*rib).rib_count){
            load_route(&((*rib).rte_list[i]), &((*rib).rte_list[index]));
            index++;
            i++;
        }
    }
}

void evaluate_route(AS** AS_list, int as_list_count, int target, queue* q){
    int i=0, c=0, a=0, flag=0, b_flag=0;
    //flag=1 means that the prefix is not yet known by the AS. The loop starts under the assumption that the AS knows, if it loops through all loc_rib elements without finding the prefix, the flag is turned to 1.
    Update u;
    //LOADING IF adj_rib COUNT IS >0

    c=0;

    while(c<(*AS_list)[target].adj_rib.rib_count){
        i=0;
        flag=0;
        while(i<(*AS_list)[target].loc_rib.rib_count){
            if(strcmp((*AS_list)[target].adj_rib.rte_list[c].rte_pref.network_address, (*AS_list)[target].loc_rib.rte_list[i].rte_pref.network_address)==0){
                flag=1;
            }
            i++;
        }
        if(flag==0){
            if((*AS_list)[target].adj_rib.rte_list[c].as_pathcount==(*AS_list)[target].adj_rib.rte_list[c].as_pathcap){
                    (*AS_list)[target].adj_rib.rte_list[c].as_pathcap=(*AS_list)[target].adj_rib.rte_list[c].as_pathcap*2;
                    (*AS_list)[target].adj_rib.rte_list[c].as_path=realloc((*AS_list)[target].adj_rib.rte_list[c].as_path, (*AS_list)[target].adj_rib.rte_list[c].as_pathcap*sizeof(int));
            }
            (*AS_list)[target].adj_rib.rte_list[c].as_path[(*AS_list)[target].adj_rib.rte_list[c].as_pathcount]=(*AS_list)[target].number;
            (*AS_list)[target].adj_rib.rte_list[c].as_pathcount++;
            if((*AS_list)[target].loc_rib.rib_count==(*AS_list)[target].loc_rib.rib_cap){
                (*AS_list)[target].loc_rib.rib_cap=(*AS_list)[target].loc_rib.rib_cap*2;
                (*AS_list)[target].loc_rib.rte_list=realloc((*AS_list)[target].loc_rib.rte_list, (*AS_list)[target].loc_rib.rib_cap*sizeof(Route));
            }
            if((*AS_list)[target].out_rib.rib_count==(*AS_list)[target].out_rib.rib_cap){
                (*AS_list)[target].out_rib.rib_cap=(*AS_list)[target].out_rib.rib_cap*2;
                (*AS_list)[target].out_rib.rte_list=realloc((*AS_list)[target].out_rib.rte_list, (*AS_list)[target].out_rib.rib_cap*sizeof(Route));
            }
            load_route(&((*AS_list)[target].adj_rib.rte_list[c]), &((*AS_list)[target].loc_rib.rte_list[(*AS_list)[target].loc_rib.rib_count]));
            load_route(&((*AS_list)[target].adj_rib.rte_list[c]), &((*AS_list)[target].out_rib.rte_list[(*AS_list)[target].out_rib.rib_count]));
            (*AS_list)[target].loc_rib.rib_count++;
            (*AS_list)[target].out_rib.rib_count++;
            a=0;
            while(a<(*AS_list)[target].neighbour_count){
                u.destination=(*AS_list)[target].neighbours[a];
                load_route(&((*AS_list)[target].out_rib.rte_list[(*AS_list)[target].out_rib.rib_count-1]), &(u.r));
                enqueue(q, u);
                printf("+++Sent route (new)+++\nTO PREFIX %s\nNOW KNOWN BY AS%d\n",u.r.rte_pref.network_address, u.destination);
                a++;
            }
            delete_route(&((*AS_list)[target].adj_rib), c);
        }
        else{
            i=0;
            while(i<(*AS_list)[target].loc_rib.rib_count){
                if(strcmp((*AS_list)[target].adj_rib.rte_list[c].rte_pref.network_address, (*AS_list)[target].loc_rib.rte_list[i].rte_pref.network_address)==0 && (*AS_list)[target].adj_rib.rte_list[c].as_pathcount<(*AS_list)[target].loc_rib.rte_list[i].as_pathcount){
                    delete_route(&((*AS_list)[target].loc_rib), i);
                    if((*AS_list)[target].adj_rib.rte_list[c].as_pathcount==(*AS_list)[target].adj_rib.rte_list[c].as_pathcap){
                        (*AS_list)[target].adj_rib.rte_list[c].as_pathcap=(*AS_list)[target].adj_rib.rte_list[c].as_pathcap*2;
                        (*AS_list)[target].adj_rib.rte_list[c].as_path=realloc((*AS_list)[target].adj_rib.rte_list[c].as_path, (*AS_list)[target].adj_rib.rte_list[c].as_pathcap*sizeof(int));
                    }
                    (*AS_list)[target].adj_rib.rte_list[c].as_path[(*AS_list)[target].adj_rib.rte_list[c].as_pathcount]=(*AS_list)[target].number;
                    (*AS_list)[target].adj_rib.rte_list[c].as_pathcount++;
                    a=0;
                    while(a<(*AS_list)[target].out_rib.rib_count){
                        if(strcmp((*AS_list)[target].adj_rib.rte_list[c].rte_pref.network_address, (*AS_list)[target].out_rib.rte_list[a].rte_pref.network_address)==0){
                            delete_route(&((*AS_list)[target].out_rib), a);
                            break;
                        }
                        a++;
                    }
                    if((*AS_list)[target].loc_rib.rib_count==(*AS_list)[target].loc_rib.rib_cap){
                        (*AS_list)[target].loc_rib.rib_cap=(*AS_list)[target].loc_rib.rib_cap*2;
                        (*AS_list)[target].loc_rib.rte_list=realloc((*AS_list)[target].loc_rib.rte_list, (*AS_list)[target].loc_rib.rib_cap*sizeof(Route));
                    }
                    if((*AS_list)[target].out_rib.rib_count==(*AS_list)[target].out_rib.rib_cap){
                        (*AS_list)[target].out_rib.rib_cap=(*AS_list)[target].out_rib.rib_cap*2;
                        (*AS_list)[target].out_rib.rte_list=realloc((*AS_list)[target].out_rib.rte_list, (*AS_list)[target].out_rib.rib_cap*sizeof(Route));
                    }
                    load_route(&((*AS_list)[target].adj_rib.rte_list[c]), &((*AS_list)[target].loc_rib.rte_list[(*AS_list)[target].loc_rib.rib_count]));
                    load_route(&((*AS_list)[target].adj_rib.rte_list[c]), &((*AS_list)[target].out_rib.rte_list[(*AS_list)[target].out_rib.rib_count]));
                    (*AS_list)[target].loc_rib.rib_count++;
                    (*AS_list)[target].out_rib.rib_count++;
                    a=0;
                    while(a<(*AS_list)[target].neighbour_count){
                        u.destination=(*AS_list)[target].neighbours[a];
                        load_route(&((*AS_list)[target].out_rib.rte_list[(*AS_list)[target].out_rib.rib_count-1]), &(u.r));
                        enqueue(q, u);
                        printf("+++Sent route (better)+++\nTO PREFIX %s\nNOW KNOWN BY AS%d\n",u.r.rte_pref.network_address, u.destination);
                        a++;
                    }
                    delete_route(&((*AS_list)[target].adj_rib), c);
                    //LAST delete route from loc_rib and out_rib, first add current AS to as_path, add the route from adj_rib into them. push the out_rib content to all neighbours.
                }
                i++;
            }
        }
        c++;
    }
    //LOADING IF NOT
    if((*AS_list)[target].loc_rib.rib_count==0){
        (*AS_list)[target].adj_rib.rte_list[0].as_path[(*AS_list)[target].adj_rib.rte_list[0].as_pathcount]=(*AS_list)[target].number;
        load_route(&((*AS_list)[target].adj_rib.rte_list[0]), &((*AS_list)[target].loc_rib.rte_list[(*AS_list)[target].loc_rib.rib_count]));
        load_route(&((*AS_list)[target].adj_rib.rte_list[0]), &((*AS_list)[target].out_rib.rte_list[(*AS_list)[target].out_rib.rib_count]));
        (*AS_list)[target].loc_rib.rib_count++;
        (*AS_list)[target].out_rib.rib_count++;
        a=0;
        while(a<(*AS_list)[target].neighbour_count){
            u.destination=(*AS_list)[target].neighbours[a];
            load_route(&((*AS_list)[target].out_rib.rte_list[(*AS_list)[target].out_rib.rib_count-1]), &(u.r));
            enqueue(q, u);
            a++;
        }
        delete_route(&((*AS_list)[target].adj_rib), 0);
    }
}

int main(){
    AS* AS_list;
    int as_list_count=0, as_list_cap=1, a=0, b=0, c=0;
    queue q;
    q.updates=malloc(2*MAXLEN*sizeof(Update));
    q.q_head=0;
    q.q_tail=0;
    Update origin, u;
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
            printf("+++Sent route (origin)+++\nTO PREFIX %s\nNOW KNOWN BY AS%d\n",origin.r.rte_pref.network_address, origin.destination);
            while (getchar() != '\n');
        }
        else{
            break;
        }
    }
    while(!queue_is_empty(q)){
        dequeue(&q, &u);
        a=as_indexfinder(u.destination, AS_list, as_list_count);
        if(AS_list[a].adj_rib.rib_count==AS_list[a].adj_rib.rib_cap){
            AS_list[a].adj_rib.rib_cap=AS_list[a].adj_rib.rib_cap*2;
            AS_list[a].adj_rib.rte_list=realloc(AS_list[a].adj_rib.rte_list, AS_list[a].adj_rib.rib_cap*sizeof(Route));
        }
        load_route(&(u.r), &(AS_list[a].adj_rib.rte_list[AS_list[a].adj_rib.rib_count]));
        AS_list[a].adj_rib.rib_count++;
        evaluate_route(&(AS_list), as_list_count, a, &q);
    }
    a=0;
    while(a<as_list_count){
        b=0;
        while(b<AS_list[a].loc_rib.rib_count){
            printf("+++AS%d can reach prefix %s It will route via:+++\n",AS_list[a].number ,AS_list[a].loc_rib.rte_list[b].rte_pref.network_address);
            c=0;
            while(c<AS_list[a].loc_rib.rte_list[b].as_pathcount){
                printf("%d:AS%d\n", c, AS_list[a].loc_rib.rte_list[b].as_path[c]);
                c++;
            }
            printf( "++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
            b++;
        }
        a++;
    }
    return 0;
}
