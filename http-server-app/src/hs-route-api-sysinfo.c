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
#include <system_info.h>
#include "http-server-log-private.h"
#include "http-server-route.h"
#include "hs-util-json.h"

#define SYSINFO_MANUFACTURER "http://tizen.org/system/manufacturer"
#define SYSINFO_PROFILE "http://tizen.org/feature/profile"
#define SYSINFO_MODEL_NAME "http://tizen.org/system/model_name"
#define SYSINFO_PLATFORM_VERSION "http://tizen.org/feature/platform.version"
#define SYSINFO_PROCESSOR "http://tizen.org/system/platform.processor"
#define SYSINFO_BUILD "http://tizen.org/system/build.string"
#define SYSINFO_RELEASE "http://tizen.org/system/build.release"
#define SYSINFO_BUILD_TYPE "http://tizen.org/system/build.type"
#define SYSINFO_BUILD_DATE "http://tizen.org/system/build.date"
#define SYSINFO_DISPLAY "http://tizen.org/feature/display"


static void route_api_sysinfo_callback(SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	bool bool_val = false;
	char *str_val = NULL;
	char *response_msg = NULL;
	gsize resp_msg_size = 0;
	JsonBuilder *builder = NULL;

	if (msg->method != SOUP_METHOD_GET) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}

	builder = json_builder_new();
	json_builder_begin_object(builder);

	system_info_get_platform_string(SYSINFO_MANUFACTURER, &str_val);
	util_json_add_str(builder, "manufacturer", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_PROFILE, &str_val);
	util_json_add_str(builder, "profile", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_PLATFORM_VERSION, &str_val);
	util_json_add_str(builder, "platformVersion", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_BUILD, &str_val);
	util_json_add_str(builder, "build", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_RELEASE, &str_val);
	util_json_add_str(builder, "buildRelease", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_BUILD_TYPE, &str_val);
	util_json_add_str(builder, "buildType", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_BUILD_DATE, &str_val);
	util_json_add_str(builder, "buildDate", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_MODEL_NAME, &str_val);
	util_json_add_str(builder, "modelName", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_string(SYSINFO_PROCESSOR, &str_val);
	util_json_add_str(builder, "processor", str_val ? str_val : " ");
	g_clear_pointer(&str_val, g_free);

	system_info_get_platform_bool(SYSINFO_DISPLAY, &bool_val);
	util_json_add_str(builder, "display", bool_val ? "headed" : "headless");

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

int hs_route_api_sysinfo_init(void)
{
	return http_server_route_handler_add("/api/systemInfo",
				route_api_sysinfo_callback, NULL, NULL);
}
