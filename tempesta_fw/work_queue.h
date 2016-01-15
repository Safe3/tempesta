/**
 *		Tempesta FW
 *
 * Copyright (C) 2016 Tempesta Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __TFW_WORK_QUEUE_H__
#define __TFW_WORK_QUEUE_H__

#include <linux/irq_work.h>

#define WQ_ITEM_SZ		32
#define TFW_WQ_CHECKSZ(t)	BUG_ON(sizeof(t) != WQ_ITEM_SZ)
typedef struct {
	long			_[WQ_ITEM_SZ / sizeof(long)];
} __WqItem;

typedef struct __ThrPos {
	atomic64_t		tail, head;
} __ThrPos;

typedef struct {
	__ThrPos __percpu	*thr_pos;
	__WqItem		*array;
	atomic64_t		head ____cacheline_aligned;
	atomic64_t		tail ____cacheline_aligned;
	atomic64_t		last_head ____cacheline_aligned;
	atomic64_t		last_tail ____cacheline_aligned;
} TfwRBQueue;

int tfw_wq_init(TfwRBQueue *wq, int node);
void tfw_wq_destroy(TfwRBQueue *wq);
int __tfw_wq_push(TfwRBQueue *wq, void *ptr);
int tfw_wq_pop(TfwRBQueue *wq, void *buf);

static inline int
tfw_wq_push(TfwRBQueue *wq, void *ptr, int cpu, struct irq_work *work)
{
	int r = __tfw_wq_push(wq, ptr);

	if (unlikely(r))
		return r;

	if (raw_smp_processor_id() != cpu)
		irq_work_queue_on(work, cpu);

	return 0;
}

#endif /* __TFW_WORK_QUEUE_H__ */
