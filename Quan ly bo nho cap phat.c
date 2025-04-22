#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<stdbool.h>

//DINH NGHIA 1 NUT
//Cau truc nut quan ly khoi bo nho trong
typedef struct MemoryBlock{
    void* start_addr; // dia chi bat dau
    size_t size;   // kich thuoc khoi
    struct MemoryBlock* next;  //Tro toi khoi tiep theo
} MemoryBlock;

//KHOI TAO VUNG NHO TRONG BAN DAU
MemoryBlock* free_list = NULL; // Danh sach cac khoi trong
MemoryBlock* last_alloc = NULL; // vi tri cap phat gan nhat
//Ham khoi tao 1 vung nho ban dau

void initialize_memory(void* base_addr, size_t total_size){   //void* dung luu dia chi bat dau cua vung nho
    free_list = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    free_list->start_addr = base_addr;
    free_list->size = total_size;
    free_list->next = NULL;
}

//HAM IN RA DANH SACH CAC KHOI DANG TRONG
//In danh sach cac khoi trong
void print_free_list(){
    MemoryBlock* current = free_list;
    printf("Free Memory Blocks:\n");
    while(current!=NULL){
        printf(" Address: %p, Size: %zu\n",current->start_addr,current->size);
        current = current->next;
    }
}

//Ham cap phat bo nho (don gian - First Fit)
void* firstfit_malloc(size_t size){
    MemoryBlock* prev = NULL;
    MemoryBlock* curr = free_list;

    while(curr){
        if(curr->size >= size){
            void* addr = curr->start_addr; //luu dia chi cap phat

            if(curr->size == size){
                //Neu khoi vua du -> xoa khoi danh sach
                if(prev) prev->next = curr->next;
                else free_list = curr->next;
                free(curr);
            } else{
                //neu con du -> cap nhat lai start va size
                curr->start_addr = (char*)curr->start_addr + size;
                curr->size -= size;
            }
            return addr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL; // ko tim duoc khoi phu hop
}

//Ham cap phat bo nho (don gian - Best Fit)
//Dung nho co kich thuoc nho nhat lon hon
void* bestfit_malloc(size_t size){
    MemoryBlock *best = NULL, *best_prev = NULL;
    MemoryBlock *curr = free_list, *prev = NULL;

    while(curr!=NULL){
        if(curr->size >= size){
            //neu day la khoi nho hon khoi tot nhat truoc do
            if(!best || curr->size < best->size){
                best = curr;
                best_prev = prev;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    //neu tim thay khoi tot nhat
    if(best){
        void* addr = best->start_addr; // luu dia chi block duoc cap phat (best) vao bien addr
        
        if(best->size == size){ // khop hoan toan
            //neu best ko phai khoi dau tien trogn danh sach
            if(best_prev) best_prev->next = best->next; // cap nhat con tro next cua node trc do de bo qua best vi best da duoc cap phat
            else free_list = best->next; // cap nhta khoi de tro den node tiep theo cua best
            free(best);
        } else{ // kich thuoc lon hon yeu cau
            //doi dia chi bat dau cua block best len phia truoc 1 doan size
            best->start_addr = (char*)best->start_addr + size;
            best->size -= size; // giam kich thuoc block best
        }
        return addr;
    }
    return NULL;
}

//HAM CAP PHAT BO NHO (WORST FIT)
// Dung vung nho co kich thuoc lon nhhat de cap phat
void* worstfit_malloc(size_t size){
    MemoryBlock *worst = NULL, *worst_prev = NULL;
    MemoryBlock *curr = free_list, *prev = NULL;
    
    //duyet tim khoi co kich thuoc lon nhat
    while(curr != NULL){
        if(curr->size >= size){
            if(!worst || curr->size > worst->size){
                worst = curr;
                worst->size = curr->size; // cpa nhat khoi hien tai la khoi lon nhat
            }
        }
        prev = curr;
        curr = curr->next;
    }

    //neu tim khoi to nhat
    if(worst){
        void* addr = worst->start_addr;

        if(worst->size == size){
            // kich thuoc khop
            if(worst_prev) worst_prev->next = worst->next;
            else free_list = worst->next;
            free(worst);
        } else{
            worst->start_addr = (char*)worst->start_addr + size;
            worst->size -= size;
        }
        return addr;
    }
    return NULL;
}

//HAM CAP PHAT BO NHO NEXT FIT
//DUng vung nho trong tiep theo(tiep tuc tu vi tri cap phat trc) co kich thuoc du lon de cap phat
void* nextfit_malloc(size_t size){
    //neu bat dau tu dau danha sahc
    if(!last_alloc) last_alloc = free_list;

    MemoryBlock *curr = free_list, *prev = NULL;
    bool check = false; // kiem tra xem da duyet het 1 vong danh sach chua
    
    //vong lap tim khoi phu hop
    while(curr){
        if(curr->size >= size){
            void* addr = curr->start_addr;
            if(curr->size == size){
                if(prev) prev->next = curr->next;
                else free_list = curr->next;
                last_alloc = curr->next; // cap nhat vi tri cap phat gan nhat
                free(curr);
            } else{ // Block lon hon yeu cau thi chia nho block
                curr->start_addr = (char*)curr->start_addr + size;
                curr->size -= size;
                last_alloc = curr;
            }
            return addr;
        }
        prev = curr;
        curr = curr->next;

        //neu den cuoi dnah sach van ko tim dc
        if(!curr && !check){
            curr = free_list;
            prev = NULL;
            check = true;
        } else if(!curr) break;
    }
    return NULL;
}

//HAM CAP PHAT BO NHO (BUDDY SYSTEM)

//Tim luy thua 2 gan nhat >= size
size_t next_power_2(size_t x){
    size_t power = 1;
    while(power < x) power <<=1; // nhan doi lien tuc cho dne khi >=x
    return power;
}

void* buddysystem_malloc(size_t size){
    size_t size_needed = next_power_2(size); // tim kich thuoc can cap phat
    MemoryBlock *curr = free_list, *prev = NULL;

    while(curr){
        if(curr->size >= size_needed){
            void* addr = curr->start_addr;
            if(curr->size == size_needed){
                // neu bang dung thi lay block day ra cap phat luon
                if(prev) prev->next = curr->next;
                else free_list = curr->next;
                free(curr);
            } else{
                curr->start_addr = (char*)curr->start_addr + size_needed;
                curr->size -= size_needed;
            }

            return addr; // tra ve dia chi dau cua block cap paht
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

//Ham giai phong bo nho va hop nhat khoi lien ke
void free_mem(void* addr, size_t size){
    //Tao 1 khoi moi cho vung nho duoc giai phong
    MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    new_block->start_addr = addr;
    new_block->size = size;
    new_block->next = NULL;

    //chen khoi moi vao danh sach theo thu tu dia chi
    if(!free_list || addr < free_list->start_addr){
        //neu danh sach trong hoac dia chi new_block nho hon free_list -> chen vao dau
        new_block->next = free_list;
        free_list = new_block;
    } else{
        MemoryBlock* curr = free_list;
        while(curr->next && curr->next->start_addr < addr){
            //tim vi tri sap cho new Block dung trc khoi co dia chi lon hon no
            curr = curr->next;
        }
        new_block->next = curr->next;
        curr->next = new_block;
    }

    // Hop nhat cac khoi lien ke trong danh sach
    MemoryBlock* curr = free_list;
    while(curr && curr->next){
        char* end_curr = (char*)curr->start_addr + curr->size; // dia hi ket thuc
        if(end_curr == curr->next->start_addr){
            //neu lien ke -. gop lai
            curr->size += curr->next->size;
            MemoryBlock* tmp = curr->next;
            curr->next = tmp->next;
            free(tmp);
        } else{
            curr = curr->next;
        }
    }
}
void print_total_free_size() {
    size_t total = 0;
    MemoryBlock* curr = free_list;

    while (curr) {
        total += curr->size;        // Cộng kích thước từng khối
        curr = curr->next;
    }

    printf("→ Tổng bộ nhớ trống còn lại: %zu bytes\n", total);
}

void nhap(){
    char input[256];
    void* ptr1 = NULL;
    void* ptr2 = NULL;
    void* ptr3 = NULL;
    void* ptr4 = NULL;
    void* memory_pool = NULL;
    
    printf("=== Nhập lệnh quản lý bộ nhớ (gõ 'exit' để thoát) ===\n");

    while(1){
        printf(">> ");
        if(!fgets(input,sizeof(input),stdin)) break;
        input[strcspn(input,"\n")]=0;
        
        //Thoat neu go exit4
        if(strcmp(input,"exit")==0) break;

        //Tach lenh ra thanh tu khoa va tham so
        char* command = strtok(input," ");
        if(!command) continue;

        if(strcmp(command,"initialize_memory")==0){
            size_t size = atoi(strtok(NULL," "));
            memory_pool = malloc(size);
            initialize_memory(memory_pool,size);
        }

        else if(strcmp(command,"print_free_list")==0){
            print_free_list();
            print_total_free_size();
        }

        else if(strcmp(command,"firstfit_malloc")==0){
            size_t size = atoi(strtok(NULL," "));
            ptr1 = firstfit_malloc(size);
            printf("ptr1 allocated at %p\n",ptr1);
            print_total_free_size();
        }

        else if (strcmp(command, "bestfit_malloc") == 0) {
            size_t size = atoi(strtok(NULL, " "));
            if (!ptr2) {
                ptr2 = bestfit_malloc(size);
                printf("ptr2 allocated at %p\n", ptr2);
                print_total_free_size();
            } else {
                ptr4 = bestfit_malloc(size);
                printf("ptr4 allocated at %p\n", ptr4);
                print_total_free_size();
            }
        }

        else if (strcmp(command, "worstfit_malloc") == 0) {
            size_t size = atoi(strtok(NULL, " "));
            void* ptr = worstfit_malloc(size);
            printf("Allocated at %p\n", ptr);
            print_total_free_size();
        }

        else if (strcmp(command, "nextfit_malloc") == 0) {
            size_t size = atoi(strtok(NULL, " "));
            ptr3 = nextfit_malloc(size);
            printf("ptr3 allocated at %p\n", ptr3);
            print_total_free_size();
        }

        else if (strcmp(command, "buddysystem_malloc") == 0) {
            size_t size = atoi(strtok(NULL, " "));
            void* ptr = buddysystem_malloc(size);
            printf("Allocated (Buddy) at %p\n", ptr);
            print_total_free_size();
        }

        else if (strcmp(command, "free_mem") == 0) {
            char* which = strtok(NULL, ", ");
            size_t size = atoi(strtok(NULL, ", "));

            if (strcmp(which, "ptr1") == 0 && ptr1) {
                free_mem(ptr1, size);
                printf("Freed ptr1\n");
                print_total_free_size();
                ptr1 = NULL;
            }
            else if (strcmp(which, "ptr2") == 0 && ptr2) {
                free_mem(ptr2, size);
                printf("Freed ptr2\n");
                print_total_free_size();
                ptr2 = NULL;
            }
            else if (strcmp(which, "ptr3") == 0 && ptr3) {
                free_mem(ptr3, size);
                printf("Freed ptr3\n");
                print_total_free_size();
                ptr3 = NULL;
            }
            else if (strcmp(which, "ptr4") == 0 && ptr4) {
                free_mem(ptr4, size);
                printf("Freed ptr4\n");
                print_total_free_size();
                ptr4 = NULL;
            }
            else {
                printf("Không tìm thấy con trỏ %s\n", which);
                print_total_free_size();
            }
        }

        else {
            printf("Lệnh không hợp lệ. Các lệnh hợp lệ:\n");
            printf("  initialize_memory <size>\n");
            printf("  print_free_list\n");
            printf("  firstfit_malloc <size>\n");
            printf("  bestfit_malloc <size>\n");
            printf("  nextfit_malloc <size>\n");
            printf("  worstfit_malloc <size>\n");
            printf("  buddysystem_malloc <size>\n");
            printf("  free_mem <ptrX>, <size>\n");
        }
    }

    if (memory_pool) free(memory_pool); // Giải phóng vùng nhớ cuối chương trình
}

int main(){
/*     //Cap phat cho bo nho pool ban dau 2048 byte
    void* memory_pool = malloc(2048);
    initialize_memory(memory_pool,2048);

    print_free_list();

    //Cap phat 256 byte bang First fit
    void* ptr1 = firstfit_malloc(256);
    printf("\nAfter allocating 256 bytes (First fit): \n");
    print_free_list();

    //Cap phat 50 bytes bang Best fit
    void* ptr2 = bestfit_malloc(50);
    printf("\nAfter allocating 50 bytes (Best fit): \n");
    print_free_list();
    */
   nhap();
}
