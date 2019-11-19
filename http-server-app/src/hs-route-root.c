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
#include <gio/gio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libsoup/soup.h>
#include <app_common.h>
#include "http-server-log-private.h"
#include "http-server-route.h"

static int serve_file(const char *file, SoupMessage *msg, guint *status_code)
{
	GMappedFile *map_file = NULL;
	char *content_type = NULL;

	map_file = g_mapped_file_new(file, FALSE, NULL);
	if (!map_file) {
		if (status_code)
			*status_code = SOUP_STATUS_INTERNAL_SERVER_ERROR;
		return -1;
	}

	soup_message_body_append(msg->response_body,
		SOUP_MEMORY_COPY,
		g_mapped_file_get_contents(map_file),
		g_mapped_file_get_length(map_file));

	g_clear_pointer(&map_file, g_mapped_file_unref);

	content_type = g_content_type_guess(file, NULL, 0, NULL);
	if (content_type) {
		char *mime_type = g_content_type_get_mime_type(content_type);
		if (mime_type) {
			soup_message_headers_set_content_type(
					msg->response_headers, mime_type, NULL);
			g_free(mime_type);
		}
		g_clear_pointer(&content_type, g_free);
	}

	return 0;
}

static void
get_static_contents(SoupMessage *msg, const char *path)
{
	struct stat st;
	guint status_code = SOUP_STATUS_OK;

	if (stat(path, &st) == -1) {
		_E("invalid path[%s]", path);
		// DO NOT use code 403
		soup_message_set_status(msg, SOUP_STATUS_NOT_FOUND);
		return;
	}

	if (S_ISDIR(st.st_mode)) {
		if (!soup_message_get_uri(msg)->path
			|| (0 == strncmp("/", soup_message_get_uri(msg)->path, 2))) {
			char *index_path = g_strdup_printf("%sindex.html", path);
			serve_file(index_path, msg, &status_code);
			g_free(index_path);
		} else {
			// DO NOT use code 403
			_E("dir[%s] is not for serve", path);
			status_code = SOUP_STATUS_NOT_FOUND;
		}
	} else {
		serve_file(path, msg, &status_code);
	}
	soup_message_set_status(msg, status_code);
}

static void route_root_callback(SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	char *file_path = NULL;
	char *res_path = NULL;

	if (msg->method != SOUP_METHOD_GET) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}
	res_path = app_get_resource_path();
	file_path = g_strdup_printf("%spublic%s", res_path, path ? path : "/");
	g_clear_pointer(&res_path, g_free);

	get_static_contents(msg, file_path);
	g_free(file_path);
}

int hs_route_root_init(void)
{
	int ret = http_server_auth_default_realm_path_add("/");
	retv_if(ret, ret);

	return http_server_route_handler_add(NULL, route_root_callback, NULL, NULL);
}
