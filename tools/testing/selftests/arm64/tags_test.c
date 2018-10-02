/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/utsname.h>

#define SHIFT_TAG(tag)		((uint64_t)(tag) << 56)
#define SET_TAG(ptr, tag)	(((uint64_t)(ptr) & ~SHIFT_TAG(0xff)) | \
					SHIFT_TAG(tag))

int main(void)
{
	struct utsname utsname;
	void *ptr = &utsname;
	void *tagged_ptr = (void *)SET_TAG(ptr, 0x42);
	int err = uname(tagged_ptr);
	return err;
}
