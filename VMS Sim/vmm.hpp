//
//  vmm.hpp
//  VMS Sim
//
//  Created by Tom Metzger on 3/21/19.
//  Copyright © 2019 Tom. All rights reserved.
//

#ifndef vmm_hpp
#define vmm_hpp


#include <unordered_map>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>

#include "page.hpp"

//#define __XCODE__
//#define DEMO





using namespace std;





class VirtualMemoryManager
{
private:
	typedef struct
	{
		char command;
		long address;
		long size;
	}instruction_t;
	
	
	
	
	int error_count;
	
	
	int page_limit;
	
	
	bool memory_is_full;
	
#ifdef DEMO
	bool fifo_memory_is_full;
	
	bool lfu_memory_is_full;
#endif
	
	
//	int lfu_page_index;
	
	
	
	
	void print_results(void);
	
	void print_results_to_file(void);
	
	
	// we want our page replacer to be agnostic of policy
	function<void(long)> choose_and_replace_page;
	
	// does nothing
	void replace_generic(long address);
	
	// Replaces the oldest page
	void replace_fifo(long address);
	
	// Replaces the Least Recently Used Page
	void replace_lru(long address);
	
	// Replaces the Least Frequently Used Page
	void replace_lfu(long address);

	
	void page_fault(long address);
	
#ifdef DEMO
	void fifo_page_fault(long address);
	void lfu_page_fault(long address);
#endif
	
	
	Page* page_for_address(long address);
	
#ifdef DEMO
	Page* fifo_page_for_address(long address);
	Page* lfu_page_for_address(long address);
#endif
	
	
	void execute_memory(long address, long size);
	
	void store_memory(long address, long size);
	
	void load_memory(long address, long size);
	
	void modify_memory(long address, long size);
	
	VirtualMemoryManager::instruction_t parse_instruction(string raw_instruction);
	
	void handle_instruction(string raw_instruction);
	
	
	
	
public:
	typedef enum {
		LRU,
		FIFO,
		OPT,
		RANDOM,
		CLOCK,
		MRU,
		LFU,
		MFU
	} policy_t;
	
	policy_t policy;
	
	
	long long pagefault_count;
	
	
	unordered_map<long, Page> memory;
	queue<Page> memory_queue;
	
#ifdef DEMO
	unordered_map<long, Page> fifo_memory;
	queue<Page> fifo_memory_queue;
	long long fifo_pagefault_count;
	
	unordered_map<long, Page> lfu_memory;
	long long lfu_pagefault_count;
#endif
	
	
	
	
	VirtualMemoryManager(policy_t policy, int number_of_pages);
	
	
	void run_simulation(void);
};

#endif /* vmm_hpp */
