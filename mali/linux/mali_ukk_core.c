/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2008-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#include <linux/fs.h>       /* file system operations */
#include <linux/slab.h>     /* memort allocation functions */
#include <asm/uaccess.h>    /* user space access */

#include "mali_ukk.h"
#include "mali_osk.h"
#include "mali_kernel_common.h"
#include "mali_session.h"
#include "mali_ukk_wrappers.h"
#include "mali_sync.h"

int get_api_version_wrapper(struct mali_session_data *session_data, _mali_uk_get_api_version_s __user *uargs)
{
	_mali_uk_get_api_version_s kargs;
    _mali_osk_errcode_t err;

    MALI_CHECK_NON_NULL(uargs, -EINVAL);

    if (0 != get_user(kargs.version, &uargs->version)) return -EFAULT;

    kargs.ctx = session_data;
    err = _mali_ukk_get_api_version(&kargs);
    if (_MALI_OSK_ERR_OK != err) return map_errcode(err);

    if (0 != put_user(kargs.version, &uargs->version)) return -EFAULT;
    if (0 != put_user(kargs.compatible, &uargs->compatible)) return -EFAULT;

    return 0;
}

int wait_for_notification_wrapper(struct mali_session_data *session_data, _mali_uk_wait_for_notification_s __user *uargs)
{
    _mali_uk_wait_for_notification_s kargs;
    _mali_osk_errcode_t err;

    MALI_CHECK_NON_NULL(uargs, -EINVAL);

    kargs.ctx = session_data;
    err = _mali_ukk_wait_for_notification(&kargs);
    if (_MALI_OSK_ERR_OK != err) return map_errcode(err);

	if(_MALI_NOTIFICATION_CORE_SHUTDOWN_IN_PROGRESS != kargs.type)
	{
		kargs.ctx = NULL; /* prevent kernel address to be returned to user space */
		if (0 != copy_to_user(uargs, &kargs, sizeof(_mali_uk_wait_for_notification_s))) return -EFAULT;
	}
	else
	{
		if (0 != put_user(kargs.type, &uargs->type)) return -EFAULT;
	}

    return 0;
}

int post_notification_wrapper(struct mali_session_data *session_data, _mali_uk_post_notification_s __user *uargs)
{
	_mali_uk_post_notification_s kargs;
	_mali_osk_errcode_t err;

	MALI_CHECK_NON_NULL(uargs, -EINVAL);

	kargs.ctx = session_data;

	if (0 != get_user(kargs.type, &uargs->type))
	{
		return -EFAULT;
	}

	err = _mali_ukk_post_notification(&kargs);
	if (_MALI_OSK_ERR_OK != err)
	{
		return map_errcode(err);
	}

	return 0;
}

int get_user_settings_wrapper(struct mali_session_data *session_data, _mali_uk_get_user_settings_s __user *uargs)
{
	_mali_uk_get_user_settings_s kargs;
	_mali_osk_errcode_t err;

	MALI_CHECK_NON_NULL(uargs, -EINVAL);

	kargs.ctx = session_data;
	err = _mali_ukk_get_user_settings(&kargs);
	if (_MALI_OSK_ERR_OK != err)
	{
		return map_errcode(err);
	}

	kargs.ctx = NULL; /* prevent kernel address to be returned to user space */
	if (0 != copy_to_user(uargs, &kargs, sizeof(_mali_uk_get_user_settings_s))) return -EFAULT;

	return 0;
}

#ifdef CONFIG_SYNC
int stream_create_wrapper(struct mali_session_data *session_data, _mali_uk_stream_create_s __user *uargs)
{
	_mali_uk_stream_create_s kargs;
	_mali_osk_errcode_t err;
	char name[32];

	MALI_CHECK_NON_NULL(uargs, -EINVAL);

	snprintf(name, 32, "mali-%u", _mali_osk_get_pid());

	kargs.ctx = session_data;
	err = mali_stream_create(name, &kargs.fd);
	if (_MALI_OSK_ERR_OK != err)
	{
		return map_errcode(err);
	}

	kargs.ctx = NULL; /* prevent kernel address to be returned to user space */
	if (0 != copy_to_user(uargs, &kargs, sizeof(_mali_uk_stream_create_s))) return -EFAULT;

	return 0;
}

int sync_fence_validate_wrapper(struct mali_session_data *session, _mali_uk_fence_validate_s __user *uargs)
{
	int fd;
	_mali_osk_errcode_t err;

	if (0 != get_user(fd, &uargs->fd))
	{
		return -EFAULT;
	}

	err = mali_fence_validate(fd);

	if (_MALI_OSK_ERR_OK == err)
	{
		return 0;
	}

	return -EINVAL;
}
#endif
