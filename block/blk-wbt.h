#ifndef WB_THROTTLE_H
#define WB_THROTTLE_H

#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/ktime.h>

#include "blk-stat.h"

enum wbt_flags {
	WBT_TRACKED		= 1,	/* write, tracked for throttling */
	WBT_READ		= 2,	/* read */
	WBT_KSWAPD		= 4,	/* write, from kswapd */

	WBT_NR_BITS		= 3,	/* number of bits */
};

enum {
	WBT_NUM_RWQ		= 2,
};

static inline void wbt_clear_state(struct blk_issue_stat *stat)
{
	stat->time &= BLK_STAT_TIME_MASK;
}

static inline enum wbt_flags wbt_stat_to_mask(struct blk_issue_stat *stat)
{
	return (stat->time & BLK_STAT_MASK) >> BLK_STAT_SHIFT;
}

static inline void wbt_track(struct blk_issue_stat *stat, enum wbt_flags wb_acct)
{
	stat->time |= ((u64) wb_acct) << BLK_STAT_SHIFT;
}

static inline bool wbt_is_tracked(struct blk_issue_stat *stat)
{
	return (stat->time >> BLK_STAT_SHIFT) & WBT_TRACKED;
}

static inline bool wbt_is_read(struct blk_issue_stat *stat)
{
	return (stat->time >> BLK_STAT_SHIFT) & WBT_READ;
}

struct rq_wait {
	wait_queue_head_t wait;
	atomic_t inflight;
};

struct rq_wb {
	/*
	 * Settings that govern how we throttle
	 */
	unsigned int wb_background;		/* background writeback */
	unsigned int wb_normal;			/* normal writeback */
	unsigned int wb_max;			/* max throughput writeback */
	int scale_step;
	bool scaled_max;

	/*
	 * Number of consecutive periods where we don't have enough
	 * information to make a firm scale up/down decision.
	 */
	unsigned int unknown_cnt;

	u64 win_nsec;				/* default window size */
	u64 cur_win_nsec;			/* current window size */

	struct timer_list window_timer;

	s64 sync_issue;
	void *sync_cookie;

	unsigned int wc;
	unsigned int queue_depth;

	unsigned long last_issue;		/* last non-throttled issue */
	unsigned long last_comp;		/* last non-throttled comp */
	unsigned long min_lat_nsec;
	struct request_queue *queue;
	struct rq_wait rq_wait[WBT_NUM_RWQ];
};

static inline unsigned int wbt_inflight(struct rq_wb *rwb)
{
	unsigned int i, ret = 0;

	for (i = 0; i < WBT_NUM_RWQ; i++)
		ret += atomic_read(&rwb->rq_wait[i].inflight);

	return ret;
}

#ifdef CONFIG_BLK_WBT

void __wbt_done(struct rq_wb *, enum wbt_flags);
void wbt_done(struct rq_wb *, struct blk_issue_stat *);
enum wbt_flags wbt_wait(struct rq_wb *, struct bio *, spinlock_t *);
int wbt_init(struct request_queue *);
void wbt_exit(struct request_queue *);
void wbt_update_limits(struct rq_wb *);
void wbt_requeue(struct rq_wb *, struct blk_issue_stat *);
void wbt_issue(struct rq_wb *, struct blk_issue_stat *);
void wbt_disable(struct rq_wb *);

void wbt_set_queue_depth(struct rq_wb *, unsigned int);
void wbt_set_write_cache(struct rq_wb *, bool);

#else

static inline void __wbt_done(struct rq_wb *rwb, enum wbt_flags flags)
{
}
static inline void wbt_done(struct rq_wb *rwb, struct blk_issue_stat *stat)
{
}
static inline enum wbt_flags wbt_wait(struct rq_wb *rwb, struct bio *bio,
				      spinlock_t *lock)
{
	return 0;
}
static inline int wbt_init(struct request_queue *q)
{
	return -EINVAL;
}
static inline void wbt_exit(struct request_queue *q)
{
}
static inline void wbt_update_limits(struct rq_wb *rwb)
{
}
static inline void wbt_requeue(struct rq_wb *rwb, struct blk_issue_stat *stat)
{
}
static inline void wbt_issue(struct rq_wb *rwb, struct blk_issue_stat *stat)
{
}
static inline void wbt_disable(struct rq_wb *rwb)
{
}
static inline void wbt_set_queue_depth(struct rq_wb *rwb, unsigned int depth)
{
}
static inline void wbt_set_write_cache(struct rq_wb *rwb, bool wc)
{
}

#endif /* CONFIG_BLK_WBT */

#endif
