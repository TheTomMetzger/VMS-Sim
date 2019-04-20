//
//  vmm.cpp
//  VMS Sim
//
//  Created by Tom Metzger on 3/21/19.
//  Copyright © 2019 Tom. All rights reserved.
//

#include "vmm.hpp"

#include <iostream>
#include <unistd.h>
#include <regex>
#include <ctime>
#include <limits.h>
#include <fstream>

#include "stdtom/stdtom.hpp"




#define PAGE_SIZE 4096





VirtualMemoryManager::VirtualMemoryManager(policy_t policy, int number_of_pages)
{
	this->policy = policy;
	
	
	switch (policy)
	{
		case FIFO:
		{
			this->choose_and_replace_page = std::bind(&VirtualMemoryManager::replace_fifo, this, std::placeholders::_1);
			break;
		}
			
		case LFU:
		{
			this->choose_and_replace_page = std::bind(&VirtualMemoryManager::replace_lfu, this, std::placeholders::_1);
			break;
		}
			
		case LRU:
		{
			this->choose_and_replace_page = std::bind(&VirtualMemoryManager::replace_lru, this, std::placeholders::_1);
			break;
		}
			
		default:
		{
			this->choose_and_replace_page = std::bind(&VirtualMemoryManager::replace_generic, this, std::placeholders::_1);
			break;
		}
	}
	
	
	this->page_limit = number_of_pages;
	
	
	this->pagefault_count = 0;
	
	
	this->memory_is_full = false;
	
#ifdef DEMO
	this->fifo_memory_is_full = false;
	this->lfu_memory_is_full = false;
#endif
	
	
	this->error_count = 0;
	
	
#ifdef DEMO
	// create separate memories for both policy
	this->fifo_pagefault_count = 0;
	this->lfu_pagefault_count = 0;
#endif
}




string policytostr(VirtualMemoryManager::policy_t policy)
{
	if (policy == VirtualMemoryManager::LRU)
	{
		return "LRU";
	}
	else if (policy == VirtualMemoryManager::FIFO)
	{
		return "FIFO";
	}
	else if (policy == VirtualMemoryManager::OPT)
	{
		return "OPT";
	}
	else if (policy == VirtualMemoryManager::RANDOM)
	{
		return "RANDOM";
	}
	else if (policy == VirtualMemoryManager::CLOCK)
	{
		return "CLOCK";
	}
	else if (policy == VirtualMemoryManager::MRU)
	{
		return "MRU";
	}
	else if (policy == VirtualMemoryManager::LFU)
	{
		return "LFU";
	}
	else if (policy == VirtualMemoryManager::MFU)
	{
		return "MFU";
	}
	else
	{
		// default to FIFO?
		return "FIFO";
	}
}




void VirtualMemoryManager::print_results()
{
#ifdef VERBOSE
	printf("\n\n\n");
	// because of tom::lprintf's behavior, we must print explicitly two different statements based on the presence of errors
	if (error_count == 0)
	{
		tom::lprintf("- RESULTS OF SIMULATION -\nPolicy: %s\nPage Faults: %d\nMemory Size: %d\n\n\n", policytostr(policy).c_str(), pagefault_count, page_limit);
	}
	else
	{
		tom::lprintf("- RESULTS OF SIMULATION -\nPolicy: %s\nPage Faults: %d\nMemory Size: %d\nError Count: %d\n\n\n", policytostr(policy).c_str(), pagefault_count, page_limit, error_count);
	}
#else
#ifdef DEMO
	printf("FIFO %d: %lld\n", page_limit, fifo_pagefault_count);
	printf("LFU %d: %lld\n", page_limit, lfu_pagefault_count);
#else
	printf("%s %d: %lld\n", policytostr(policy).c_str(), page_limit, pagefault_count);
#endif
	
	if (error_count > 0)
	{
		printf("Error Count: %d\n", error_count);
	}
#endif
}




