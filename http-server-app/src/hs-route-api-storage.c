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
#include <storage.h>
#include "http-server-log-private.h"
#include "http-server-route.h"
#include "hs-util-json.h"

static const char *storage_type_to_str(storage_type_e type)
{
	const char *str = NULL;

	switch (type) {
	case STORAGE_TYPE_INTERNAL:
		str = "Internal";
		break;
	case STORAGE_TYPE_EXTERNAL:
		str = "External";
		break;
	case STORAGE_TYPE_EXTENDED_INTERNAL:
		str = "Extended-internal";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

static const char *storage_state_to_str(storage_state_e state)
{
	const char *str = NULL;

	switch (state) {
	case STORAGE_STATE_UNMOUNTABLE:
		str = "Unmountable";
		break;
	case STORAGE_STATE_REMOVED:
		str = "Removed";
		break;
	case STORAGE_STATE_MOUNTED:
		str = "Mounted";
		break;
	case STORAGE_STATE_MOUNTED_READ_ONLY:
		str = "Mounted RO";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

static bool storage_device_callback(int storage_id, storage_type_e type,
					storage_state_e state, const char *path, void *user_data)
{
	JsonBuilder *builder = user_data;
	unsigned long long total = 0;
	unsigned long long avail = 0;
	gint64 total_kb = 0;
	gint64 avail_kb = 0;

	retv_if(!builder, false);

	json_builder_begin_object(builder);

	util_json_add_int(builder, "id", storage_id);
	util_json_add_str(builder, "type", storage_type_to_str(type));
	util_json_add_str(builder, "state", storage_state_to_str(state));
	util_json_add_str(builder, "path", path);

	storage_get_total_space(storage_id, &total);
	if (total > 0)
		total_kb = total / 1024;
	util_json_add_int(builder, "totalSpace", total_kb);

	storage_get_available_space(storage_id, &avail);
	if (avail > 0)
		avail_kb = avail / 1024;
	util_json_add_int(builder, "availSpace", avail_kb);

	json_builder_end_object(builder);

	return true;
}

static void route_api_storage_callback(SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	int ret = 0;
	char *response_msg = NULL;
	gsize resp_msg_size = 0;
	JsonBuilder *builder = NULL;

	if (msg->method != SOUP_METHOD_GET) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}

	builder = json_builder_new();
	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "storageInfoList");
	json_builder_begin_array(builder);


	ret = storage_foreach_device_supported(storage_device_callback, builder);
	if (ret) {
		soup_message_set_status(msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		g_object_unref(builder);
		return;
	}

	json_builder_end_array(builder);
	json_builder_end_object(builder);

	response_msg = util_json_generate_str(builder, &resp_msg_size);
	g_clear_pointer(&builder, g_object_unref);

	soup_message_body_append(msg->response_body, SOUP_MEMORY_COPY,
					response_msg, resp_msg_size);
	g_clear_pointer(&response_msg, g_free);

	soup_message_headers_set_content_type(
						msg->response_headers, "application/json", NULL);

	soup_message_set_status(msg, SOUP_STATUS_OK);
}

int hs_route_api_storage_init(void)
{
	return http_server_route_handler_add("/api/storage",
				route_api_storage_callback, NULL, NULL);
}
