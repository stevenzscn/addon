#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_palloc.h>
#include <ngx_hash.h>

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r) {
    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    char *uri_key = "uri:";
    ngx_buf_t *uri_key_buf;
    uri_key_buf = ngx_create_temp_buf(r->pool, strlen(uri_key));
    if (uri_key_buf == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(uri_key_buf->pos, uri_key, strlen(uri_key));
    uri_key_buf->last = uri_key_buf->pos + strlen(uri_key);
    uri_key_buf->last_buf = 0;

    ngx_buf_t *uri_value_buf;
    uri_value_buf = ngx_create_temp_buf(r->pool, r->uri.len);
    if (uri_value_buf == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(uri_value_buf->pos, r->uri.data, r->uri.len);
    uri_value_buf->last = uri_value_buf->pos + r->uri.len;
    uri_value_buf->last_buf = 1;

    ngx_chain_t out_key, out_value;
    out_key.buf = uri_key_buf;
    out_value.buf = uri_value_buf;
    out_key.next = &out_value;
    out_value.next = NULL;

    ngx_str_t type = ngx_string("text/plain");

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = strlen(uri_key) + r->uri.len;
    r->headers_out.content_type = type;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out_key);
}

static char *
ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_mytest_handler;

    return NGX_CONF_OK;
}

static ngx_command_t ngx_http_mytest_commands[] = {
    {
        ngx_string("mytest"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        ngx_http_mytest,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    }
    ,
    ngx_null_command
};

static ngx_http_module_t ngx_http_mytest_module_ctx = {
    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL
};

ngx_module_t ngx_http_mytest_module = {
    NGX_MODULE_V1,
    &ngx_http_mytest_module_ctx,
    ngx_http_mytest_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

