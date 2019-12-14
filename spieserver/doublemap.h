//
//  doublemap.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef doublemap_h
#define doublemap_h

#include <stdio.h>

int doublemunlock(const void *addr, size_t size);
void* doublemap(size_t size);

#endif /* doublemap_h */