void VirtualMemoryManager::print_results_to_file()
{
	ofstream output_file;
	
	
#ifdef DEMO
	output_file.open("./PRISM_OUTPUT.txt");
	
	output_file << "FIFO " << page_limit << ": " << fifo_pagefault_count << "\n";
	output_file << "LFU " << page_limit << ": " << lfu_pagefault_count << "\n";
	
	if (error_count > 0)
	{
		output_file << "Error Count: " << error_count << "\n";
	}
#else
	output_file.open("./vms_sim_results.txt");
#ifdef VERBOSE
	output_file << "- RESULTS OF SIMULATION -\nPolicy: " << policytostr(policy) << "\nPage Faults: " << pagefault_count << "\nMemory Size: " << page_limit;
	
	if (error_count != 0)
	{
		output_file << "\nError Count: " << error_count;
	}

	output_file << "\n\n\n";
#else
	output_file << policytostr(policy) << " " << page_limit << ": " << pagefault_count << "\n";
	
	if (error_count > 0)
	{
		output_file << "Error Count: " << error_count << "\n";
	}
#endif
#endif
	
	
	output_file.close();
}




void VirtualMemoryManager::replace_generic(long base_page_address)
{
	printf("[vmm] Generic Page Replace called.\n");
	printf("[vmm] All this does is keep your program from exploding.\n");
}




void VirtualMemoryManager::replace_fifo(long page_address)
{
#ifdef DEMO
	fifo_memory.erase(fifo_memory_queue.front().page_number);
	fifo_memory_queue.pop();
	
	
	Page new_page{};// = new Page();
	
	new_page.page_number = (page_address >> 12);
	new_page.is_in_memory = true;
	
	fifo_memory.insert(pair<long, Page>(new_page.page_number, new_page));
	fifo_memory_queue.push(new_page);
#else
#ifdef VERBOSE
	tom::lprintf("REPLACING FIFO - EVICTING: %d", memory_queue.front().page_number);
#endif
	memory.erase(memory_queue.front().page_number);
	memory_queue.pop();
	
	
	Page new_page{};// = new Page();
	
	new_page.page_number = (page_address >> 12);
	new_page.is_in_memory = true;
	
	memory.insert(pair<long, Page>(new_page.page_number, new_page));
	memory_queue.push(new_page);
#endif
}




// Not yet complete - implementation only halfway there
void VirtualMemoryManager::replace_lru(long page_address)
{
	time_t current_oldest = 0;
	int index_of_current_oldest = 0;
	long page_number_of_evicted = 0;
	
	int i = 0;
	for (auto &page : memory)
	{
		// check if oldest has been set first
		if (page.second.last_accessed < current_oldest)
		{
			current_oldest = page.second.last_accessed;
			page_number_of_evicted = page.second.page_number;
			index_of_current_oldest = i;
		}
		
		i++;
	}
	
	memory.erase(page_number_of_evicted);
	
	
#ifdef VERBOSE
	tom::lprintf("REPLACING LRU - EVICTING: %d", page_number_of_evicted);
#endif
	
	
	Page new_page{};// = Page();
	
	new_page.page_number = (page_address >> 12);
	new_page.access_count = 1;
	new_page.is_in_memory = true;
	
	memory[new_page.page_number] = new_page;
}





void VirtualMemoryManager::replace_lfu(long page_address)
{
#ifdef DEMO
	int current_lowest_access_count = 0;
	int index_of_current_lowest_access_count = 0;
	long page_number_of_evicted = 0;
	
	int i = 0;
	for (auto &page : lfu_memory)
	{
		if (current_lowest_access_count == 0)
		{
			current_lowest_access_count = page.second.access_count;
			page_number_of_evicted = page.second.page_number;
			index_of_current_lowest_access_count = i;
		}
		if (page.second.access_count < current_lowest_access_count)
		{
			current_lowest_access_count = page.second.access_count;
			page_number_of_evicted = page.second.page_number;
			index_of_current_lowest_access_count = i;
		}
		
		i++;
	}
	
	lfu_memory.erase(page_number_of_evicted);
	
	
	//	tom::lprintf("REPLACING LFU - EVICTING: %d", page_number_of_evicted);
	
	
	Page new_page{};// = Page();
	
	new_page.page_number = (page_address >> 12);
	new_page.access_count = 1;
	new_page.is_in_memory = true;
	
	lfu_memory[new_page.page_number] = new_page;
#else
	int current_lowest_access_count = 0;
	int index_of_current_lowest_access_count = 0;
	long page_number_of_evicted = 0;
	
	
	int i = 0;
	for (auto &page : memory)
	{
		if (current_lowest_access_count == 0)
		{
			current_lowest_access_count = page.second.access_count;
			page_number_of_evicted = page.second.page_number;
			index_of_current_lowest_access_count = i;
		}
		if (page.second.access_count < current_lowest_access_count)
		{
			current_lowest_access_count = page.second.access_count;
			page_number_of_evicted = page.second.page_number;
			index_of_current_lowest_access_count = i;
		}
		
		i++;
	}
	
	memory.erase(page_number_of_evicted);
	
	
#ifdef VERBOSE
	tom::lprintf("REPLACING LFU - EVICTING: %d", page_number_of_evicted);
#endif
	
	
	Page new_page{};// = Page();
	
	new_page.page_number = (page_address >> 12);
	new_page.access_count = 1;
	new_page.is_in_memory = true;
	
	memory[new_page.page_number] = new_page;
#endif
}




