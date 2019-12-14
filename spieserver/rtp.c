//
//  rtp.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-12.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "rtp.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>

int create_socket() {
    int sd;
    int ret;

    char *dst_ip = "192.168.1.10";
    int dst_port = 7373;
    
    struct sockaddr_in to;
    memset(&to, 0, sizeof(struct sockaddr_in));
    to.sin_family = AF_INET;
    to.sin_port = htons(dst_port);
    ret = inet_pton(AF_INET, dst_ip, &to.sin_addr);
    assert(ret == 1);

    // Creating socket file descriptor
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sd > 0);

    ret = connect(sd, (struct sockaddr*) &to, sizeof(struct sockaddr_in));
    assert(ret == 0);
    
    return sd;
}
