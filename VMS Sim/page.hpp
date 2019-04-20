//
//  page.hpp
//  VMS Sim
//
//  Created by Tom Metzger on 3/21/19.
//  Copyright © 2019 Tom. All rights reserved.
//

#ifndef page_hpp
#define page_hpp

#include <stdio.h>
#include <string>
#include <ctime>





using namespace std;





class Page
{
private:
	typedef struct{
		long minimum;
		long maximum;
	} range_t;
	
	
	
	
public:
	time_t last_accessed;
//	time_t average_time_between_accesses;
//	time_t next_predicted_access;

	int access_count;
	
	bool is_dirty;
	bool is_in_memory = false;
	
	long page_number;
	
//	range_t address_range;
};

#endif /* page_hpp */
