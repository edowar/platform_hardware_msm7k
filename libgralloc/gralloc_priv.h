/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRALLOC_PRIV_H_
#define GRALLOC_PRIV_H_

#include <stdint.h>
#ifdef HAVE_ANDROID_OS      // just want PAGE_SIZE define
# include <asm/page.h>
#else
# include <sys/user.h>
#endif
#include <limits.h>
#include <sys/cdefs.h>
#include <hardware/gralloc.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <cutils/native_handle.h>

#include <linux/fb.h>

/*****************************************************************************/

struct private_module_t;
struct private_handle_t;

inline size_t roundUpToPageSize(size_t x) {
    return (x + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}

int mapFrameBufferLocked(struct private_module_t* module);
int terminateBuffer(gralloc_module_t const* module, private_handle_t* hnd);

/*****************************************************************************/

class Locker {
    pthread_mutex_t mutex;
public:
    class Autolock {
        Locker& locker;
    public:
        inline Autolock(Locker& locker) : locker(locker) {  locker.lock(); }
        inline ~Autolock() { locker.unlock(); }
    };
    inline Locker()        { pthread_mutex_init(&mutex, 0); }
    inline ~Locker()       { pthread_mutex_destroy(&mutex); }
    inline void lock()     { pthread_mutex_lock(&mutex); }
    inline void unlock()   { pthread_mutex_unlock(&mutex); }
};

/*****************************************************************************/

struct private_handle_t;

struct private_module_t {
    gralloc_module_t base;

    private_handle_t* framebuffer;
    uint32_t flags;
    uint32_t numBuffers;
    uint32_t bufferMask;
    pthread_mutex_t lock;
    buffer_handle_t currentBuffer;
    int pmem_master;
    void* pmem_master_base;
    int gpu;
    void* gpu_base;

    struct fb_var_screeninfo info;
    struct fb_fix_screeninfo finfo;
    float xdpi;
    float ydpi;
    float fps;
    
    enum {
        // flag to indicate we'll post this buffer
        PRIV_USAGE_LOCKED_FOR_POST = 0x80000000
    };
};

/*****************************************************************************/

struct private_handle_t : public native_handle
{
    enum {
        PRIV_FLAGS_FRAMEBUFFER = 0x00000001,
        PRIV_FLAGS_USES_PMEM   = 0x00000002,
        PRIV_FLAGS_USES_GPU    = 0x00000004,
    };

    enum {
        LOCK_STATE_WRITE     =   1<<31,
        LOCK_STATE_MAPPED    =   1<<30,
        LOCK_STATE_READ_MASK =   0x3FFFFFFF
    };

    // file-descriptors
    int     fd;
    // ints
    int     magic;
    int     flags;
    int     size;
    int     offset;
    int     gpu_fd; // stored as an int, b/c we don't want it marshalled
    
    // FIXME: the attributes below should be out-of-line
    int     base;
    int     lockState;
    int     writeOwner;
    int     pid;

    static const int sNumInts = 9;
    static const int sNumFds = 1;
    static const int sMagic = 'msm7';

    private_handle_t(int fd, int size, int flags) :
        fd(fd), magic(sMagic), flags(flags), size(size), offset(0),
        base(0), lockState(0), writeOwner(0), pid(getpid())
    {
        version = sizeof(native_handle);
        numInts = sNumInts;
        numFds = sNumFds;
    }
    ~private_handle_t() {
        magic = 0;
    }

    bool usesPhysicallyContiguousMemory() {
        return (flags & (PRIV_FLAGS_USES_PMEM|PRIV_FLAGS_USES_GPU)) != 0;
    }

    static int validate(const native_handle* h) {
        const private_handle_t* hnd = (const private_handle_t*)h;
        if (!h || h->version != sizeof(native_handle) ||
                h->numInts != sNumInts || h->numFds != sNumFds ||
                hnd->magic != sMagic) 
        {
            LOGE("invalid gralloc handle (at %p)", h);
            return -EINVAL;
        }
        return 0;
    }

    static private_handle_t* dynamicCast(const native_handle* in) {
        if (validate(in) == 0) {
            return (private_handle_t*) in;
        }
        return NULL;
    }
};

#endif /* GRALLOC_PRIV_H_ */