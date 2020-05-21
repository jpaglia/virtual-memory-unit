/**
 * Virtual Memory Unit
 * EECS 3221 Operating System Concepts
 * Mini Project 3
 * Author: Julia Paglia
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//**************************************************************
//**************************************************************
// NOTE FOR GRADING: Change the value of PHYS_MEM_SIZE below to
// 1) 256 for NO PAGE REPLACEMENT or
// 2) 128 for PAGE REPLACEMENT
#define PHYS_MEM_SIZE 128 
//**************************************************************
//**************************************************************
#define PAGE_SIZE 256
#define PAGE_TBL_SIZE 256
#define TLB_SIZE 16
#define SHIFT_8BIT 8
#define MASK_8BIT 0xFF
#define TYPE_PAGE_TABLE "PAGE_TABLE"
#define TYPE_TLB "TLB"

struct page_entry {
    int pg_num;
    signed char pg_data[PAGE_SIZE];
};

struct node {
    struct page_entry *page;
    struct node *next;
};

//Prototypes
int has_value(int pg_number, char *type);
void insert_in_LRU(struct node **head, struct page_entry *pg);
void delete_from_LRU(struct node **head, struct page_entry *pg);
unsigned int get_phys_addr(struct page_entry *pg, int offset);
int get_LRU_head(void);
int get_frame_number(int pg_num);

// Initialize 2 Page_Entry structers for the page_table and the TLB
struct page_entry *page_table[PHYS_MEM_SIZE];  //page table size 256
struct page_entry *tlb[TLB_SIZE];

struct node *lru_head;
// int frames[PAGE_TBL_SIZE];

int main(int argc, char *argv[])
{ 
    //Initialize all necessary variables
    int a_line;
    int offset;
    int page_num;
    int log_addr;
    int virtual_addr;
    int phys_addr;
    int pg_tbl_val;         //returns value in page table
    int tlb_val;            //returns value in TLB
    signed char result_val; //result value from backing store

    int total_count = 0;
    int tlb_hit_count = 0;
    int pg_fault_count = 0;
    int next_tlb = 0;
    int first_index = 0;
    int curr_index = 0;
    int frame_counter = 0;
    int frm = 0;

    if (argc < 3) {
        printf("ERROR: WRONG NUMBER OF ARGS\nExecute in the following format:\n./<.out FILE> BACKING_STORE.bin addresses.txt\n");
        exit(0);
    }

    char *bin_file = argv[1];
    char *input_file = argv[2];
    char *output_file = "output.csv";

    FILE *in_file = fopen(input_file, "r");
    FILE *out_file = fopen(output_file, "w");
    FILE *bs_file = fopen(bin_file, "rb");

    unsigned char line[16];
    //Traverse through logical addresses in addresses.txt
    while (fgets(line, sizeof(line), in_file)) {
        total_count++;
        a_line = atoi(line);
        log_addr = a_line;
        offset = log_addr & MASK_8BIT;
        page_num = (log_addr >> 8) & MASK_8BIT;

        // Get the values in the TLB if it exists
        tlb_val = has_value(page_num, TYPE_TLB);

        // Case 1: TLB Hit -> when has_value for tlb[page_num] does not return -1
        // Case 2: TLB Miss -> when has_value for tlb[page_num] returns -1
        // Case 2a: Page Exists in page_table -> when has_value for page_table[page_num] does not return -1
        // Case 2b: Page Fault -> when has_value for page_table[page_num] returns -1

        if (tlb_val == -1) {    //TLB Miss (Case 2)
            // Get the values in the page_table if it exists
            pg_tbl_val = has_value(page_num, TYPE_PAGE_TABLE);

            if (pg_tbl_val == -1) {     //Page Fault (Case 2b)
                pg_fault_count++;
                tlb[next_tlb] = malloc(sizeof(struct page_entry));
                tlb[next_tlb]->pg_num = page_num;

                virtual_addr = log_addr - offset;
                fseek(bs_file, virtual_addr, SEEK_SET);
                fread(tlb[next_tlb]->pg_data, PAGE_SIZE, 1, bs_file);
                next_tlb = (next_tlb + 1) % 16;

                if (first_index >= PHYS_MEM_SIZE) {    //Table is full
                    curr_index = 0;
                    for (int k = 0; k < PHYS_MEM_SIZE; k++) {
                        if (page_table[k]->pg_num == lru_head->page->pg_num) {
                            curr_index = k;
                        }
                    }
                    delete_from_LRU(&lru_head, lru_head->page);
                    first_index++;
                } else {
                    curr_index = first_index;
                    first_index++;
                }

                // Add new page to page table
                page_table[curr_index] = malloc(sizeof(struct page_entry));
                page_table[curr_index]->pg_num = page_num;

                // Assign frame numbers for page_num
                // if (frames[page_num] == -1) {
                //     frames[page_num] = frame_counter;
                //     frame_counter++;
                // }

                // Add new page to the LRU
                insert_in_LRU(&lru_head, page_table[curr_index]);
                fseek(bs_file, virtual_addr, SEEK_SET);
                fread(page_table[curr_index]->pg_data, PAGE_SIZE, 1, bs_file);

                result_val = page_table[curr_index]->pg_data[offset];
                // phys_addr = ((unsigned int)frames[page_num] << SHIFT_8BIT) | offset;
                frm = get_frame_number(page_num);
                phys_addr = ((unsigned int)frm<< SHIFT_8BIT) | offset;
                fprintf(out_file, "%u,%u,%d\n", log_addr, phys_addr, result_val); 

            } else {    //Page Exists (Case 2a)
                //Adding to TLB
                tlb[next_tlb] = malloc(sizeof(struct page_entry));
                tlb[next_tlb]->pg_num = page_num;

                virtual_addr = log_addr - offset;
                fseek(bs_file, virtual_addr, SEEK_SET);
                fread(tlb[next_tlb]->pg_data, PAGE_SIZE, 1, bs_file);
                next_tlb = (next_tlb + 1) % 16;

                result_val = page_table[pg_tbl_val]->pg_data[offset];
                // phys_addr = ((unsigned int)frames[page_num] << SHIFT_8BIT) | offset;
                frm = get_frame_number(page_num);
                phys_addr = ((unsigned int)frm<< SHIFT_8BIT) | offset;
                fprintf(out_file, "%u,%u,%d\n", log_addr, phys_addr, result_val);

                // Move page to end of LRU list
                delete_from_LRU(&lru_head, page_table[pg_tbl_val]);
                insert_in_LRU(&lru_head, page_table[pg_tbl_val]);
            }
        } else {    //TLB Hit (Case 1)
            tlb_hit_count++;
            // Move page to end of LRU list
            for (int pt=0; pt < PHYS_MEM_SIZE; pt++) {
                if (page_table[pt] != NULL) {
                    if (page_table[pt]->pg_num == tlb[tlb_val]->pg_num) {
                        delete_from_LRU(&lru_head, page_table[pt]);
                        insert_in_LRU(&lru_head, page_table[pt]);
                    }
                }
            }
            result_val = tlb[tlb_val]->pg_data[offset];
            // phys_addr = ((unsigned int)frames[page_num] << SHIFT_8BIT) | offset;
            frm = get_frame_number(page_num);
            phys_addr = ((unsigned int)frm<< SHIFT_8BIT) | offset;
            fprintf(out_file, "%u,%u,%d\n", log_addr, phys_addr, result_val);
        }
    }

    // Print final statistics to stdout
    printf("Total Page Faults = %d\n", pg_fault_count);
    printf("Page Fault Rate = %0.2f%%\n", ((pg_fault_count*100.00)/total_count));
    printf("Total TLB Hits = %d\n", tlb_hit_count);
    printf("TLB Hit Rate = %0.2f%%\n", ((tlb_hit_count*100.00)/total_count));
    fclose(in_file);
    fclose(bs_file);
    fclose(out_file);
    return 0;
}

// If page_table or TLB has an entry at the index (page number), return that entry value
// If no page entry exists, return -1
int has_value(int pg_number, char *type) {
    // Checking page_table...
    if (strcmp(type, TYPE_PAGE_TABLE) == 0) {
        for (int n = 0; n < PHYS_MEM_SIZE; n++) {
            if (page_table[n] == NULL) { 
                return -1;
            } else {
                if (page_table[n]->pg_num == pg_number) { return n; }
            }
        }
        return -1;
    }
    // Checking tlb...
    else if (strcmp(type, TYPE_TLB) == 0) {
        for (int m = 0; m < TLB_SIZE; m++) {
            if (tlb[m] == NULL) {
                return -1;
            } else {
                if (tlb[m]->pg_num == pg_number) {
                    return m;
                }
            }
        }
        return -1;
    }
}

// Return the frame number of the page
int get_frame_number(int pg_num) {
    for (int f = 0; f < PHYS_MEM_SIZE; f++) {
        if (page_table[f]->pg_num == pg_num) {
            return f;
        }
    }
}

// Implementation adapted from Mini Project 2
void insert_in_LRU(struct node **head, struct page_entry *pg) {
    struct node *temp, *prev, *after;
    temp = malloc(sizeof(struct node));
    temp->page = pg;
    temp->next = NULL;
    if (*head == NULL) {
        *head = temp;
    } else {
        prev = NULL;
        after = *head;
        while(after != NULL){
            prev = after;
            after = after->next;
        }
        prev->next = temp;
    }
}

// Implementation adapted from Mini Project 2
void delete_from_LRU(struct node **head, struct page_entry *pg) {
    struct node *temp, *prev;
    temp = *head;

    if (pg->pg_num == temp->page->pg_num) {
        *head = (*head)->next;
    }
    else {
        prev = *head;
        temp = temp->next;
        while (pg->pg_num != temp->page->pg_num) {
            prev = temp;
            temp = temp->next;
        }
        prev->next = temp->next;
    }
}


