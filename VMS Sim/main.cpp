//
//  main.cpp
//  VMS Sim
//
//  Created by Tom Metzger on 3/21/19.
//  Copyright © 2019 Tom. All rights reserved.
//

#include <iostream>

#include "stdtom/stdtom.hpp"
#include <string.h>

#include "vmm.hpp"
//#define __XCODE__
//#define DEMO
#include <unistd.h>
#include <fstream>





void begin_simulation(VirtualMemoryManager::policy_t policy, int number_of_pages)
{
	VirtualMemoryManager *vmm = new VirtualMemoryManager(policy, number_of_pages);
	
	
	vmm->run_simulation();
}




VirtualMemoryManager::policy_t strtopolicy(string policy_str)
{
	if (policy_str == "LRU")
	{
		return VirtualMemoryManager::LRU;
	}
	else if (policy_str == "FIFO")
	{
		return VirtualMemoryManager::FIFO;
	}
	else if (policy_str == "OPT")
	{
		return VirtualMemoryManager::OPT;
	}
	else if (policy_str == "RANDOM")
	{
		return VirtualMemoryManager::RANDOM;
	}
	else if (policy_str == "CLOCK")
	{
		return VirtualMemoryManager::CLOCK;
	}
	else if (policy_str == "MRU")
	{
		return VirtualMemoryManager::MRU;
	}
	else if (policy_str == "LFU")
	{
		return VirtualMemoryManager::LFU;
	}
	else if (policy_str == "MFU")
	{
		return VirtualMemoryManager::MFU;
	}
	else
	{
		// default to FIFO?
		return VirtualMemoryManager::FIFO;
	}
}




void init(int argc, const char *argv[], string* policy, int* num_pages)
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--num-pages") == 0)
		{
			if ((i + 1) < argc)
			{
				string *error = NULL;
				*num_pages = tom::strtonum(argv[i+1], 0, __INT_MAX__, error);
				
				if (error != NULL)
				{
					tom::lprintf("ERROR: %s", error);
				}
			}
		}
		else if (strcmp(argv[i], "--policy") == 0)
		{
			if ((i + 1) < argc)
			{
				*policy = argv[i+1];
			}
		}
	}
	
	
	if (*policy == "")
	{
		tom::lprintf("ERROR: Page Replacement Policy not specified.");
		
		
		exit(EXIT_FAILURE);
	}
	
	if (*num_pages == -1)
	{
		tom::lprintf("ERROR: Number of Pages not specified.");
		
		
		exit(EXIT_FAILURE);
	}
}




int main(int argc, const char *argv[])
{
#ifdef DEMO

#endif
	
	
	string page_replacement_policy = "";
	int number_of_pages = -1;
	
	
	tom::set_lprefix("[vmss] ");


	if (argc == 5)
	{
		init(argc, argv, &page_replacement_policy, &number_of_pages);

//		tom::lprintf("Policy: %s", page_replacement_policy.c_str());
//		tom::lprintf("Number of Pages: %d", number_of_pages);

		begin_simulation(strtopolicy(page_replacement_policy), number_of_pages);
	}
	else
	{
		tom::lprintf("ERROR: Invalid Arguments.\n   Usage: vmss -policy <policy> -num-pages <number>");
		return -1;
	}
	
//	VirtualMemoryManager::policy_t junk = strtopolicy("FIFO");
	
//	printf("%d", junk);
	
	
	return 0;
}
