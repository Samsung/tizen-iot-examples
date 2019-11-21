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
#include <wifi-manager.h>
#include "http-server-log-private.h"
#include "http-server-route.h"
#include "hs-util-json.h"

#define API_SUB_WIFI "wifiScan"

struct wifi_data {
	SoupMessage *msg;
	wifi_manager_h wifi;
	bool activated;
};

static void wifi_deactivated_cb(wifi_manager_error_e result, void *user_data)
{
	struct wifi_data *data = user_data;
	wifi_manager_deinitialize(data->wifi);
	http_server_unpause_message(data->msg);
	g_free(data);
}

static bool wifi_found_ap_cb(wifi_manager_ap_h ap, void *user_data)
{
	JsonBuilder *builder = user_data;
	char *essid = NULL;
	int rssi = 0;
	bool fav = false;

	wifi_manager_ap_get_essid(ap, &essid);
	wifi_manager_ap_get_rssi(ap, &rssi);
	wifi_manager_ap_is_favorite(ap, &fav);

	json_builder_begin_object(builder);

	util_json_add_str(builder, "essid", essid);
	util_json_add_int(builder, "rssi", rssi);
	util_json_add_bool(builder, "favorite", fav);

	json_builder_end_object(builder);

	g_free(essid);

	return true;
}

static void wifi_info_response_append(wifi_manager_h wifi, SoupMessage *msg)
{
	char *response_msg = NULL;
	gsize resp_msg_size = 0;
	JsonBuilder *builder = NULL;

	builder = json_builder_new();

	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "apList");

	json_builder_begin_array(builder);
	wifi_manager_foreach_found_ap(wifi, wifi_found_ap_cb, builder);
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

static void wifi_scan_finished_cb(wifi_manager_error_e result, void *user_data)
{
	struct wifi_data *data = user_data;

	_D("wifi scan finished");

	if (result != WIFI_MANAGER_ERROR_NONE) {
		_E("wifi_scan_finished_cb() with error(%x)", result);
		soup_message_set_status(data->msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		goto OUT;
	}

	wifi_info_response_append(data->wifi, data->msg);

	if (!data->activated) {
		int ret = wifi_manager_deactivate(data->wifi, wifi_deactivated_cb, data);
		if (ret) {
			_E("failed to wifi_manager_deactivate() - %d", ret);
			http_server_unpause_message(data->msg);
		}
		return;
	}

OUT:
	wifi_manager_deinitialize(data->wifi);
	http_server_unpause_message(data->msg);
	g_free(data);
}

static void wifi_activated_cb(wifi_manager_error_e result, void *user_data)
{
	struct wifi_data *data = user_data;

	if (result != WIFI_MANAGER_ERROR_NONE) {
		_E("wifi_activated_cb() with error(%x)", result);
		soup_message_set_status(data->msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		wifi_manager_deinitialize(data->wifi);
		http_server_unpause_message(data->msg);
		g_free(data);
		return;
	}

	wifi_manager_scan(data->wifi, wifi_scan_finished_cb, user_data);
}

void handle_connection_wifi(SoupMessage *msg, GHashTable *query)
{
	wifi_manager_h wifi = NULL;
	bool activated = false;
	struct wifi_data *data = NULL;
	int ret = 0;

	if (msg->method != SOUP_METHOD_GET) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}

	data = g_try_new0(struct wifi_data, 1);
	if (!data) {
		_E("failed to alloc wifi data");
		soup_message_set_status(msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	ret = wifi_manager_initialize(&wifi);
	if (ret) {
		_E("failed to wifi_manager_initialize");
		soup_message_set_status(msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		g_free(data);
		return;
	}

	ret = wifi_manager_is_activated(wifi, &activated);
	if (ret) {
		_E("failed to wifi_manager_is_activated");
		goto ERROR;
	}

	data->msg = msg;
	data->wifi = wifi;
	data->activated = activated;

	if (activated)
		ret = wifi_manager_scan(wifi, wifi_scan_finished_cb, data);
	else
		ret = wifi_manager_activate(wifi, wifi_activated_cb, data);


	if (ret) {
		_E("failed to wifi_manager scan or activate [%d] - %x", activated, ret);
		goto ERROR;
	}

	http_server_pause_message(msg);

	return;
ERROR:
	soup_message_set_status(msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
	wifi_manager_deinitialize(wifi);
	g_free(data);
}
