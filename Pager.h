#ifndef PAGER_H
#define PAGER_H

#include <deque>
#include <vector>

class Pager{
  public:
    virtual int select_victim_frame() = 0; 
    virtual void add_free_frame(int frame_id) = 0;
};
#endif  // PAGER_H
