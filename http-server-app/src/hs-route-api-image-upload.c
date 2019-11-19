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
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include "http-server-log-private.h"
#include "http-server-route.h"

static void
image_file_save(SoupMessage *msg,
	const char *filename, const char *type, SoupBuffer *buffer)
{
	char *response_msg = NULL;

	//TODO : save file(This is mock function, I'm not sure we really need save file?)

	response_msg = g_strdup_printf(""
		"{ \"filename\": \"%s\", \"type\": \"%s\", \"size\": %d }",
		filename, type, buffer->length);

	soup_message_body_append(msg->response_body, SOUP_MEMORY_COPY,
					response_msg, strlen(response_msg));
	g_clear_pointer(&response_msg, g_free);

	soup_message_headers_set_content_type(
						msg->response_headers, "application/json", NULL);

	soup_message_set_status(msg, SOUP_STATUS_OK);
	http_server_unpause_message(msg);
}

static void route_api_image_upload_callback(SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	char *filename = NULL;
	char *type = NULL;
	SoupBuffer *buffer = NULL;
	GHashTable *part_hash = NULL;

	if (msg->method != SOUP_METHOD_POST) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}

	part_hash = soup_form_decode_multipart(msg, "imageFile",
						&filename, &type, &buffer);

	_D("filename : %s, type : %s, file size : %d",
		filename, type, buffer->length);

	http_server_pause_message(msg);

	image_file_save(msg, filename, type, buffer);

	g_free(filename);
	g_free(type);
	soup_buffer_free(buffer);
	g_hash_table_destroy(part_hash);
}

int hs_route_api_image_upload_init(void)
{
	int ret = 0;

	ret = http_server_route_handler_add("/api/imageUpload",
				route_api_image_upload_callback, NULL, NULL);

	return ret;
}
