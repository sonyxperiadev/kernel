/*
 * Userspace implementations of fallback calls
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

#ifndef __VDSO_COMPILER_H
#define __VDSO_COMPILER_H

#error	"vdso: Provide architectural overrides such as ARCH_PROVIDES_TIMER,"
#error	"      DEFINE_FALLBACK and __arch_counter_get or any overrides. eg:"
#error	"      vdso entry points or compilation time helpers."

#endif /* __VDSO_COMPILER_H */
