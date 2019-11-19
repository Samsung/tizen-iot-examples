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
#include <json-glib/json-glib.h>
#include "http-server-log-private.h"

char *util_json_generate_str(JsonBuilder *builder, gsize *len)
{
	JsonNode *root = NULL;
	JsonGenerator *generator = NULL;
	char *str = NULL;
	gsize length = 0;

	retv_if(!builder, NULL);

	root = json_builder_get_root(builder);
	generator = json_generator_new();

	json_generator_set_root(generator, root);
	str = json_generator_to_data(generator, &length);

	if (len)
		*len = length;

	if (length == 0) {
		g_free(str);
		str = NULL;
	}

	g_object_unref(generator);
	json_node_unref(root);

	return str;
}

void
util_json_add_int(JsonBuilder *builder, const gchar *name, gint64 value)
{
	ret_if(!builder);
	ret_if(!name);

	json_builder_set_member_name(builder, name);
	json_builder_add_int_value(builder, value);
}

void
util_json_add_double(JsonBuilder *builder, const gchar *name, gdouble value)
{
	ret_if(!builder);
	ret_if(!name);

	json_builder_set_member_name(builder, name);
	json_builder_add_double_value(builder, value);
}

void
util_json_add_bool(JsonBuilder *builder, const gchar *name, gboolean value)
{
	ret_if(!builder);
	ret_if(!name);

	json_builder_set_member_name(builder, name);
	json_builder_add_boolean_value(builder, value);
}

void
util_json_add_str(JsonBuilder *builder, const gchar *name, const gchar *value)
{
	ret_if(!builder);
	ret_if(!name);

	json_builder_set_member_name(builder, name);
	json_builder_add_string_value(builder, value);
}

void
util_json_add_null(JsonBuilder *builder, const gchar *name)
{
	ret_if(!builder);
	ret_if(!name);

	json_builder_set_member_name(builder, name);
	json_builder_add_null_value(builder);
}
