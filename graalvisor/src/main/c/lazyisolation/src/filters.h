#ifndef __FILTERS_H__
#define __FILTERS_H__

enum filter_name { ALLOW_RW };

struct sock_fprog *get_filter(int filter);

#endif