#ifndef PAGEALGO_H
#define PAGEALGO_H

#include <deque>
#include <vector>
#include <iostream>
#include <climits>

#include "Pager.h"
#include "MMUDataStruct.h"
#include "Process.h"

using namespace std;

/***********/
// FIFO
/***********/
class FIFOPager : public Pager{
  public:
    FIFOPager(deque<int>& free_pool, int total_num_frames)
        : free_pool_(free_pool),
          total_num_frames_(total_num_frames){};

    int select_victim_frame(); 
    void add_free_frame(int frame_id);

  private:
    int hand = 0;
    int total_num_frames_;
    deque<int>& free_pool_;  
};

/***********/
// Clock
/***********/
class CLOCKPager : public Pager{
  public:
    CLOCKPager(deque<int>& free_pool, int total_num_frames, 
               vector<Process>& process_list,
               vector<frame_t>& frame_table)
        : free_pool_(free_pool),
          process_list_(process_list),
          frame_table_(frame_table),
          total_num_frames_(total_num_frames){};

    int select_victim_frame(); 
    void add_free_frame(int frame_id);

  private:
    int hand = 0;
    int total_num_frames_;
    deque<int>& free_pool_;  
    vector<Process>& process_list_;
    vector<frame_t>& frame_table_;
};

/***********/
// ESC
/***********/
class ESCPager : public Pager{
  public:
    ESCPager(deque<int>& free_pool, int total_num_frames, 
               vector<Process>& process_list,
               vector<frame_t>& frame_table,
               unsigned long& curr_inst_count)
        : free_pool_(free_pool),
          process_list_(process_list),
          frame_table_(frame_table),
          curr_inst_count_(curr_inst_count),
          total_num_frames_(total_num_frames){};

    int select_victim_frame(); 
    void add_free_frame(int frame_id);

  private:
    unsigned long& curr_inst_count_;
    int total_num_frames_;
    deque<int>& free_pool_;  
    vector<Process>& process_list_;
    vector<frame_t>& frame_table_;

    unsigned long last_ref_update_ = 0;
    int hand = 0;
    bool need_reset_ref_ = false;
};

/***********/
// RANDOM
/***********/
class RANDOMPager : public Pager{
  public:
    RANDOMPager(deque<int>& free_pool, int total_num_frames, 
               vector<int>& random_number_list)
        : free_pool_(free_pool),
          random_number_list_(random_number_list),
          total_num_frames_(total_num_frames){};

    int select_victim_frame(); 
    void add_free_frame(int frame_id);

  private:
    int total_num_frames_;
    deque<int>& free_pool_;  
    vector<int>& random_number_list_;  
    int ofs_ = 0;
};

/***********/
// AGING
/***********/
class AGINGPager : public Pager{
  public:
    AGINGPager(deque<int>& free_pool, int total_num_frames, 
               vector<Process>& process_list,
               vector<frame_t>& frame_table)
        : free_pool_(free_pool),
          process_list_(process_list),
          frame_table_(frame_table),
          total_num_frames_(total_num_frames){};

    int select_victim_frame(); 
    void add_free_frame(int frame_id);

  private:
    int hand = 0;
    int total_num_frames_;
    deque<int>& free_pool_;  
    vector<Process>& process_list_;
    vector<frame_t>& frame_table_;
};

/****************/
// WORKINGSET
/****************/
class WORKINGSETPager : public Pager{
  public:
    WORKINGSETPager(deque<int>& free_pool, int total_num_frames, 
               vector<Process>& process_list,
               vector<frame_t>& frame_table,
               unsigned long& curr_inst_count)
        : free_pool_(free_pool),
          process_list_(process_list),
          frame_table_(frame_table),
          curr_inst_count_(curr_inst_count),
          total_num_frames_(total_num_frames){};

    int select_victim_frame(); 
    void add_free_frame(int frame_id);

  private:
    unsigned long& curr_inst_count_;
    int total_num_frames_;
    deque<int>& free_pool_;  
    vector<Process>& process_list_;
    vector<frame_t>& frame_table_;

    int hand = 0;
};


#endif //PAGEALGO_H