#ifdef DEMO
void VirtualMemoryManager::fifo_page_fault(long address)
{
	//	tom::lprintf("PAGE FAULT");
	fifo_pagefault_count++;
	
	
	if (fifo_memory.size() < page_limit)
	{
		// add new page to memory
		Page new_page{};// = new Page();
		
		new_page.page_number = (address >> 12);
		new_page.is_in_memory = true;
		
		
		fifo_memory.insert(pair<long, Page>(new_page.page_number, new_page)); // this may explode?
		fifo_memory_queue.push(new_page);
	}
	else
	{
		replace_fifo(address);
	}
}


void VirtualMemoryManager::lfu_page_fault(long address)
{
	//	tom::lprintf("PAGE FAULT");
	lfu_pagefault_count++;
	
	
	if (lfu_memory.size() < page_limit)
	{
		// add new page to memory
		Page new_page = Page();
		
		new_page.page_number = (address >> 12);
		new_page.is_in_memory = true;
		
		
		lfu_memory[new_page.page_number] = new_page;
	}
	else
	{
		replace_lfu(address);
	}
}
#endif




void VirtualMemoryManager::page_fault(long address)
{
#ifdef VERBOSE
	tom::lprintf("PAGE FAULT");
#endif
	
	
	pagefault_count++;


	if (memory.size() < page_limit)
	{
#ifdef VERBOSE
		tom::lprintf("INSERTING NEW PAGE");
#endif
		// add new page to memory
		Page new_page{};// = Page();
		
		new_page.page_number = (address >> 12);
		new_page.access_count = 1;
		new_page.is_in_memory = true;
		
		memory[new_page.page_number] = new_page;
	}
	else
	{
		choose_and_replace_page(address);
	}
}




#ifdef DEMO
Page* VirtualMemoryManager::fifo_page_for_address(long address)
{
	if (fifo_memory.size() == 0)
	{
		return NULL;
	}
	
	
	try
	{
//		printf("trying\n");
		if (fifo_memory[(address >> 12)].is_in_memory)
		{
			return &fifo_memory[address >> 12];
		}
		else
		{
			fifo_memory.erase(address >> 12); // because [] inserts if not found
			return NULL;
		} // inserts page rather than accessing it
	}
	catch (...)
	{
//		printf("NULL\n");
		return NULL;
	}
	
	
	
	return NULL; //still want to explicitly return NULL;
}


Page* VirtualMemoryManager::lfu_page_for_address(long address)
{
	if (lfu_memory.size() == 0)
	{
		return NULL;
	}
	
	
	try
	{
		if (lfu_memory[(address >> 12)].is_in_memory)
		{
			return &lfu_memory[address >> 12];
		}
		else
		{
			lfu_memory.erase(address >> 12); // because [] inserts if not found
			return NULL;
		}
	}
	catch (...)
	{
		return NULL;
	}
	
	
	
	return NULL; //still want to explicitly return NULL;
}
#endif


// might want to modify to return the index of the page in memory (this will be helpful for setting the dirty bit without searching again)
Page* VirtualMemoryManager::page_for_address(long address)
{
	if (memory.size() == 0)
	{
		return NULL;
	}
	
	
	try
	{
		if (memory[(address >> 12)].is_in_memory)
		{
			return &memory[address >> 12];
		}
		else
		{
			memory.erase(address >> 12); // because [] inserts if not found
			return NULL;
		}
	}
	catch (...)
	{
		return NULL;
	}
	
	
	
	return NULL; //still want to explicitly return NULL;
}



