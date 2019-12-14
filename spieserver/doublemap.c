//
//  doublemap.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "doublemap.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

/*int pagesize() {
    return getpagesize();
}*/

int doublemunlock(const void *addr, size_t size) {
    return munlock(addr, 2 * size);
}

void* doublemap(size_t size) {
    // Virtual memory magic starts here.
    char  path[] = "/tmp/buffer-XXXXXX";
    int   fd;
    int   ret;
    void  *buff;
    void  *addr;

    // Create temp file
    if((fd = mkstemp(path)) < 0) {
        perror("mkstemp");
        return NULL;
    }
    // Remove link from filesystem
    if((ret = unlink(path)) < 0) {
        perror("unlink");
        return NULL;
    }
    // Set size
    if((ret = ftruncate(fd, size)) < 0) {
        perror("ftruncate");
        return NULL;
    }

    // Try to map a continuous area of memory of 2 * size.
    buff = mmap(NULL, 2 * size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(buff == MAP_FAILED) {
        perror("mmap(NULL, 2 * size)");
        return NULL;
    }

    // Then map size bytes twice.
    addr = mmap(buff, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    if (addr != buff) {
        perror("mmap(buff, size)");
        return NULL;
    }

    addr = mmap(buff + size , size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    if (addr != buff + size) {
        perror("mmap(buff + size, size)");
        return NULL;
    }

    if(close(fd) < 0) {
        perror("close");
        return NULL;
    }

    // Zero buffer
    memset(buff, 0x00, size);

    // All ok
    return buff;
}
