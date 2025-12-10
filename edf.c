#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct process {
    int cpuTime;
    int remainingTime;
    int period;
    int pid;
    int deadline;
    int age;
} Process;

typedef struct node {
    Process* data;
    struct node* next;
} Node; 

typedef struct queue {
    Node* head;
    int length;
} Queue;

Process* create_process(int cpu_time, int period, int pid,int deadline,int age,int remainingTime) {
    Process* new_proc = (Process*)malloc(sizeof(Process));
    if(new_proc == NULL){
        return NULL;
    }
    new_proc->cpuTime = cpu_time;
    new_proc->period = period;
    new_proc->age = age;
    new_proc->deadline = deadline;
    new_proc->pid = pid;
    new_proc->remainingTime = remainingTime;
    return new_proc;
}

Node* create_node(Process* data){
    Node* new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL){
        return NULL;
    }
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}

Queue* create_queue(){
    Queue* pq = (Queue*)malloc(sizeof(Queue));
    if(pq == NULL){
        return NULL;
    }
    pq->length = 0;
    pq->head = NULL;
    return pq;
}

void destroy_queue(Queue* pq){
    while(pq->head!=NULL){
        Node* temp = pq->head;
        pq->head = pq->head->next;
        free(temp->data); // frees the process in the node
        free(temp); // frees the node itself
    }
    free(pq); // free the rest of the queue
}

Process* peek(Queue* queue){
    return queue->head->data;
}

void print_queue(Queue* pq){
    Node* current = pq->head;
    while(current!=NULL){
        printf("(PID: %d, Deadline: %d, Age: %d)\n",current->data->pid,current->data->deadline,current->data->age);
        current = current->next;
    }
}