// normalized to here
void VirtualMemoryManager::execute_memory(long address, long size)
{
#ifdef DEMO
	Page* fifo_accessed_page = fifo_page_for_address(address);
	
	if (fifo_accessed_page != NULL)
	{
		
		if (fifo_accessed_page->page_number == ((address + size) >> 12))
		{
			//			tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			fifo_accessed_page->access_count++;
			fifo_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (fifo_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				fifo_page_fault((address + size));
			}
		}
	}
	else
	{
		fifo_page_fault(address);
		
		if (fifo_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			fifo_page_fault((address + size));
		}
	}
	
	
	
	Page* lfu_accessed_page = lfu_page_for_address(address);
	
	if (lfu_accessed_page != NULL)
	{
		
		if (lfu_accessed_page->page_number == ((address + size) >> 12))
		{
			//			tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			lfu_accessed_page->access_count++;
			lfu_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (lfu_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				lfu_page_fault((address + size));
			}
		}
	}
	else
	{
		lfu_page_fault(address);
		
		if (lfu_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			lfu_page_fault((address + size));
		}
	}
#else
#ifdef VERBOSE
	tom::lprintf("Executing 0x%X", address);
#endif
	// no higher level checks needed because this already checks if memory is empty, and page_fault already checks if memory is full
	Page* accessed_page = page_for_address(address);
	
	if (accessed_page != NULL)
	{
		
		if (accessed_page->page_number == ((address + size) >> 12))
		{
#ifdef VERBOSE
			tom::lprintf("HIT");
#endif
			accessed_page->access_count++;
			accessed_page->last_accessed = time(0);
		}
		else
		{
			if (page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				page_fault((address + size));
			}
		}
	}
	else
	{
		page_fault(address);
		
		if (page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			page_fault((address + size));
		}
	}
#endif
}




void VirtualMemoryManager::store_memory(long address, long size)
{
#ifdef DEMO
	Page* fifo_accessed_page = fifo_page_for_address(address);
	
	if (fifo_accessed_page != NULL)
	{
		
		if (fifo_accessed_page->page_number == ((address + size) >> 12))
		{
			//			tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			fifo_accessed_page->is_dirty = true;
			fifo_accessed_page->access_count++;
			fifo_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (fifo_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				fifo_page_fault((address + size));
			}
		}
	}
	else
	{
		fifo_page_fault(address);
		
		if (fifo_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			fifo_page_fault((address + size));
		}
	}
	
	
	Page* lfu_accessed_page = lfu_page_for_address(address);
	
	if (lfu_accessed_page != NULL)
	{
		
		if (lfu_accessed_page->page_number == ((address + size) >> 12))
		{
			//			tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			lfu_accessed_page->is_dirty = true;
			lfu_accessed_page->access_count++;
			lfu_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (lfu_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				lfu_page_fault((address + size));
			}
		}
	}
	else
	{
		lfu_page_fault(address);
		
		if (lfu_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			lfu_page_fault((address + size));
		}
	}
#else
#ifdef VERBOSE
	tom::lprintf("Storing 0x%X", address);
#endif
	Page* accessed_page = page_for_address(address);
	
	if (accessed_page != NULL)
	{
		
		if (accessed_page->page_number == ((address + size) >> 12))
		{
#ifdef VERBOSE
			tom::lprintf("HIT");
#endif
			accessed_page->is_dirty = true;
			accessed_page->access_count++;
			accessed_page->last_accessed = time(0);
		}
		else
		{
			if (page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				page_fault((address + size));
			}
		}
	}
	else
	{
		page_fault(address);
		
		if (page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			page_fault((address + size));
		}
	}
#endif
}




