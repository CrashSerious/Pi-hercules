/* W32DL.H      (c) Copyright Jan Jaeger, 2004-2009                  */
/*              dlopen compat                                        */

// $Id: w32dl.h 5127 2009-01-23 13:25:01Z bernard $
//
// $Log$
// Revision 1.6  2007/06/23 00:04:19  ivan
// Update copyright notices to include current year (2007)
//
// Revision 1.5  2006/12/08 09:43:33  jj
// Add CVS message log
//

#ifndef _W32_DL_H
#define _W32_DL_H

#ifdef _WIN32

#define RTLD_NOW 0

#define dlopen(_name, _flags) \
        (void*) ((_name) ? LoadLibrary((_name)) : GetModuleHandle( NULL ) )
#define dlsym(_handle, _symbol) \
        (void*)GetProcAddress((HMODULE)(_handle), (_symbol))
#define dlclose(_handle) \
        FreeLibrary((HMODULE)(_handle))
#define dlerror() \
        ("(unknown)")
 
#endif /* _WIN32 */

#endif /* _W32_DL_H */
