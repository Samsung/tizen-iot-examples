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

#include <service_app.h>
#include <net_connection.h>
#include "http-server-log-private.h"
#include "http-server-common.h"
#include "hs-route-root.h"
#include "hs-route-api-connection.h"
#include "hs-route-api-applist.h"
#include "hs-route-api-sysinfo.h"
#include "hs-route-api-storage.h"
#include "hs-route-api-image-upload.h"


#define SERVER_NAME "http-server-app"
#define SERVER_PORT 8080

struct app_data {
	connection_h conn_h;
	connection_type_e cur_conn_type;
};

static int route_modules_init(void)
{
	int ret = 0;

	ret = hs_route_root_init();
	retv_if(ret, -1);

	ret = hs_route_api_connection_init();
	retv_if(ret, -1);

	ret = hs_route_api_applist_init();
	retv_if(ret, -1);

	ret = hs_route_api_sysinfo_init();
	retv_if(ret, -1);

	ret = hs_route_api_storage_init();
	retv_if(ret, -1);

	ret = hs_route_api_image_upload_init();
	retv_if(ret, -1);


	return 0;
}

static void server_destroy(void)
{
	http_server_destroy();
	_D("server is destroyed");
}

static int server_init_n_start(void)
{
	int ret = 0;

	ret = http_server_create(SERVER_NAME, SERVER_PORT);
	retv_if(ret, -1);

	ret = route_modules_init();
	retv_if(ret, -1);

	ret = http_server_start();
	retv_if(ret, -1);

	_D("server is started");
	return 0;
}

static void conn_type_changed_cb(connection_type_e type, void *data)
{
	struct app_data *ad = data;

	_D("connection type is changed [%d] -> [%d]", ad->cur_conn_type, type);

	server_destroy();

	if (type != CONNECTION_TYPE_DISCONNECTED) {
		int ret = 0;
		_D("restart server");
		ret = server_init_n_start();
		if (ret) {
			_E("failed to start server");
			service_app_exit();
		}
	}

	ad->cur_conn_type = type;

	return;
}

static void service_app_control(app_control_h app_control, void *data)
{
}

static bool service_app_create(void *data)
{
	struct app_data *ad = data;
	int ret = 0;

	retv_if(!ad, false);

	ret = connection_create(&ad->conn_h);
	retv_if(ret, false);

	ret = connection_set_type_changed_cb(ad->conn_h, conn_type_changed_cb, ad);
	goto_if(ret, ERROR);

	connection_get_type(ad->conn_h, &ad->cur_conn_type);
	if (ad->cur_conn_type == CONNECTION_TYPE_DISCONNECTED) {
		_D("network is not connected, waiting to be connected to any type of network");
		return true;
	}

	ret = server_init_n_start();
	goto_if(ret, ERROR);

	return true;

ERROR:
	if (ad->conn_h)
		connection_destroy(ad->conn_h);

	server_destroy();
	return false;
}

static void service_app_terminate(void *data)
{
	struct app_data *ad = data;

	server_destroy();

	if (ad->conn_h) {
		connection_destroy(ad->conn_h);
		ad->conn_h = NULL;
	}
	ad->cur_conn_type = CONNECTION_TYPE_DISCONNECTED;

	return;
}

int main(int argc, char* argv[])
{
	struct app_data ad;
	service_app_lifecycle_callback_s event_callback;

	ad.conn_h = NULL;
	ad.cur_conn_type = CONNECTION_TYPE_DISCONNECTED;

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_main(argc, argv, &event_callback, &ad);

	return 0;
}