void VirtualMemoryManager::load_memory(long address, long size)
{
#ifdef DEMO
	Page* fifo_accessed_page = fifo_page_for_address(address);
	
	if (fifo_accessed_page != NULL)
	{
		if (fifo_accessed_page->page_number == ((address + size) >> 12))
		{
			//			tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			fifo_accessed_page->access_count++;
			fifo_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (fifo_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				fifo_page_fault((address + size));
			}
		}
	}
	else
	{
		fifo_page_fault(address);
		
		if (fifo_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			fifo_page_fault((address + size));
		}
	}
	
	Page* lfu_accessed_page = lfu_page_for_address(address);
	
	if (lfu_accessed_page != NULL)
	{
		if (lfu_accessed_page->page_number == ((address + size) >> 12))
		{
			//			tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			lfu_accessed_page->access_count++;
			lfu_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (lfu_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				lfu_page_fault((address + size));
			}
		}
	}
	else
	{
		lfu_page_fault(address);
		
		if (lfu_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			lfu_page_fault((address + size));
		}
	}
#else
#ifdef VERBOSE
	tom::lprintf("Loading 0x%X", address);
#endif
	Page* accessed_page = page_for_address(address);
	
	if (accessed_page != NULL)
	{
		if (accessed_page->page_number == ((address + size) >> 12))
		{
#ifdef VERBOSE
			tom::lprintf("HIT");
#endif
			accessed_page->access_count++;
			accessed_page->last_accessed = time(0);
		}
		else
		{
			if (page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				page_fault((address + size));
			}
		}
	}
	else
	{
		page_fault(address);
		
		if (page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			page_fault((address + size));
		}
	}
#endif
}




void VirtualMemoryManager::modify_memory(long address, long size)
{
	//dont forget to make it dirty
	
#ifdef DEMO
	Page* fifo_accessed_page = fifo_page_for_address(address);
	
	if (fifo_accessed_page != NULL)
	{
		
		if (fifo_accessed_page->page_number == ((address + size) >> 12))
		{
			//tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			fifo_accessed_page->is_dirty = true;
			fifo_accessed_page->access_count++;
			fifo_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (fifo_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				fifo_page_fault((address + size));
			}
		}
	}
	else
	{
		fifo_page_fault(address);
		
		if (fifo_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			fifo_page_fault((address + size));
		}
	}
	
	
	Page* lfu_accessed_page = lfu_page_for_address(address);
	
	if (lfu_accessed_page != NULL)
	{
		
		if (lfu_accessed_page->page_number == ((address + size) >> 12))
		{
			//tom::lprintf("HIT");
			//			printf("%ld hit\n", accessed_page->page_number);
			output_line_num++;
			lfu_accessed_page->is_dirty = true;
			lfu_accessed_page->access_count++;
			lfu_accessed_page->last_accessed = time(0);
		}
		else
		{
			if (lfu_page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				lfu_page_fault((address + size));
			}
		}
	}
	else
	{
		lfu_page_fault(address);
		
		if (lfu_page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			lfu_page_fault((address + size));
		}
	}
#else
#ifdef VERBOSE
	tom::lprintf("Modifying 0x%X", address);
#endif
	Page* accessed_page = page_for_address(address);
	
	if (accessed_page != NULL)
	{
		
		if (accessed_page->page_number == ((address + size) >> 12))
		{
#ifdef VERBOSE
			tom::lprintf("HIT");
#endif
			accessed_page->is_dirty = true;
			accessed_page->access_count++;
			accessed_page->last_accessed = time(0);
		}
		else
		{
			if (page_for_address((address + size)) != NULL)
			{
				//				printf("%ld hit\n", accessed_page->page_number);
			}
			else
			{
				page_fault((address + size));
			}
		}
	}
	else
	{
		page_fault(address);
		
		if (page_for_address((address + size)) != NULL)
		{
			// do nothing
		}
		else
		{
			page_fault((address + size));
		}
	}
#endif
}




