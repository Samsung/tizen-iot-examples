 /*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <glib.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libsoup/soup.h>
#include <service_app.h>
#include <app_common.h>
#include "http-server-log-private.h"
#include "http-server-route.h"

#define SIGNAL_DEBUG 0
#define HTDIGEST_FILE "/auth-data/auth-passwd.dat"

struct route_callback_data {
	http_server_route_callback callback;
	gpointer user_data;
	GDestroyNotify destroy_func;
};

static SoupServer *g_server;
static SoupAuthDomain *default_auth_domain;

#if SIGNAL_DEBUG
static void
request_aborted_cb(SoupServer *server, SoupMessage *message,
				SoupClientContext *client, gpointer user_data)
{
	_D("request-aborted : [%s]", soup_client_context_get_host(client));
}

static void
request_finished_cb(SoupServer *server, SoupMessage *message,
				SoupClientContext *client, gpointer user_data)
{
	_D("request-finished : [%s]", soup_client_context_get_host(client));
}

static void
request_read_cb(SoupServer *server, SoupMessage *message,
				SoupClientContext *client, gpointer user_data)
{
	_D("request-read : [%s]", soup_client_context_get_host(client));
}

static void
request_started_cb(SoupServer *server, SoupMessage *message,
				SoupClientContext *client, gpointer user_data)
{
	_D("request-started : [%s]", soup_client_context_get_host(client));
}
#endif /* SIGNAL_DEBUG */

static char *
digest_auth_cb(SoupAuthDomain *domain, SoupMessage *msg,
	const char *username, gpointer user_data)
{
	char *res_path = NULL;
	char *digest_path = NULL;
	GKeyFile *key_file = NULL;
	char *password = NULL;

	_D("requested user - [%s]", username);

	res_path = app_get_resource_path();
	digest_path = g_strdup_printf("%s%s", res_path, HTDIGEST_FILE);
	g_clear_pointer(&res_path, g_free);

	key_file = g_key_file_new();
	if (!g_key_file_load_from_file(key_file,
			digest_path, G_KEY_FILE_NONE, NULL)) {
		_E("failed to load htdigest file");
		return NULL;
	}
	g_clear_pointer(&digest_path, g_free);

	password = g_key_file_get_string(key_file,
					soup_auth_domain_get_realm(domain), username, NULL);

	g_key_file_unref(key_file);

	return password;
}

static int auth_domain_create(SoupServer *server)
{
	SoupAuthDomain *sad = NULL;
	retv_if(!server, -1);

	sad = soup_auth_domain_digest_new(SOUP_AUTH_DOMAIN_REALM, "default", NULL);
	retvm_if(!sad, -1, "failed to soup_auth_domain_digest_new");

	soup_auth_domain_digest_set_auth_callback(sad, digest_auth_cb, NULL, NULL);
	soup_server_add_auth_domain(server, sad);
	default_auth_domain = sad;

	return 0;
}

int http_server_create(const char *name, unsigned int port)
{
	SoupServer *s = NULL;

	retv_if(!name, -1);
	retvm_if(g_server, -1, "server is already created");

	s = soup_server_new(SOUP_SERVER_SERVER_HEADER, name,
						SOUP_SERVER_PORT, port, NULL);
	retvm_if(!s, -1, "failed to soup_server_new");

#if SIGNAL_DEBUG
	g_signal_connect(s, "request-aborted", G_CALLBACK(request_aborted_cb), NULL);
	g_signal_connect(s, "request-finished", G_CALLBACK(request_finished_cb), NULL);
	g_signal_connect(s, "request-read", G_CALLBACK(request_read_cb), NULL);
	g_signal_connect(s, "request-started", G_CALLBACK(request_started_cb), NULL);
#endif /* SIGNAL_DEBUG */

	if (auth_domain_create(s)) {
		_E("failed to auth_domain_create()");
		g_object_unref(s);
		return -1;
	}

	g_server = s;

	return 0;
}

void http_server_destroy(void)
{
	if (!g_server)
		return;

	soup_server_disconnect(g_server);

	if (default_auth_domain)
		g_object_unref(default_auth_domain);
	default_auth_domain = NULL;

	g_object_unref(g_server);
	g_server = NULL;
}

int http_server_start(void)
{
	retvm_if(!g_server, -1, "server is NOT created");

	soup_server_run_async(g_server);

	return 0;
}

int http_server_stop(void)
{
	retvm_if(!g_server, -1, "server is NOT created");

	soup_server_quit(g_server);

	return 0;
}

static void _route_callback_data_free(gpointer data)
{
	struct route_callback_data *cd = data;
	if (cd->destroy_func)
		cd->destroy_func(cd->user_data);

	g_free(cd);
}

static void
_http_server_callback(SoupServer *server, SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	struct route_callback_data *cd = user_data;

	ret_if(!cd);
	ret_if(!cd->callback);

	_D("client : %s", soup_client_context_get_host(client));
	_D("METHOD(%s) PATH(%s) URI_PATH(%s) HTTP/1.%d",
		msg->method, path, soup_message_get_uri(msg)->path,
		soup_message_get_http_version(msg));

	cd->callback(msg, path, query, client, cd->user_data);
}

int http_server_route_handler_add(const char *path, http_server_route_callback callback,
						gpointer user_data, GDestroyNotify destroy)
{
	struct route_callback_data *cd = NULL;
	retvm_if(!g_server, -1, "server is NOT created");
	retvm_if(!callback, -1, "callback is NULL");

	cd = g_try_new0(struct route_callback_data, 1);
	retvm_if(!cd, -1, "failed to alloc route_callback_data");
	cd->callback = callback;
	cd->user_data = user_data;
	cd->destroy_func = destroy;

	soup_server_add_handler(g_server, path,
			_http_server_callback, cd, _route_callback_data_free);

	return 0;
}

int http_server_route_handler_remove(const char *path)
{
	retvm_if(!g_server, -1, "server is NOT created");
	retvm_if(!path, -1, "path is NULL");

	soup_server_remove_handler(g_server, path);
	return 0;
}

int http_server_pause_message(SoupMessage *msg)
{
	retvm_if(!g_server, -1, "server is NOT created");
	retvm_if(!msg, -1, "msg is NULL");

	soup_server_pause_message(g_server, msg);
	return 0;
}

int http_server_unpause_message(SoupMessage *msg)
{
	retvm_if(!g_server, -1, "server is NOT created");
	retvm_if(!msg, -1, "msg is NULL");

	soup_server_unpause_message(g_server, msg);
	return 0;
}

int http_server_auth_default_realm_path_add(const char *path)
{
	retvm_if(!g_server, -1, "server is NOT created");
	retvm_if(!default_auth_domain, -1, "default_auth_domain is NOT created");
	retvm_if(!path, -1, "path is NULL");

	soup_auth_domain_add_path(default_auth_domain, path);

	return 0;
}

int http_server_auth_default_realm_path_remove(const char *path)
{
	retvm_if(!g_server, -1, "server is NOT created");
	retvm_if(!default_auth_domain, -1, "default_auth_domain is NOT created");
	retvm_if(!path, -1, "path is NULL");

	soup_auth_domain_remove_path(default_auth_domain, path);

	return 0;
}
