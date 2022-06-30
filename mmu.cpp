#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <deque>
#include <climits>

#include "Pager.h"
#include "PageAlgo.h"
#include "Process.h"
#include "MMUDataStruct.h"

using namespace std;

/***********************************/
// Global Variables
/***********************************/
// input data
int total_num_frames;
string algo;
string options;
bool O_flag = false;
bool P_flag = false;
bool F_flag = false;
bool S_flag = false;
vector<int> random_number_list;

// frame
vector<frame_t> frame_table;
deque<int> free_pool;

// simulation
const int max_vpages = 64; // each process 64 vpages
int frame_id = 0;

int curr_process_num;
unsigned long inst_count = 0;
unsigned long ctx_switches = 0;
unsigned long process_exits = 0;
unsigned long read_write_num = 0;
string operation;
int vpage_operation;

// /******************************************************/
// // Operator Overload print vma_vector_ in Class Process
// /******************************************************/
// ostream& operator<<(std::ostream& out, const VMA& vma) {
//   out << "beg: " << vma.starting_virtual_page << " end: " << vma.ending_virtual_page
//       << " w: " << vma.write_protected << " f: "
//       << vma.filemapped << endl;
//   return out;
// }

/*****************************************************************/
// Operator Overload print page_table_entry in Class : Process
/*****************************************************************/
ostream& operator<<(std::ostream& out, const pte_t& pte) {
  if (pte.valid){
    // R->referenced, M->modified, S->swapped out
    string R = (pte.referenced) ? "R" : "-";
    string M = (pte.modified) ? "M" : "-";
    string S = (pte.pagedout) ? "S" : "-";
    out << ":" << R << M << S;
  }else{
    if (pte.pagedout){
      out << " #";
    }else{
      out << " *";
    }
  }
  return out;
}

// /*************************************************************/
// // Operator Overload print page_table_ in Class : Process
// /*************************************************************/
// ostream& operator<<(std::ostream& out, const pte_t& pte) {
//   out << "valid: " << pte.valid << " referenced: " << pte.referenced <<
//   " custom: " << pte.custom << endl;
//   return out;
// }

/**************************************/
// process cmd
/**************************************/
void ProcessCmd(int argc, char* argv[]){
  int o;
  while ((o = getopt(argc, argv, "f:a:o:")) != -1){
    switch (o) {
      case 'f':
        total_num_frames = stoi(optarg);
        break;
      case 'a':
        algo = optarg;
        break;
      case 'o':
        options = optarg;
        break;
      case '?':
        printf("error optopt: %c\n", optopt);
        printf("error opterr: %d\n", opterr);
        break;
      default:
        exit(EXIT_FAILURE);
        break;
    }
  }
}

/***********************************/
// Read Rfile
/***********************************/
void RandomNumberGenerator(string rfilename){
  fstream rfile;
  rfile.open(rfilename, ios::in);
  bool random_file_len = true;
  while(!rfile.eof()){
    int x;
    rfile >> x;
    if (rfile.eof()) break;
    if (random_file_len){
      random_file_len = false;
    } else{
      random_number_list.push_back(x);
    } 
  }
  rfile.close();
}

/**************************************/
// pointing options
/**************************************/
void CheckPrintOption(){
  for (auto &ch : options){
    switch (ch){
    case 'O':
      O_flag = true;
      break;
    case 'P':
      P_flag = true;
      break;
    case 'F':
      F_flag = true;
      break;
    case 'S':
      S_flag = true;
      break;
    default:
      break;
    }
  }
}

