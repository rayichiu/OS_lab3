#include "PageAlgo.h"
using namespace std;

/***********/
// FIFO
/***********/
int FIFOPager::select_victim_frame(){
  int frame_id;
  // has free frame
  if (free_pool_.size() != 0){
    frame_id = free_pool_.front();
    free_pool_.pop_front();
    return frame_id;
  }
  // no free frame -> select victim frame
  frame_id = hand; 
  hand = ((hand + 1) == total_num_frames_) ? 0 : (hand + 1);
  return frame_id;     
}

void FIFOPager::add_free_frame(int frame_id){
  free_pool_.push_back(frame_id);
}

/***********/
// Clock
/***********/
int CLOCKPager::select_victim_frame(){
  int frame_id;
  // has free frame
  if (free_pool_.size() != 0){
    frame_id = free_pool_.front();
    free_pool_.pop_front();
    return frame_id;
  }

  // no free frame -> select victim frame
  int vpage = frame_table_[hand].vpage;
  int pid = frame_table_[hand].pid;
  pte_t* pte = &process_list_[pid].page_table_[vpage];
  while(pte->referenced == 1){
    pte->referenced = 0;   
    hand = ((hand + 1) == total_num_frames_) ? 0 : (hand + 1);
    vpage = frame_table_[hand].vpage;
    pid = frame_table_[hand].pid;
    pte = &process_list_[pid].page_table_[vpage];  
  } 
  frame_id = hand;
  hand = ((hand + 1) == total_num_frames_) ? 0 : (hand + 1);
  return frame_id;     
}

void CLOCKPager::add_free_frame(int frame_id){
  free_pool_.push_back(frame_id);
}

/***********/
// ESC
/***********/
int ESCPager::select_victim_frame(){
  int frame_id;
  // has free frame
  if (free_pool_.size() != 0){
    frame_id = free_pool_.front();
    free_pool_.pop_front();
    return frame_id;
  }

  // no free frame -> select victim frame
  need_reset_ref_ = false;
  if (curr_inst_count_ - last_ref_update_ >= 50){
    need_reset_ref_ = true;
    last_ref_update_ = curr_inst_count_;
  }

  int vpage = frame_table_[hand].vpage;
  int pid = frame_table_[hand].pid;
  pte_t* pte_old = &process_list_[pid].page_table_[vpage];
  int class_curr = 2*pte_old->referenced + pte_old->modified;
  int victim_frame = hand;
  for (int i = 0; i < total_num_frames_; i++){
    if (class_curr == 0) {  break; } 
    hand = ((hand + 1) == total_num_frames_) ? 0 : (hand + 1);
    vpage = frame_table_[hand].vpage;
    pid = frame_table_[hand].pid;
    pte_t* pte_new = &process_list_[pid].page_table_[vpage];
    int class_new = 2*pte_new->referenced + pte_new->modified;
    if (class_new < class_curr){
      victim_frame = hand;
      class_curr = class_new;
    }  
  }

  // when clock >= 50 reset "referenced" bit
  if(need_reset_ref_){
    for (int i = 0; i < total_num_frames_; i++){
      vpage = frame_table_[i].vpage;
      pid = frame_table_[i].pid;
      pte_t* pte = &process_list_[pid].page_table_[vpage];
      pte->referenced = 0;
    }
  }

  frame_id = victim_frame;
  hand = ((victim_frame + 1) == total_num_frames_) ? 0 : (victim_frame + 1);
  return frame_id;    
}

void ESCPager::add_free_frame(int frame_id){
  free_pool_.push_back(frame_id);
}

/***********/
// RANDOM
/***********/
int RANDOMPager::select_victim_frame(){
  int frame_id;
  // has free frame
  if (free_pool_.size() != 0){
    frame_id = free_pool_.front();
    free_pool_.pop_front();
    return frame_id;
  }

  // no free frame -> select victim frame
  int random_num = random_number_list_[ofs_] % total_num_frames_;
  ofs_++;
  if (ofs_ == random_number_list_.size()) { ofs_ = 0; }
  return random_num;     
}

void RANDOMPager::add_free_frame(int frame_id){
  free_pool_.push_back(frame_id);
}