VirtualMemoryManager::instruction_t VirtualMemoryManager::parse_instruction(string raw_instruction)
{
	instruction_t instruction;
	
	
	while (raw_instruction[0] == ' ') //remove *all* leading spaces
	{
		raw_instruction.erase(raw_instruction.begin());
	}
	
	instruction.command = raw_instruction[0];
	
	
	const char* remaining = raw_instruction.c_str();
	remaining++;
	remaining++;
	
	string search = remaining;
	size_t found = search.find(", ");
	
	if (found != std::string::npos)
	{
		//end of address is found - 1
		string address_str = search.substr(0, (found));
		
		while (address_str[0] == ' ') //remove *all* leading spaces
		{
			address_str.erase(address_str.begin());
		}

		long address;
		int check = sscanf(address_str.c_str(), "%lx", &address);
		
		instruction.address = address;
		
		if (check != 1)
		{
			tom::lprintf("ERROR: Could not get address '%s'", address_str.c_str());
			error_count++;
			
			
			return {0,0,0};
		}
		
		
		string size_str = search.substr((found + 2));
		
		while (size_str[0] == ' ') //remove *all* leading spaces
		{
			size_str.erase(size_str.begin());
		}
	
		string error = "";
		instruction.size = tom::strtonum(size_str, 0, INT_MAX, &error);
		
		if (error != "")
		{
			tom::lprintf("ERROR: Could not get size '%s': %s", size_str.c_str(), error.c_str());
			error_count++;
			
			
			return {0,0,0};
		}
	}
	else // There's probably a better way to go about this, but oh well
	{
		string search = remaining;
		size_t found = search.find(",");
		
		if (found != std::string::npos)
		{
			//end of address is found - 1
			string address_str = search.substr(0, (found));
			
			while (address_str[0] == ' ') //remove *all* leading spaces
			{
				address_str.erase(address_str.begin());
			}
			
			long address;
			int check = sscanf(address_str.c_str(), "%lx", &address);
			
			instruction.address = address;
			
			if (check != 1)
			{
				tom::lprintf("ERROR: Could not get address '%s'", address_str.c_str());
				error_count++;
				
				
				return {0,0,0};
			}
			
			string size_str = search.substr((found + 1));
			
			while (size_str[0] == ' ') //remove *all* leading spaces
			{
				size_str.erase(size_str.begin());
			}

			string error = "";
			instruction.size = tom::strtonum(size_str, 0, INT_MAX, &error);
			
			if (error != "")
			{
				tom::lprintf("ERROR: Could not get size '%s': %s", size_str.c_str(), error.c_str());
				error_count++;
				
				
				return {0,0,0};
			}
		}
		else
		{
			tom::lprintf("ERROR: Invalid Instruction Format: '%s'.", raw_instruction.c_str());
			error_count++;
		
		
			return {0,0,0};
		}
	}
	
	
	return instruction;
}




void VirtualMemoryManager::handle_instruction(string raw_instruction)
{
	VirtualMemoryManager::instruction_t instruction = parse_instruction(raw_instruction);
	instruction.size = instruction.size - 1;
	
	switch (instruction.command)
	{
		case 'I':
		{
			execute_memory(instruction.address, instruction.size);
			break;
		}
			
		case 'S':
		{
			store_memory(instruction.address, instruction.size);
			break;
		}
			
		case 'L':
		{
			load_memory(instruction.address, instruction.size);
			break;
		}
			
		case 'M':
		{
			modify_memory(instruction.address, instruction.size);
			break;
		}
			
		default:
		{
			tom::lprintf("ERROR: Invalid command '%c' in instruciton '%s'.", instruction.command, raw_instruction.c_str());
			error_count++;
			break;
		}
	}
}




void VirtualMemoryManager::run_simulation()
{
#ifdef __XCODE__
	// c concatenates adjacent strings - looks a little gross, but easier than making a variable and modifying it to accomplish the same thing
	if (access(PROJECT_DIR"/VMS Sim/tests/cat.val.out", F_OK) == 0) // No need to check for PROJECT_DIR definition - if we're in Xcode, it should be defined (though still manually)
	{
		if (freopen(PROJECT_DIR"/VMS Sim/tests/cat.val.out", "r", stdin) == NULL)
		{
			printf("ERROR!\n");
			
			
			return;
		}
	}
	else
	{
		printf("ERROR: %s\n", strerror(errno));
		
		
		return;
	}
#endif
	
	
#ifndef VERBOSE // Verbose mode is mostly for debugging, so it's ok if it's a touch slower
	// make stdin/out faster by a litte
	cin.tie(NULL);
	ios_base::sync_with_stdio(false);
#endif
	
	
#ifdef VERBOSE
	tom::lprintf("- SIMULATION BEGIN -\n");
#endif
	
	
#ifdef DEMO
	long long linenum = 1;
#endif
	for (string line; getline(cin, line);)
	{
		handle_instruction(line);
		
#ifdef DEMO
		linenum++;
		if (linenum % 50000000 == 0)
		{
			double percent_complete = (static_cast<double>(linenum) / 5813305348 ) * 100;
			tom::lprintf("%f%% Complete!", percent_complete);
		}
#endif
	}
	
#ifdef DEMO
	tom::lprintf("100%% Complete!");
#endif
	
#ifdef VERBOSE
	printf("\n");
	tom::lprintf("- SIMULATION END -");
#endif
	
	
	print_results();
	
	print_results_to_file();
}
