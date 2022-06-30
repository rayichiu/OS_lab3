#include "Process.h"
using namespace std;

int Process::InvalidRef(int vpage){
  if (page_table_[vpage].write_filemap_set == 1){
    return -2;
  }
  for(int i = 0; i < vma_vector_.size(); i++){
    if (vma_vector_[i].starting_virtual_page <= vpage 
        && vma_vector_[i].ending_virtual_page >= vpage){
      return i;
    }
  }
  return -1;
}

void Process::SetPageTable_w_f (int vma_num, int vpage){
  page_table_[vpage].write_protect = (int) vma_vector_[vma_num].write_protected;
  page_table_[vpage].filemap = (int) vma_vector_[vma_num].filemapped;
  page_table_[vpage].write_filemap_set = 1;
}