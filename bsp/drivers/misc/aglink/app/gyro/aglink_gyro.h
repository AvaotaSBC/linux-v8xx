#ifndef _AGLINK_GYRO_H_
#define _AGLINK_GYRO_H_

#include <linux/types.h>

#define GYRO_INT1 0x1
#define GYRO_INT2 0x2
#define GYRO_INT_MASK 0x3

struct gyro_data {
	float x;
	float y;
	float z;
};

typedef union {
	s16 i16bit[3];
	u8 u8bit[6];
} gyro3bit16_t;

struct gyro_config {
	u8 xl_en;
	u8 gy_en;
	u8 speed;
	u8 ts_en;
	u8 int_sel;
	u8 gpio_active;
	u16 addr;
	u32 reg;
	s32 gpio_num;
	u32 fifo_deep;
};

struct aglink_gyro_hwio_ops {
	int (*write)(u16 addr, u8 reg, const u8 *bufp, u16 len);
	int (*read)(u16 addr, u8 reg, u8 *bufp, u16 len);
};

struct gyro_ops {
	int (*probe)(struct aglink_gyro_hwio_ops *gyro_hwio, struct gyro_config config);
	int (*remove)(struct gyro_config config);
	int (*racc)(struct gyro_data *gyrodata);
	int (*rang)(struct gyro_data *gyrodata);
	s16 (*get_fifo_num)(void);
	s16 (*get_fifo_dat)(struct gyro_data *gyrodata);
	int (*get_fifo_time_ns)(u64 *time);
	int (*read_fifo_start)(void);
	int (*read_fifo_stop)(void);
	float (*get_speed)(u8 speed_cnt);
};

enum aglink_gyro_hw_id {
	AGLINK_LSM6DSOX_ID,
	AGLINK_GYRO_ID_MAX,
};

/* define in aglink_gyro.c, call for aglink_gyro_hwio.c*/
void aglink_gyro_set_ops(u32 dev_id);
void aglink_gyro_clear_ops(void);

/* define in aglink_gyro_hwio.c, call for take_gyro.c*/
void aglink_gyro_get_gpio_config(int *gpio_num, u8 *gpio_active);
bool aglink_gyro_is_init(void);

/* define in aglink_gyro.c, call for aglink_gyro_hwio.c*/
s32 aglink_gpro_chip_probe(struct aglink_gyro_hwio_ops *gyro_hwio, struct gyro_config gyroconfig);
s32 aglink_gpro_chip_remove(struct gyro_config gyroconfig);
/* define in aglink_gyro.c, call for take_gyro.c*/
int aglink_gyro_read_acceleration(struct gyro_data *gyrodata);
int aglink_gyro_read_angular(struct gyro_data *gyrodata);
int aglink_gyro_read_fifo_start(void);
int aglink_gyro_read_fifo_stop(void);
s16 aglink_gyro_get_fifo_num(void);
s16 aglink_gyro_get_fifo_dat(struct gyro_data *gyrodata);
int aglink_gyro_get_timestamp_from_fifo_ns(u64 *time);
float aglink_gyro_get_speed(u8 speed_cnt);
#endif
