/*
 * Copyright (c) 2019 Tom. All Rights Reserved.
 *
 * @TOM_LICENSE_HEADER_START@
 *
 * 1) Credit would be sick, but I really can't control what you do ¯\_(ツ)_/¯
 * 2) I'm not responsible for what you do with this AND I'm not responsible for any damage you cause ("THIS SOFTWARE IS PROVIDED AS IS", etc)
 * 3) I'm under no obligation to provide support. (But if you reach out I'll gladly take a look if I have time)
 *
 * @TOM_LICENSE_HEADER_END@
 */
/*
 * A collection of useful functions, particularly with obscure usecases
 */

#ifndef stdtom_h
#define stdtom_h

#include <stdio.h>
#include <stdbool.h>
#include <string>




#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

// just in case a more explicit name is desired
//#define log(format, ...) lprintf(format, ##__VA_ARGS__)

// TODO
//// alt name for dgblprintf
//#define dbglog(format, ...) dbglprintf(format, ##__VA_ARGS__)
//
//// for debugging with normal printf
//#define dbgprintf(format, ...) printf(__FILE__, __LINE__, format, ##__VA_ARGS__)
#pragma clang diagnostic pop


// do nothing - primarily for use to get rid of the 'variable not used' warning when warnings are treated as errors
#define NOTUSED(x) (void)(x)





using namespace std;





namespace tom
{
	// clears the console
	void printclr(void);


	// converts a string to an integer - unlike atoi & friends, it tells you if and why it fails
	int strtonum(string numstr, int minval, int maxval, string *errstrp);

	// appends s2 to s1 and returns the result
	char* cstrapp(const char* s1, const char* s2);

	// trims strings down to their real size. Also removes newline characters.
	char* cstrtrim(char* str);

	// appends char to string (one small caveat:  if the string isn't big enough this will break)
	void cstr_append_char(char* str, char character);

	// reads the contents of a file into a string
	string str_from_file_contents(string filename);

	// zeros out string - apprently there's a thing called bzero also does this?
	void zero_cstr(char** str);

	// checks if a string contains all 0's (as it won't be equal to NULL)
	bool cstr_is_0d(char* str);


#ifdef __APPLE__
	/*
		gets the path to the executable belonging the the handle given by dlopen()
		Warning: this is not a thread safe function
	 */
	string path_from_handle(void* handle);



	// retruns the path to the executable that calls it
	string path_to_current_executable(void);
#endif
	

	// set the prefix used by lprintf
	void set_lprefix(string new_prefix);

	// printf, but with a prefix and ending newling - 'l' for 'log'
	void lprintf(const char* format, ...);


	// print the defined meaning for an error value
	void printerrno(int err);
}


#endif /* stdtom_h */
