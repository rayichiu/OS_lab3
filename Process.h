#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>
#include <vector>

#include "MMUDataStruct.h"

using namespace std;
/************************************************/
// statistics per process one statistics table
/************************************************/
struct STATISTICS{
  // summary statistics
  unsigned long unmaps = 0;
  unsigned long maps = 0;
  unsigned long ins = 0;
  unsigned long outs = 0;
  unsigned long fins = 0;
  unsigned long fouts = 0;
  unsigned long zeros = 0;
  unsigned long segv = 0;
  unsigned long segprot = 0;
};

class Process {
  public:
    vector<pte_t> page_table_; // a per process array of fixed size=64 of pte_t
    const vector<VMA> vma_vector_; 
    STATISTICS pstats; 

    Process(const vector<VMA>& vma_vector, int max_vpages)
        : vma_vector_(vma_vector),
          page_table_(max_vpages){};
    
    // check SEGV 
    // return vma number "-1" means SEGV
    // return vma number "-2" means write_protected/ filemap already set
    // vma number "-1" && vma number "-2" don't need to SetPageTable_w_f
    int InvalidRef(int vpage);

    // set write filemap instruction when vma_num != -1 and vma_num != -2
    void SetPageTable_w_f (int vma_num, int vpage);

};

#endif  // PROCESS_H