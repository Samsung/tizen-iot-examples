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
#include <net_connection.h>
#include "http-server-log-private.h"
#include "http-server-route.h"

#define API_CONNECTION "/api/connection"
#define API_CONNECTION_LEN sizeof(API_CONNECTION)



#define DEACTIVATED_STR "deactivated"
#define DISCONNECTED_STR "disconnected"
#define CONNECTED_STR "connected"

//declare sub modules
#define API_SUB_WIFI "wifiScan"
extern void handle_connection_wifi(SoupMessage *msg, GHashTable *query);


static const char *get_connection_type(connection_h connection)
{
	connection_type_e type = CONNECTION_TYPE_DISCONNECTED;
	const char *conn_type = NULL;

	retv_if(!connection, "unknown");

	connection_get_type(connection, &type);

	switch (type) {
	case CONNECTION_TYPE_DISCONNECTED:
		conn_type = DISCONNECTED_STR;
		break;
	case CONNECTION_TYPE_WIFI:
		conn_type = "wifi";
		break;
	case CONNECTION_TYPE_CELLULAR:
		conn_type = "cellular";
		break;
	case CONNECTION_TYPE_ETHERNET:
		conn_type = "ethernet";
		break;
	case CONNECTION_TYPE_BT:
		conn_type = "bluetooth";
		break;
	case CONNECTION_TYPE_NET_PROXY:
		conn_type = "net proxy";
		break;
	default:
		conn_type = "unknown";
		break;
	}

	return conn_type;
}

static const char *get_wifi_state(connection_h connection)
{
	connection_wifi_state_e wifi_state = CONNECTION_WIFI_STATE_DEACTIVATED;
	const char *wifi = NULL;

	retv_if(!connection, "unknown");
	connection_get_wifi_state(connection, &wifi_state);

	switch (wifi_state) {
	case CONNECTION_WIFI_STATE_DEACTIVATED:
		wifi = DEACTIVATED_STR;
		break;
	case CONNECTION_WIFI_STATE_DISCONNECTED:
		wifi = DISCONNECTED_STR;
		break;
	case CONNECTION_WIFI_STATE_CONNECTED:
		wifi = CONNECTED_STR;
	}

	return wifi;
}

static const char *get_ethernet_state(connection_h connection)
{
	connection_ethernet_state_e eth_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;
	const char *ethernet = NULL;

	retv_if(!connection, "unknown");
	connection_get_ethernet_state(connection, &eth_state);

	switch (eth_state) {
	case CONNECTION_ETHERNET_STATE_DEACTIVATED:
		ethernet = DEACTIVATED_STR;
		break;
	case CONNECTION_ETHERNET_STATE_DISCONNECTED:
		ethernet = DISCONNECTED_STR;
		break;
	case CONNECTION_ETHERNET_STATE_CONNECTED:
		ethernet = CONNECTED_STR;
	}

	return ethernet;
}

static const char *get_bt_state(connection_h connection)
{
	connection_bt_state_e bt_state = CONNECTION_BT_STATE_DEACTIVATED;
	const char *bt = NULL;

	retv_if(!connection, "unknown");
	connection_get_bt_state(connection, &bt_state);

	switch (bt_state) {
	case CONNECTION_BT_STATE_DEACTIVATED:
		bt = DEACTIVATED_STR;
		break;
	case CONNECTION_BT_STATE_DISCONNECTED:
		bt = DISCONNECTED_STR;
		break;
	case CONNECTION_BT_STATE_CONNECTED:
		bt = CONNECTED_STR;
	}

	return bt;
}

static void
handle_sub_path(SoupMessage *msg, const char *sub_path, GHashTable *query)
{
	_D("sub path : %s", sub_path);
	if (0 == g_strcmp0(sub_path, API_SUB_WIFI))
		handle_connection_wifi(msg, query);
	else
		soup_message_set_status(msg, SOUP_STATUS_BAD_REQUEST);
}

static void handle_connection_info(SoupMessage *msg)
{
	connection_h connection = NULL;
	SoupBuffer *buffer;
	char *response_msg = NULL;

	if (msg->method != SOUP_METHOD_GET) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}

	connection_create(&connection);

	response_msg = g_strdup_printf(
				"{ \"connection_type\": \"%s\", "
				"\"wifi\": \"%s\", "
				"\"ethernet\": \"%s\", "
				"\"bluetooth\": \"%s\" }",
				get_connection_type(connection),
				get_wifi_state(connection),
				get_ethernet_state(connection),
				get_bt_state(connection));

	connection_destroy(connection);
	connection = NULL;

	buffer = soup_buffer_new_with_owner(response_msg, strlen(response_msg),
					response_msg, (GDestroyNotify)g_free);
	soup_message_body_append_buffer(msg->response_body, buffer);
	soup_buffer_free(buffer);

	soup_message_headers_set_content_type(
						msg->response_headers, "application/json", NULL);

	soup_message_set_status(msg, SOUP_STATUS_OK);
}

static void route_api_connection_callback(SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	if (strlen(path) <= API_CONNECTION_LEN) {
		handle_connection_info(msg);
	} else {
		const char *sub_path = path + API_CONNECTION_LEN;
		handle_sub_path(msg, sub_path, query);
	}
}

int hs_route_api_connection_init(void)
{
	int ret = 0;

	ret = http_server_route_handler_add(API_CONNECTION,
				route_api_connection_callback, NULL, NULL);

	return ret;
}
