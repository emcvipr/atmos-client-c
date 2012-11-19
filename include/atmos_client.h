/*
 * atmos_client.h
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_CLIENT_H_
#define ATMOS_CLIENT_H_

#include "atmos.h"

typedef struct {
    RestClient parent;
    char uid[ATMOS_UID_MAX];
    char secret[ATMOS_SECRET_MAX];
    int signature_debug;
    int enable_utf8_metadata;
} AtmosClient;

AtmosClient*
AtmosClient_init(AtmosClient *self, const char *host, int port, const char *uid, const char *secret);

void
AtmosClient_destroy(AtmosClient *self);

void
AtmosClient_get_service_information(AtmosClient *self, AtmosServiceInfoResponse *response);

void
AtmosClient_create_object(AtmosClient *self, AtmosCreateObjectRequest *request,
        AtmosCreateObjectResponse *response);

void
AtmosClient_create_object_simple(AtmosClient *self, const char *data,
        size_t data_size,
        const char *content_type, AtmosCreateObjectResponse *response);

void
AtmosClient_create_object_simple_ns(AtmosClient *self, const char *path,
        const char *data, size_t data_size, const char *content_type,
        AtmosCreateObjectResponse *response);

void
AtmosClient_create_object_file(AtmosClient *self, FILE *f,
        off_t content_length, const char *content_type,
        AtmosCreateObjectResponse *response);

void
AtmosClient_create_object_file_ns(AtmosClient *self, const char *path, FILE *f,
        off_t content_length, const char *content_type,
        AtmosCreateObjectResponse *response);

void
AtmosClient_delete_object(AtmosClient *self, const char *object_id,
        RestResponse *response);

void
AtmosClient_delete_object_ns(AtmosClient *self, const char *path,
        RestResponse *response);

#endif /* ATMOS_CLIENT_H_ */