int main (int argc, char** argv) {
  setbuf(stdout, NULL);
  string filename = argv[argc - 2];
  string rfilename = argv[argc - 1];
  ProcessCmd(argc, argv);
  RandomNumberGenerator(rfilename);
  
  fstream file;
  file.open(filename, ios::in);
  string tmp_line;
  while (getline(file, tmp_line)){
    if (tmp_line[0] != '#') break;
  }
  istringstream iss(tmp_line);
  int total_process;
  iss >> total_process;

  // create process list
  vector<Process> process_list;

  for (int i = 0; i < total_process; i++){
    while (getline(file, tmp_line)){
      if (tmp_line[0] != '#') break;
    }
    istringstream iss(tmp_line);
    int total_vma;
    iss >> total_vma;
    // create vma vector for each process
    vector<VMA> vma_vector;

    for (int j = 0; j < total_vma; j++){
      getline(file, tmp_line);
      istringstream iss(tmp_line);
      
      // create vmas for each vma vector 
      int start, end, write, file;
      iss >> start >> end >> write >> file;
      VMA vma;
      vma.starting_virtual_page = start;
      vma.ending_virtual_page = end;
      vma.write_protected = (bool) write;
      vma.filemapped = (bool) file;

      vma_vector.push_back(vma);
    }
    Process pros(vma_vector, max_vpages);
    process_list.push_back(pros);
  }  

  // create frame table
  for (int i = 0; i < total_num_frames; i++){
    frame_t frame;
    frame.pid = -1;
    frame.vpage = -1;
    frame.free_frame = 1;
    frame.aging = 0;
    frame.last_used_time = 0;
    frame_table.push_back(frame);
  }

  // create free frame pool
  for (int i = 0; i < total_num_frames; i++){
    free_pool.push_back(i);
  }

  // determine pager algo
  Pager* The_Pager;
  switch (algo[0]){
    case 'f':
      The_Pager = new FIFOPager(free_pool, total_num_frames);
      break;
    case 'c':
      The_Pager = new CLOCKPager(free_pool, total_num_frames, process_list, frame_table);
      break;
    case 'e':
      The_Pager = new ESCPager(free_pool, total_num_frames, process_list, frame_table, inst_count);
      break;
    case 'r':
      The_Pager = new RANDOMPager(free_pool, total_num_frames, random_number_list);
      break;
    case 'a':
      The_Pager = new AGINGPager(free_pool, total_num_frames, process_list, frame_table);
      break;
    case 'w':
      The_Pager = new WORKINGSETPager(free_pool, total_num_frames, process_list, frame_table, inst_count);
      break;
    default:
      break;
  }

  // determine print out options
  CheckPrintOption(); 
  
  while (getline(file, tmp_line)){
    if (tmp_line[0] == '#') {
      continue;
    }  
    istringstream iss(tmp_line);
    iss >> operation >> vpage_operation;
    if (O_flag){
      cout << inst_count << ": ==> " << operation << " "
           << vpage_operation << endl;
    }
    inst_count++;

    Process* curr_process = &process_list[curr_process_num];

    // handle special case of “c” and “e” instruction
    
    /************************/
    // context switch
    /************************/    
    if (operation == "c"){
      curr_process_num = vpage_operation;
      ctx_switches ++;
      continue;
    }
    /****************/
    // exit
    /****************/
    if (operation == "e"){
      process_exits ++;
      if (O_flag){
        cout << "EXIT current process " << curr_process_num << endl;
      }

      for (int i = 0; i < max_vpages; i++){
        pte_t* pte = &curr_process->page_table_[i];       
        bool exit_pte_valid = pte->valid;
        // for each valid entry unmap the page
        if (exit_pte_valid){
          pte->valid = 0;
          curr_process->pstats.unmaps++;
          if (O_flag){
          cout << " UNMAP " << curr_process_num << ":"
               << i << endl;
          }
          // free the frame at the frame table
          // cannot direct iterate through frame table 
          // the order enqueue free_frame is not the same
          frame_t* fte = &frame_table[pte->physical_frame];
          fte->free_frame = 1;
          fte->aging = 0;
          fte->last_used_time = 0;
          fte->pid = -1;
          fte->vpage = -1;
          The_Pager->add_free_frame(pte->physical_frame);
        }      
        // FOUT modified filemapped pages
        if (pte->modified && pte->filemap && exit_pte_valid){
          pte->modified = 0;
          curr_process->pstats.fouts++;
          if (O_flag) { cout << " FOUT" << endl; } 
        }
        
        // clear pte of that process 
        pte->referenced = 0;
        pte->write_protect = 0;
        pte->pagedout = 0;
        pte->physical_frame = 0;
        pte->filemap = 0;
        pte->write_filemap_set = 0;
      }

      // remove process
      curr_process_num = -1;
      continue;
    }

    /*******************************/
    // read and write operations
    /*******************************/
    read_write_num ++;
    // get page table entry from current process
    pte_t* pte = &curr_process->page_table_[vpage_operation];

    if (!(pte->valid)){
      // SEGV
      int vma_num;
      vma_num = curr_process->InvalidRef(vpage_operation);
      if (vma_num == -1){
        if (O_flag) { cout << " SEGV" << endl; }
        curr_process->pstats.segv ++;
        continue;
      }
      // not SEGV if write_protected/ filemapped not set yet -> set it
      if (!(pte->write_filemap_set)){
        curr_process->SetPageTable_w_f(vma_num, vpage_operation);
      }

      frame_id = The_Pager->select_victim_frame();
      frame_t* frame = &frame_table[frame_id];

      // dealing with victim frame
      if (!(frame->free_frame)){
        // add unmap on the previous process
        process_list[frame->pid].pstats.unmaps++;
        if (O_flag){
          cout << " UNMAP " << frame->pid << ":"
               << frame->vpage << endl;
        }

        Process* old_process = &process_list[frame->pid];
        pte_t* old_pte = &(old_process->page_table_[frame->vpage]);
        
        if (old_pte->modified) {
          if (old_pte->filemap){
            old_process->pstats.fouts++;
            if (O_flag) { cout << " FOUT" << endl; } 
          }else{
            old_process->pstats.outs++;
            old_pte->pagedout = 1;
            if (O_flag) { cout << " OUT" << endl; }
          } 
        }
        // reset the info of victim page/ pte
        old_pte->valid = 0;
        old_pte->referenced = 0;       
        old_pte->modified = 0;
        old_pte->physical_frame = 0;
      } else{
        frame->free_frame = false;
      }

      // reset the info of curr page/ pte
      pte->valid = 1;
      pte->physical_frame = frame_id;    

      if (pte->filemap){
        curr_process->pstats.fins++;
        if (O_flag) { cout << " FIN" << endl; }
      } else if (pte->pagedout){
        curr_process->pstats.ins++;
        if (O_flag) { cout << " IN" << endl; }
      } else{
        curr_process->pstats.zeros++;
        if (O_flag) { cout << " ZERO" << endl; }
      }

      // reset the info of the selected frame      
      frame->pid = curr_process_num;
      frame->vpage = vpage_operation;

      curr_process->pstats.maps++;
      if (O_flag) { cout << " MAP " << pte->physical_frame << endl; }
    }

    // reset the info of curr page/ pte
    // referenced may be unset even if the page is valid
    pte->referenced = 1;
   
    if (operation == "w"){
      if (pte->write_protect){
        curr_process->pstats.segprot++;
        if (O_flag) { cout << " SEGPROT" << endl; }
      }else{
        //dirty page
        pte->modified = 1;
      }
    }
  }
  file.close();

  // print page table P_flag
  if (P_flag){
    for (int i = 0; i < process_list.size(); i++){
      cout << "PT[" << i << "]:";
      vector<pte_t>& page_table = process_list[i].page_table_;  
      for (int j = 0; j < page_table.size(); j++){
        pte_t* pte = &page_table[j];
        if (pte->valid){
          cout << " " << j << page_table[j];
        }else{
          cout << page_table[j];
        }
      }
      cout << endl;
    }   
  }

  // print frame table F_flag
  if (F_flag){
    cout << "FT:";
    for (int i = 0; i < total_num_frames; i++) {
      if (frame_table[i].free_frame) {
          cout << " *";
          continue;
      }
      cout << " " << frame_table[i].pid << ":" << frame_table[i].vpage;
    }
    cout << endl; 
  }

  // print statistics S_flag
  if (S_flag){
    unsigned long long cost = 0;
    for (int i = 0; i < process_list.size(); i++){
      Process* proc = &process_list[i];
      printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
                        i, proc->pstats.unmaps, proc->pstats.maps, proc->pstats.ins, proc->pstats.outs,
                        proc->pstats.fins, proc->pstats.fouts, proc->pstats.zeros, proc->pstats.segv, proc->pstats.segprot);
      cost += (proc->pstats.maps) * 300 + (proc->pstats.unmaps) * 400 +
              (proc->pstats.ins) * 3100 + (proc->pstats.outs) * 2700 +
              (proc->pstats.fins) * 2800 + (proc->pstats.fouts) * 2400 +
              (proc->pstats.zeros) * 140 + (proc->pstats.segv) * 340 +
              (proc->pstats.segprot) * 420;
    }
    cost += read_write_num * 1 + ctx_switches * 130 + process_exits * 1250;
    printf("TOTALCOST %lu %lu %lu %llu %lu\n",
        inst_count, ctx_switches, process_exits, cost, sizeof(pte_t));
        // sizeof(pte_t) -> memory size in byte : 32 bits/8 = 4 " 1 byte -> 8 bits"
  }
  
}