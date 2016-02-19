/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2016 The Open Watcom Contributors. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  POSIX thread spinlock implementation for Linux only
*
* Author: J. Armstrong
*
****************************************************************************/

#include "variety.h"
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include "atomic.h"

#define SL_LOCKED     0
#define SL_UNLOCKED   1

_WCRTLINK int pthread_spin_init(pthread_spinlock_t *__lock, int __ignored_pshared)
{
    if(__lock == NULL) 
        return( EINVAL );
    
    __lock->value = (int *)malloc(sizeof(int));
    if(__lock->value == NULL)
        return( ENOMEM );
        
    *__lock->value = SL_UNLOCKED;
    return( 0 );
}

_WCRTLINK int pthread_spin_destroy(pthread_spinlock_t *__lock)
{
    if(__lock == NULL)
        return( EINVAL );

    if(pthread_spin_trylock(__lock) != 0)
        return( EBUSY );
    
    free(__lock->value);
    return( 0 );
}

_WCRTLINK int pthread_spin_lock(pthread_spinlock_t *__lock)
{
    if(__lock == NULL)
        return( EINVAL );

    /* spin away */
    while(pthread_spin_trylock(__lock) != 0);
    
    return( 0 );
}

_WCRTLINK int pthread_spin_trylock(pthread_spinlock_t *__lock)
{
    if(__lock == NULL)
        return( EINVAL );
        
    if(__atomic_compare_and_swap(__lock->value, SL_LOCKED, SL_UNLOCKED))
        return( 0 );
        
    return( EBUSY );
}

_WCRTLINK int pthread_spin_unlock(pthread_spinlock_t *__lock)
{
    if(__lock == NULL)
        return( EINVAL );
        
    __atomic_compare_and_swap(__lock->value, SL_UNLOCKED, SL_LOCKED);
    
    return( 0 );
}
