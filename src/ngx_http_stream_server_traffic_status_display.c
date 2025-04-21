
/*
 * Copyright (C) YoungJoo Kim (vozlt)
 */


#include <ngx_config.h>

#include "ngx_http_stream_server_traffic_status_module.h"
#include "ngx_http_stream_server_traffic_status_shm.h"
#include "ngx_http_stream_server_traffic_status_display.h"




ngx_int_t
ngx_http_stream_server_traffic_status_display_get_upstream_nelts(ngx_http_request_t *r) 
{
    ngx_uint_t                                    i, j, n;
    ngx_stream_upstream_server_t                 *us;
#if (NGX_STREAM_UPSTREAM_ZONE)
    ngx_stream_upstream_rr_peer_t                *peer;
    ngx_stream_upstream_rr_peers_t               *peers;
#endif
    ngx_stream_upstream_srv_conf_t               *uscf, **uscfp;
    ngx_stream_upstream_main_conf_t              *umcf;
    ngx_http_stream_server_traffic_status_ctx_t  *ctx;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_stream_server_traffic_status_module);
    umcf = ctx->upstream;
    uscfp = umcf->upstreams.elts;

    for (i = 0, j = 0, n = 0; i < umcf->upstreams.nelts; i++) {

        uscf = uscfp[i];

        /* groups */
        if (uscf->servers && !uscf->port) {
            us = uscf->servers->elts;

#if (NGX_HTTP_UPSTREAM_ZONE)
            if (uscf->shm_zone == NULL) {
                goto not_supported;
            }   

            peers = uscf->peer.data;

            ngx_http_upstream_rr_peers_rlock(peers);

            for (peer = peers->peer; peer; peer = peer->next) {
                n++;
            }   

            ngx_http_upstream_rr_peers_unlock(peers);

not_supported:

#endif

            for (j = 0; j < uscf->servers->nelts; j++) {
                n += us[j].naddrs;
            }
        }   
    }   

    return n;
}


ngx_int_t
ngx_http_stream_server_traffic_status_display_get_size(ngx_http_request_t *r)
{
    ngx_uint_t                                         size;
    ngx_slab_pool_t                                   *shpool;
    ngx_http_stream_server_traffic_status_loc_conf_t  *stscf;
    ngx_http_stream_server_traffic_status_shm_info_t  *shm_info;

    stscf = ngx_http_get_module_loc_conf(r, ngx_http_stream_server_traffic_status_module);
    shpool = (ngx_slab_pool_t *) stscf->shm_zone->shm.addr;

    shm_info = ngx_pcalloc(r->pool, sizeof(ngx_http_stream_server_traffic_status_shm_info_t));
    if (shm_info == NULL) {
        return NGX_ERROR;
    }

    /* Caveat: Do not use duplicate ngx_shmtx_lock() before this function. */
    ngx_shmtx_lock(&shpool->mutex);

    ngx_http_stream_server_traffic_status_shm_info(r, shm_info);

    ngx_shmtx_unlock(&shpool->mutex);

    /* allocate memory for the upstream groups even if upstream node not exists */
    size = 0;

    if (size <= 0) {
        size = shm_info->max_size;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "sts::display_get_size(): size[%ui] used_size[%ui], used_node[%ui]",
                   size, shm_info->used_size, shm_info->used_node);

    return size;
}


u_char *
ngx_http_stream_server_traffic_status_display_get_time_queue(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_time_queue_t *q,
    ngx_uint_t offset
    )
{
    u_char     *p, *s;
    ngx_int_t   i;

    if (q->front == q->rear) {
        return (u_char *) "";
    }

    p = ngx_pcalloc(r->pool, q->len * NGX_INT_T_LEN);

    s = p;

    for (i = q->front; i != q->rear; i = (i + 1) % q->len) {
        s = ngx_sprintf(s, "%M,", *((ngx_msec_t *) ((char *) &(q->times[i]) + offset)));
    }

    if (s > p) {
       *(s - 1) = '\0';
    }

    return p;
}


u_char *
ngx_http_stream_server_traffic_status_display_get_time_queue_times(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_time_queue_t *q)
{
    return ngx_http_stream_server_traffic_status_display_get_time_queue(r, q,
               offsetof(ngx_http_stream_server_traffic_status_node_time_t, time));
}


u_char *
ngx_http_stream_server_traffic_status_display_get_time_queue_msecs(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_time_queue_t *q)
{
    return ngx_http_stream_server_traffic_status_display_get_time_queue(r, q,
               offsetof(ngx_http_stream_server_traffic_status_node_time_t, msec));
}


u_char *
ngx_http_stream_server_traffic_status_display_get_histogram_bucket(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_histogram_bucket_t *b,
    ngx_uint_t offset,
    const char *fmt)
{
    char        *dst;
    u_char      *p, *s;
    ngx_uint_t   i, n;

    n = b->len;

    if (n == 0) {
        return (u_char *) "";
    }

    p = ngx_pcalloc(r->pool, n * NGX_INT_T_LEN);
    if (p == NULL) {
        return (u_char *) "";
    }

    s = p;

    for (i = 0; i < n; i++) {
        dst = (char *) &(b->buckets[i]) + offset;

        if (ngx_strncmp(fmt, "%M", 2) == 0) {
            s = ngx_sprintf(s, fmt, *((ngx_msec_t *) dst));

        } else if (ngx_strncmp(fmt, "%uA", 3) == 0) {
            s = ngx_sprintf(s, fmt, *((ngx_atomic_uint_t *) dst));
        }
    }

    if (s > p) {
       *(s - 1) = '\0';
    }

    return p;
}


u_char *
ngx_http_stream_server_traffic_status_display_get_histogram_bucket_msecs(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_histogram_bucket_t *b)
{
    return ngx_http_stream_server_traffic_status_display_get_histogram_bucket(r, b,
               offsetof(ngx_http_stream_server_traffic_status_node_histogram_t, msec), "%M,");
}


u_char *
ngx_http_stream_server_traffic_status_display_get_histogram_bucket_counters(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_histogram_bucket_t *b)
{
    return ngx_http_stream_server_traffic_status_display_get_histogram_bucket(r, b,
               offsetof(ngx_http_stream_server_traffic_status_node_histogram_t, counter), "%uA,");
}
/* vi:set ft=c ts=4 sw=4 et fdm=marker: */