void push_queue(Queue* pq, Node* node){
    if(pq->head == NULL){
        pq->head = node;
        pq->head->next = NULL;
        pq->length+=1;
    } else if(node->data->deadline < pq->head->data->deadline){ // preempt
        node->next = pq->head;
        pq->head = node;
        pq->length+=1;
    } else if(node->data->deadline == pq->head->data->deadline && node->data->age > pq->head->data->age){
        node->next = pq->head;
        pq->head = node;
        pq->length+=1;
    } else if(node->data->deadline == pq->head->data->deadline && node->data->age == pq->head->data->age && node->data->pid <= pq->head->data->pid){
        node->next = pq->head;
        pq->head = node;
        pq->length+=1;
    } else {
        Node* current = pq->head;
        while(current->next != NULL && ( (current->next->data->deadline < node->data->deadline) || (current->next->data->deadline == node->data->deadline && current->next->data->age == node->data->age && current->next->data->pid < node->data->pid) || (current->next->data->deadline == node->data->deadline && current->next->data->age > node->data->age)) ) {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
        pq->length+=1;
    }
}

Process* pop_queue(Queue* pq, Node* node_to_pop){
    if(node_to_pop == pq->head){
        Node* temp = pq->head;
        pq->head = pq->head->next;
        Process* proc = temp->data;
        free(temp);
        pq->length--;
        return proc;
    } else {
        Node* current = pq->head;
        while(current->next!=node_to_pop){
            current = current->next;
        }
        Node* temp = current->next;
        Process* data = temp->data;
        current->next = current->next->next;
        free(temp);
        pq->length--;
        return data;
    }
}

// Sort ages from greatest to least
int sort_age(const void* a, const void* b) {
    Process* a1 = *((Process**)a);
    Process* b1 = *((Process**)b);
    int ageA = a1->age;
    int ageB = b1->age;
    if (ageA < ageB) return 1;
    if (ageA > ageB) return -1;
    if(ageA == ageB){
        int pidA = a1->pid;
        int pidB = b1->pid;
        if(pidA < pidB) return -1;
        if(pidA > pidB) return 1;
    }
    return 0;
}

// Sort pids for missed deadlines
int sort_pid(const void* a, const void* b){
    Process* a1 = *((Process**)a);
    Process* b1 = *((Process**)b);
    int pidA = a1->pid;
    int pidB = b1->pid;
    if (pidA < pidB) return -1;
    if (pidA > pidB) return 1;
    return 0;
}

void print_oldest_first(Queue* pq,int time){
    Process** proc_array = (Process**)malloc(pq->length*sizeof(Process*));
    Node* current = pq->head;
    int i = 0;
    while(current!=NULL){
        proc_array[i] = create_process(current->data->cpuTime,current->data->period,current->data->pid,current->data->deadline,current->data->age,current->data->remainingTime);
        current = current->next;
        i+=1;
    }
    qsort(proc_array, i, sizeof(Process*),sort_age);
    printf("%d: processes (oldest first): ",time);
    for(int j = 0; j < pq->length; j++){
        if(j == pq->length-1){
            printf("%d (%d ms)\n",proc_array[j]->pid,proc_array[j]->remainingTime);
        } else {
            printf("%d (%d ms) ",proc_array[j]->pid,proc_array[j]->remainingTime);
        }
    }
    for(int j = 0; j < pq->length; j++){
        free(proc_array[j]);
    }
    free(proc_array);
}


int gcd(int a,int b){
    if(b==0){
        return a;
    }
    return gcd(b, a % b);
}

int find_lcm(int* arr, int size){
    int answer = arr[0];
    for(int i = 1; i < size; i++){
        answer = (((arr[i] * answer)) / (gcd(arr[i],answer)));
    }
    return answer;
}

int main(){
    int num_proc = 0;
    
    printf("Enter the number of processes to schedule: ");
    scanf("%d",&num_proc);
    
    if(num_proc <= 0){
        fprintf(stderr,"Error: Cannot have a number of processes less than or equal to zero.\n");
        exit(EXIT_FAILURE);
    }
    
    int* period_arr = (int*)malloc(num_proc * sizeof(int));
    if(period_arr == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    Queue* pq = create_queue();
    if(pq == NULL){
        perror("malloc");
        free(period_arr);
        exit(EXIT_FAILURE);
    }

    Process** proc_array = (Process**)malloc(num_proc*sizeof(Process*));
    if(proc_array == NULL){
        perror("malloc");
        free(period_arr);
        destroy_queue(pq);
        exit(EXIT_FAILURE);
    }
 
    // Create initial batch of procs into queue
    for(int i = 0; i < num_proc; i++){
        int cpu_time = 0,period = 0;
        printf("Enter the CPU time of process %d: ",i+1);
        scanf("%d",&cpu_time);
        printf("Enter the period of process %d: ",i+1);
        scanf("%d",&period);
        period_arr[i] = period;
        Process* new_proc = create_process(cpu_time, period, i+1,period,0,cpu_time);
        if(new_proc == NULL){
            perror("malloc");
            free(period_arr);
            destroy_queue(pq);
            exit(EXIT_FAILURE);
        }
        // create node from process
        Node* new_node = create_node(new_proc);
        if(new_node == NULL){
            perror("malloc");
            free(period_arr);
            free(new_proc);
            destroy_queue(pq);
            exit(EXIT_FAILURE);
        }
        proc_array[i] = create_process(cpu_time,period,i+1,period,0,cpu_time); //TODO: error check proc_array
        push_queue(pq,new_node);
    }
    
    int max_time = find_lcm(period_arr,num_proc);
    free(period_arr);
    int total_proc = num_proc,sum_waiting_time = 0;
    int time = 0;
    
    // print the initial queue
    print_oldest_first(pq,time);

    while(time < max_time){
        Process* head_proc = NULL;
        if(pq->head != NULL){
            //print_queue(pq);
            head_proc = peek(pq);
        }
        
        // handling missed deadlines
        if(pq->head != NULL){
            Node* current = pq->head; 
            Process** proc_array = (Process**)malloc(pq->length*sizeof(Process*));
            int z = 0;
            while(current!=NULL){
                Node* temp = current->next;
                if(time >= current->data->deadline){
                    Process* proc_to_modify = pop_queue(pq,current);
                    proc_to_modify->deadline+=proc_to_modify->period;
                    proc_array[z] = create_process(proc_to_modify->cpuTime,proc_to_modify->period,proc_to_modify->pid,proc_to_modify->deadline,proc_to_modify->age,proc_to_modify->remainingTime);
                    Node* new_node = create_node(proc_to_modify);
                    push_queue(pq,new_node);
                    z++;
                }
                current = temp;
            }
            qsort(proc_array,z,sizeof(Process*),sort_pid);
            for(int l = 0; l < z; l++){
                printf("%d: process %d missed deadline (%d ms left), new deadline is %d\n",time,proc_array[l]->pid,proc_array[l]->remainingTime,proc_array[l]->deadline);
                free(proc_array[l]);
            }
            free(proc_array);
        }
       
        // Adding procs if need be
        int proc_added=0;
        int preempt = 0;
        for(int i = 0; i < num_proc; i++){
            if((time % proc_array[i]->period == 0) && time > 0){
                Process* new_proc = create_process(proc_array[i]->cpuTime,proc_array[i]->period,i+1,time+proc_array[i]->period,0,proc_array[i]->cpuTime);
                Node* new_node = create_node(new_proc);
                push_queue(pq,new_node);
                total_proc++;
                proc_added = 1;
                if(head_proc && pq->head->data!=head_proc){
                    preempt = 1;
                }
            }
        }
        if(proc_added) print_oldest_first(pq,time);
        
        if(pq->head != NULL){
            // If some sort of preemption occurred
            //print_queue(pq);
            if(preempt){
                printf("%d: process %d preempted!\n",time,head_proc->pid);
            }
            if(preempt && pq->head->data->remainingTime != pq->head->data->cpuTime){
                printf("%d: process %d starts\n",time,pq->head->data->pid);
            }
            if(pq->head->data->remainingTime == pq->head->data->cpuTime){
                printf("%d: process %d starts\n",time,pq->head->data->pid);
            } 
            pq->head->data->remainingTime--;
            sum_waiting_time+=(pq->length-1);
            time++;
            if(pq->head->data->remainingTime == 0){
                Process* temp = pop_queue(pq,pq->head);
                printf("%d: process %d ends\n",time,temp->pid);
                free(temp);
                if(pq->head!=NULL && pq->head->data->remainingTime != pq->head->data->cpuTime){
                    printf("%d: process %d starts\n",time,pq->head->data->pid);
                }
            }
        } else {
            time++;
        }
        Node* current = pq->head;
        while(current!=NULL){
            //Dont need to update the age of dead proc
            if(current->data->remainingTime > 0){ 
                current->data->age++;
            }
            current = current->next;
        }
    }

    for(int i = 0; i < num_proc; i++){
        free(proc_array[i]);
    }
    free(proc_array);
    destroy_queue(pq);
    
    printf("%d: Max Time reached\n",time);
    printf("Sum of all waiting times: %d\nNumber of processes created: %d\nAverage Waiting Time: %.2lf\n",sum_waiting_time,total_proc,(double)sum_waiting_time/total_proc);

    return 0;
}
