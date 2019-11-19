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
#include <app_manager.h>
#include "http-server-log-private.h"
#include "http-server-route.h"
#include "hs-util-json.h"

#define ASYNC_RESPONSE 1

#define APP_UNDEFINED "Unknown"
#define APP_RUNNING "Running"
#define APP_NOT_RUNNING "Not Running"

#define APP_FOREGROUND "Foreground"
#define APP_BACKGROUND "Background"
#define APP_SERVICE "Service"
#define APP_TERMINATED "Terminated"

static const char *__app_state_to_str(app_state_e state)
{
	const char *str = NULL;
	switch (state) {
	case APP_STATE_UNDEFINED:
		str = APP_UNDEFINED;
		break;
	case APP_STATE_FOREGROUND:
		str = APP_FOREGROUND;
		break;
	case APP_STATE_BACKGROUND:
		str = APP_BACKGROUND;
		break;
	case APP_STATE_SERVICE:
		str = APP_SERVICE;
		break;
	case APP_STATE_TERMINATED:
		str = APP_TERMINATED;
		break;
	}
	return str;
}

static bool app_info_foreach_cb(app_info_h app_info, void *user_data)
{
	JsonBuilder *builder = user_data;
	char *app_id = NULL;
	app_context_h app_context = NULL;
	int pid = 0;
	int ret = 0;
	const char *app_state = NULL;
	retv_if(!builder, false);

	app_info_get_app_id(app_info, &app_id);
	retv_if(!app_id, false);

	ret = app_manager_get_app_context(app_id, &app_context);
	if (ret != APP_MANAGER_ERROR_NONE) {
		if (ret == APP_MANAGER_ERROR_NO_SUCH_APP) {
			app_state = APP_NOT_RUNNING;
		} else {
			_E("failed to get app context");
			app_state = APP_UNDEFINED;
		}
	} else {
		app_state_e state;
		app_context_get_pid(app_context, &pid);
		app_context_get_app_state(app_context, &state);
		app_state = __app_state_to_str(state);
		app_context_destroy(app_context);
		app_context = NULL;
	}

	json_builder_begin_object(builder);

	util_json_add_str(builder, "appId", app_id);
	util_json_add_str(builder, "appState", app_state);

	if (pid > 0)
		util_json_add_int(builder, "appPid", pid);

	json_builder_end_object(builder);

	return true;
}

static void app_info_response_append(SoupMessage *msg)
{
	char *response_msg = NULL;
	gsize resp_msg_size = 0;
	JsonBuilder *builder = NULL;

	builder = json_builder_new();

	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "installedAppList");

	json_builder_begin_array(builder);
	app_manager_foreach_app_info(app_info_foreach_cb, builder);
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

#if ASYNC_RESPONSE
static gboolean __handle_message_finished(gpointer data)
{
	SoupMessage *msg = data;
	http_server_unpause_message(msg);
	return FALSE;
}

static gpointer app_info_thread(gpointer data)
{
	SoupMessage *msg = data;

	app_info_response_append(msg);

	g_idle_add(__handle_message_finished, msg);

	return NULL;
}
#endif /* ASYNC_RESPONSE */

static void route_api_applist_callback(SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	if (msg->method != SOUP_METHOD_GET) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}

#if ASYNC_RESPONSE
	GThread *thread = g_thread_try_new(NULL, app_info_thread, msg, NULL);
	if (!thread) {
		_E("failed to create thread");
		soup_message_set_status(msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	http_server_pause_message(msg);
#else
	app_info_response_append(msg);
#endif /* ASYNC_RESPONSE */
}

int hs_route_api_applist_init(void)
{
	return http_server_route_handler_add("/api/applicationList",
				route_api_applist_callback, NULL, NULL);
}
