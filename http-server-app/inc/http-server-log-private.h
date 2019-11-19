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

#ifndef __HTTP_SERVER_LOG_H__
#define __HTTP_SERVER_LOG_H__

#include <dlog.h>

#ifdef	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG "HTTPSERVER"

#if !defined(_V)
#define _V(fmt, arg...) dlog_print(DLOG_VERBOSE, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#if !defined(_D)
#define _D(fmt, arg...) dlog_print(DLOG_DEBUG, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#if !defined(_I)
#define _I(fmt, arg...) dlog_print(DLOG_INFO, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#if !defined(_W)
#define _W(fmt, arg...) dlog_print(DLOG_WARN, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#if !defined(_E)
#define _E(fmt, arg...) dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#endif

#define retvm_if(expr, val, fmt, arg...) do { \
	if (expr) { \
		_E(fmt, ##arg); \
		_E("[%s:%d] (%s) -> %s() return", __func__, __LINE__, #expr, __FUNCTION__); \
		return val; \
	} \
} while (0)

#define retv_if(expr, val) do { \
	if (expr) { \
		_E("[%s:%d] (%s) -> %s() return", __func__, __LINE__, #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retm_if(expr, fmt, arg...) do { \
	if (expr) { \
		_E(fmt, ##arg); \
		_E("[%s:%d] (%s) -> %s() return", __func__, __LINE__, #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define ret_if(expr) do { \
	if (expr) { \
		_E("[%s:%d] (%s) -> %s() return", __func__, __LINE__, #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define goto_if(expr, val) do { \
	if (expr) { \
		_E("[%s:%d] (%s) -> goto", __func__, __LINE__, #expr); \
		goto val; \
	} \
} while (0)

#define break_if(expr) { \
	if (expr) { \
		_E("[%s:%d] (%s) -> break", __func__, __LINE__, #expr); \
		break; \
	} \
}

#define continue_if(expr) { \
	if (expr) { \
		_E("[%s:%d] (%s) -> continue", __func__, __LINE__, #expr); \
		continue; \
	} \
}

#endif /* __HTTP_SERVER_LOG_H__*/
