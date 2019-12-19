/*
 * Userspace implementations of __get_datapage
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VDSO_DATAPAGE_H
#define __VDSO_DATAPAGE_H

#include <linux/bitops.h>
#include <linux/types.h>
#include <asm/vdso_datapage.h>

/*
 * We use the hidden visibility to prevent the compiler from generating a GOT
 * relocation. Not only is going through a GOT useless (the entry couldn't and
 * mustn't be overridden by another library), it does not even work: the linker
 * cannot generate an absolute address to the data page.
 *
 * With the hidden visibility, the compiler simply generates a PC-relative
 * relocation (R_ARM_REL32), and this is what we need.
 */
extern const struct vdso_data _vdso_data __attribute__((visibility("hidden")));

static inline const struct vdso_data *__get_datapage(void)
{
	const struct vdso_data *ret;
	/*
	 * This simply puts &_vdso_data into ret. The reason why we don't use
	 * `ret = &_vdso_data` is that the compiler tends to optimise this in a
	 * very suboptimal way: instead of keeping &_vdso_data in a register,
	 * it goes through a relocation almost every time _vdso_data must be
	 * accessed (even in subfunctions). This is both time and space
	 * consuming: each relocation uses a word in the code section, and it
	 * has to be loaded at runtime.
	 *
	 * This trick hides the assignment from the compiler. Since it cannot
	 * track where the pointer comes from, it will only use one relocation
	 * where __get_datapage() is called, and then keep the result in a
	 * register.
	 */
	asm("" : "=r"(ret) : "0"(&_vdso_data));
	return ret;
}

/* We can only guarantee 56 bits of precision. */
#define ARCH_CLOCK_FIXED_MASK GENMASK_ULL(55, 0)

#endif /* __VDSO_DATAPAGE_H */