/***********/
// AGING
/***********/
int AGINGPager::select_victim_frame(){
  int frame_id;
  // has free frame
  if (free_pool_.size() != 0){
    frame_id = free_pool_.front();
    free_pool_.pop_front();
    return frame_id;
  }

  // no free frame -> select victim frame
  // assume the victim frame's frame_id = hand  
  int victim_frame_num = hand;
  int vpage = frame_table_[hand].vpage;
  int pid = frame_table_[hand].pid;
  pte_t* pte = &process_list_[pid].page_table_[vpage];

  // assume the victim frame's aging = min_age 
  unsigned long min_age = frame_table_[hand].aging >> 1;
  if (pte->referenced == 1) {
      min_age = (min_age | 0x80000000);
  }
  
  for(int i = 0; i < total_num_frames_; i++){     
    int vpage_new = frame_table_[hand].vpage;
    int pid_new = frame_table_[hand].pid;
    pte_t* pte_new = &process_list_[pid_new].page_table_[vpage_new];
    frame_t* fte_new = &(frame_table_[hand]);
    // 1. left shift 1 bit 
    fte_new->aging = fte_new->aging >> 1;

    // 2. append the new R bit at the left side
    if (pte_new->referenced == 1) {
        fte_new->aging = (fte_new->aging | 0x80000000);
    }
    pte_new->referenced = 0;

    // 3. choose min aging as victim
    if (fte_new->aging < min_age) {
        min_age = fte_new->aging;
        victim_frame_num = hand;
    }
    hand = ((hand + 1) == total_num_frames_) ? 0 : (hand + 1);
  }
  // 4. reset min aging frame
  frame_table_[victim_frame_num].aging = 0;

  hand = ((victim_frame_num + 1) == total_num_frames_) ? 0 : (victim_frame_num + 1);
  return victim_frame_num; 
}

void AGINGPager::add_free_frame(int frame_id){
  free_pool_.push_back(frame_id);
}

/****************/
// WORKINGSET
/****************/
int WORKINGSETPager::select_victim_frame(){
  int frame_id;
  // has free frame
  if (free_pool_.size() != 0){
    frame_id = free_pool_.front();
    free_pool_.pop_front();
    return frame_id;
  }

  // no free frame -> select victim frame
  // assume the victim frame's frame_id = hand  
  int victim_frame_num = hand;
  // min time is not the only criteria for paging out 
  // cannot assume the victim frame's last use time = min time
  // If sample victim frame R == 1 (smaller last use time) 
  // and the other page (larger last use time) R==0 && age < 50
  // that other page should be paged out.  
  unsigned long min_time = ULONG_MAX;
  
  for(int i = 0; i < total_num_frames_; i++){     
    int vpage_new = frame_table_[hand].vpage;
    int pid_new = frame_table_[hand].pid;
    pte_t* pte_new = &process_list_[pid_new].page_table_[vpage_new];
    frame_t* fte_new = &(frame_table_[hand]);
    // 1. find age
    unsigned long age = curr_inst_count_ - (fte_new->last_used_time);

    // 2. case 1: -> R == 1, set time of last use to curr time
    if (pte_new->referenced == 1) {
        pte_new->referenced = 0;
        fte_new->last_used_time = curr_inst_count_;
    } else{
      if(age >= 50){
        // 2. case 2: -> R == 0 && age > tau remove this page
        victim_frame_num = hand;
        hand = ((hand + 1) == total_num_frames_) ? 0 : (hand + 1);
        return victim_frame_num;
      }else{
        // 2. case 3: -> R == 0 && age <= tau set smallest time
        if (fte_new->last_used_time < min_time){
          victim_frame_num = hand;
          min_time = fte_new->last_used_time;         
        }       
      }
    }
    hand = ((hand + 1) == total_num_frames_) ? 0 : (hand + 1);
  }

  hand = ((victim_frame_num + 1) == total_num_frames_) ? 0 : (victim_frame_num + 1);
  return victim_frame_num; 
}

void WORKINGSETPager::add_free_frame(int frame_id){
  free_pool_.push_back(frame_id);
}
