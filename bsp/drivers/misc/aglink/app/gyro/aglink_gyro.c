#include "aglink_gyro.h"
#include <stddef.h>
#include <linux/export.h>
#include "debug.h"

extern struct gyro_ops lsm6dsox_ops;
struct gyro_ops *gyro;

struct gyro_ops *gyro_ops_list[AGLINK_GYRO_ID_MAX] = {
	[AGLINK_LSM6DSOX_ID] = &lsm6dsox_ops,
};

void aglink_gyro_set_ops(u32 dev_id)
{
	gyro = gyro_ops_list[dev_id];
}

void aglink_gyro_clear_ops(void)
{
	gyro = NULL;
}

s32 aglink_gpro_chip_probe(struct aglink_gyro_hwio_ops *gyro_hwio, struct gyro_config gyroconfig)
{
	if (gyro)
		return gyro->probe(gyro_hwio, gyroconfig);
	else
		return -1;
}

s32 aglink_gpro_chip_remove(struct gyro_config gyroconfig)
{
	if (gyro)
		return gyro->remove(gyroconfig);
	else
		return -1;
}

s32 aglink_gyro_read_acceleration(struct gyro_data *gyrodata)
{
	if (gyro)
		return gyro->racc(gyrodata);
	else
		return -1;
}

s32 aglink_gyro_read_angular(struct gyro_data *gyrodata)
{
	if (gyro)
		return gyro->rang(gyrodata);
	else
		return -1;
}

s32 aglink_gyro_read_fifo_start(void)
{
	if (gyro)
		return gyro->read_fifo_start();
	else
		return -1;
}

s32 aglink_gyro_read_fifo_stop(void)
{
	if (gyro)
		return gyro->read_fifo_stop();
	else
		return -1;
}

s16 aglink_gyro_get_fifo_num(void)
{
	if (gyro)
		return gyro->get_fifo_num();
	else
		return -1;
}

s16 aglink_gyro_get_fifo_dat(struct gyro_data *gyrodata)
{
	if (gyro)
		return gyro->get_fifo_dat(gyrodata);
	else
		return -1;
}

s32 aglink_gyro_get_timestamp_from_fifo_ns(u64 *time)
{
	if (gyro)
		return gyro->get_fifo_time_ns(time);
	else
		return -1;
}

float aglink_gyro_get_speed(u8 speed_cnt)
{
	if (gyro)
		return gyro->get_speed(speed_cnt);
	else
		return 0.0;
}
