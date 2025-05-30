
/*
 * Copyright (C) YoungJoo Kim (vozlt)
 */


#ifndef _NGX_HTTP_STREAM_STS_DISPLAY_H_INCLUDED_
#define _NGX_HTTP_STREAM_STS_DISPLAY_H_INCLUDED_

#include "ngx_http_stream_server_traffic_status_node.h"

ngx_int_t ngx_http_stream_server_traffic_status_display_get_upstream_nelts(
    ngx_http_request_t *r);
ngx_int_t ngx_http_stream_server_traffic_status_display_get_size(
    ngx_http_request_t *r);

u_char *ngx_http_stream_server_traffic_status_display_get_time_queue(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_time_queue_t *q,
    ngx_uint_t offset);
u_char *ngx_http_stream_server_traffic_status_display_get_time_queue_times(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_time_queue_t *q);
u_char *ngx_http_stream_server_traffic_status_display_get_time_queue_msecs(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_time_queue_t *q);

u_char *ngx_http_stream_server_traffic_status_display_get_histogram_bucket(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_histogram_bucket_t *b,
    ngx_uint_t offset, const char *fmt);
u_char *ngx_http_stream_server_traffic_status_display_get_histogram_bucket_msecs(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_histogram_bucket_t *b);
u_char *ngx_http_stream_server_traffic_status_display_get_histogram_bucket_counters(
    ngx_http_request_t *r,
    ngx_http_stream_server_traffic_status_node_histogram_bucket_t *q);
#endif /* _NGX_HTTP_STREAM_STS_DISPLAY_H_INCLUDED_ */

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */
