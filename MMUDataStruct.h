#ifndef MMUDataStruct_H
#define MMUDataStruct_H

/********************/
// global frame table
/********************/
struct frame_t {
  int vpage;
  int pid;
  bool free_frame;

  // instead of construct aging vector put aging here
  // since only active pages can have an age
  unsigned int aging:32;

  // WORKINGSET 
  unsigned long last_used_time;
};

/*****************/
// VMA
/*****************/
struct VMA {
    int starting_virtual_page;
    int ending_virtual_page;
    bool write_protected;
    bool filemapped;
};

/********************/
// page table
/********************/
struct pte_t {
  // 5 bits for pte identity 
  // unsigned -> non negative number
  unsigned int valid:1;
  unsigned int referenced:1;
  unsigned int modified:1;
  unsigned int write_protect:1;
  unsigned int pagedout:1;

  // reserve for frame
  unsigned int physical_frame:7;

  // 20 bits for your own usage
  unsigned int filemap:1;
  unsigned int write_filemap_set:1; // already paged fault
  unsigned int custom:18;
};

#endif  // MMUDataStruct_H