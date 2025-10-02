/**
 * @file dl.c  Interface to dynamic linking loader
 *
 * Copyright (C) 2010 Creytiv.com
 */
 
#include <re_types.h>
#include <re_fmt.h>
#include "mod_internal.h"


#define DEBUG_MODULE "dl"
#define DEBUG_LEVEL 5
#include <re_dbg.h>




/**
 * Load a dynamic library file
 *
 * @param name Name of library to load
 *
 * @return Opaque library handle, NULL if not loaded
 */
void *_mod_open(const char *name)
{
	 
		return NULL;
 
}


/**
 * Resolve address of symbol in dynamic library
 *
 * @param h      Library handle
 * @param symbol Name of symbol to resolve
 *
 * @return Address, NULL if failure
 */
void *_mod_sym(void *h, const char *symbol)
{
  
	
		return NULL;
}


/**
 * Unload a dynamic library
 *
 * @param h Library handle
 */
void _mod_close(void *h)
{
	 
}
