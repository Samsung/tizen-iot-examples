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

#ifndef __HTTP_SERVER_ROUTE_H__
#define __HTTP_SERVER_ROUTE_H__

#include <glib.h>
#include <libsoup/soup.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*http_server_route_callback) (SoupMessage *msg, const char *path,
						GHashTable *query, SoupClientContext *client,
						gpointer user_data);

int http_server_route_handler_add(const char *route_path,
							http_server_route_callback callback,
							gpointer user_data,
							GDestroyNotify destroy);

int http_server_route_handler_remove(const char *path);

int http_server_pause_message(SoupMessage *msg);
int http_server_unpause_message(SoupMessage *msg);

int http_server_auth_default_realm_path_add(const char *path);
int http_server_auth_default_realm_path_remove(const char *path);

#ifdef __cplusplus
}
#endif
#endif /* __HTTP_SERVER_ROUTE_H__ */

