#include "lsm6dsox_reg.h"
#include "../aglink_gyro.h"
#include "debug.h"

static stmdev_ctx_t dev_ctx;
static u8 whoamI;
static struct aglink_gyro_hwio_ops *hwio;
static struct gyro_config gyro_attri;
#define BIT_ERR(x) (1 << (x))

#define LSM6DSOX_ADDR ((LSM6DSOX_I2C_ADD_L >> 1) & 0xff)
#define LSM6DSOX_SPEED_CNT_MAX 11

float gyro_speed[12] = {0.0, 12.5, 26.0, 52.0, 104.0, 208.0, 417.0, 833.0, 1667.0, 3333.0, 6667.0, 6.5};

static s32 platform_write(void *handle, u8 reg, const u8 *bufp,
								u16 len)
{
	return hwio->write(LSM6DSOX_ADDR, reg, bufp, len);
}

static s32 platform_read(void *handle, u8 reg, u8 *bufp,
							u16 len)
{
	return hwio->read(LSM6DSOX_ADDR, reg, bufp, len);
}

static void platform_delay(u32 ms)
{
	msleep(ms);
}

static s32 lsm6dsox_probe(struct aglink_gyro_hwio_ops *gyro_hwio, struct gyro_config config)
{
	u32 timeout = 0xfff;
	u8 rst;
	s32 ret = 0;

	hwio = gyro_hwio;
	/* Initialize mems driver interface */
	dev_ctx.write_reg = platform_write;
	dev_ctx.read_reg = platform_read;
	dev_ctx.mdelay = platform_delay;
	dev_ctx.handle = NULL;

	/* Check device ID */
	lsm6dsox_device_id_get(&dev_ctx, &whoamI);

	if (whoamI != LSM6DSOX_ID) {
		while (1 && timeout--)
			;
		aglink_printk(AGLINK_DBG_ERROR, "get id(%d) failed\n", whoamI);
		goto id_timeout;
	}

	/* Restore default configuration */
	lsm6dsox_reset_set(&dev_ctx, PROPERTY_ENABLE);

	do {
		lsm6dsox_reset_get(&dev_ctx, &rst);
	} while (rst && timeout--);

	if (rst) {
		aglink_printk(AGLINK_DBG_ERROR, "reset failed\n");
		goto reset_timeout;
	}

	memset(&gyro_attri, 0x00, sizeof(gyro_attri));
	memcpy(&gyro_attri, &config, sizeof(gyro_attri));

	/* Disable I3C interface */
	if (lsm6dsox_i3c_disable_set(&dev_ctx, LSM6DSOX_I3C_DISABLE))
		ret |= BIT_ERR(0);
	/* Enable Block Data Update */
	if (lsm6dsox_block_data_update_set(&dev_ctx, PROPERTY_ENABLE))
		ret |= BIT_ERR(1);
	/* Set full scale */
	if (lsm6dsox_xl_full_scale_set(&dev_ctx, LSM6DSOX_2g))
		ret |= BIT_ERR(2);
	if (lsm6dsox_gy_full_scale_set(&dev_ctx, LSM6DSOX_2000dps))
		ret |= BIT_ERR(3);
	/*
	 * Set FIFO watermark (number of unread sensor data TAG + 6 bytes
	 * stored in FIFO)
	 */
	if (lsm6dsox_fifo_watermark_set(&dev_ctx, gyro_attri.fifo_deep))
		ret |= BIT_ERR(4);
	/* Set FIFO mode to Stream mode (aka Continuous Mode) */
	if (lsm6dsox_fifo_mode_set(&dev_ctx, LSM6DSOX_STREAM_MODE))
		ret |= BIT_ERR(5);
	/* Enable drdy 75 μs pulse: uncomment if interrupt must be pulsed */
	if (lsm6dsox_data_ready_mode_set(&dev_ctx, LSM6DSOX_DRDY_PULSED))
		ret |= BIT_ERR(6);

	if (lsm6dsox_pin_polarity_set(&dev_ctx,
								 gyro_attri.gpio_active ? LSM6DSOX_ACTIVE_HIGH : LSM6DSOX_ACTIVE_LOW))
		ret |= BIT_ERR(7);

	if (ret) {
		aglink_printk(AGLINK_DBG_ERROR, "lsm6dsox init failed (%08x)\n", ret);
		return -1;
	}

	aglink_printk(AGLINK_DBG_ALWY, "lsm6dsox init succces\n");

	return 0;

id_timeout:
reset_timeout:
	return -1;
}

static s32 lsm6dsox_remove(struct gyro_config config)
{
	memset(&gyro_attri, 0x00, sizeof(gyro_attri));
	hwio = NULL;
	dev_ctx.write_reg = NULL;
	dev_ctx.read_reg = NULL;
	dev_ctx.mdelay = NULL;
	dev_ctx.handle = NULL;
	return 0;
}

static s32 lsm6dsox_fifo_start(void)
{
	int ret = 0;
	/* Uncomment to configure INT 1 */
	lsm6dsox_pin_int1_route_t int1_route;
	/* Uncomment to configure INT 2 */
	lsm6dsox_pin_int2_route_t int2_route;
	/* Uncomment if interrupt generation on Free Fall INT1 pin */
	if ((gyro_attri.int_sel & GYRO_INT_MASK) == GYRO_INT1) {
		if (lsm6dsox_pin_int1_route_get(&dev_ctx, &int1_route))
			ret |= BIT_ERR(0);
		int1_route.fifo_th = PROPERTY_ENABLE;
		if (lsm6dsox_pin_int1_route_set(&dev_ctx, int1_route))
			ret |= BIT_ERR(1);
	}
	/* Uncomment if interrupt generation on Free Fall INT2 pin */
	if ((gyro_attri.int_sel & GYRO_INT_MASK) == GYRO_INT2) {
		if (lsm6dsox_pin_int2_route_get(&dev_ctx, NULL, &int2_route))
			ret |= BIT_ERR(2);
		int2_route.fifo_th = PROPERTY_ENABLE;
		if (lsm6dsox_pin_int2_route_set(&dev_ctx, NULL, int2_route))
			ret |= BIT_ERR(3);
	}

	if (gyro_attri.xl_en) {
		if (lsm6dsox_fifo_xl_batch_set(&dev_ctx, gyro_attri.speed))
			ret |= BIT_ERR(4);
		if (lsm6dsox_xl_data_rate_set(&dev_ctx, gyro_attri.speed))
			ret |= BIT_ERR(5);
	}

	if (gyro_attri.gy_en) {
		if (lsm6dsox_fifo_gy_batch_set(&dev_ctx, gyro_attri.speed))
			ret |= BIT_ERR(6);
		if (lsm6dsox_gy_data_rate_set(&dev_ctx, gyro_attri.speed))
			ret |= BIT_ERR(7);
	}

	if (gyro_attri.ts_en) {
		if (lsm6dsox_fifo_timestamp_decimation_set(&dev_ctx, LSM6DSOX_DEC_1))
			ret |= BIT_ERR(8);
		if (lsm6dsox_timestamp_set(&dev_ctx, PROPERTY_ENABLE))
			ret |= BIT_ERR(9);
	}

	return ret;
}

static s32 lsm6dsox_fifo_stop(void)
{
	int ret = 0;
	/* Uncomment to configure INT 1 */
	lsm6dsox_pin_int1_route_t int1_route;
	/* Uncomment to configure INT 2 */
	lsm6dsox_pin_int2_route_t int2_route;
	/* Uncomment if interrupt generation on Free Fall INT1 pin */

	if (gyro_attri.xl_en) {
		if (lsm6dsox_fifo_xl_batch_set(&dev_ctx, LSM6DSOX_XL_NOT_BATCHED))
			ret |= BIT_ERR(0);
		if (lsm6dsox_xl_data_rate_set(&dev_ctx, LSM6DSOX_XL_ODR_OFF))
			ret |= BIT_ERR(1);
	}

	if (gyro_attri.gy_en) {
		if (lsm6dsox_fifo_gy_batch_set(&dev_ctx, LSM6DSOX_GY_NOT_BATCHED))
			ret |= BIT_ERR(2);
		if (lsm6dsox_gy_data_rate_set(&dev_ctx, LSM6DSOX_XL_ODR_OFF))
			ret |= BIT_ERR(3);
	}

	if (gyro_attri.ts_en) {
		if (lsm6dsox_fifo_timestamp_decimation_set(&dev_ctx, LSM6DSOX_NO_DECIMATION))
			ret |= BIT_ERR(4);
		if (lsm6dsox_timestamp_set(&dev_ctx, PROPERTY_DISABLE))
			ret |= BIT_ERR(5);
	}

	if ((gyro_attri.int_sel & GYRO_INT_MASK) == GYRO_INT1) {
		if (lsm6dsox_pin_int1_route_get(&dev_ctx, &int1_route))
			ret |= BIT_ERR(6);
		int1_route.fifo_th = PROPERTY_DISABLE;
		if (lsm6dsox_pin_int1_route_set(&dev_ctx, int1_route))
			ret |= BIT_ERR(7);
	}
	/* Uncomment if interrupt generation on Free Fall INT2 pin */
	if ((gyro_attri.int_sel & GYRO_INT_MASK) == GYRO_INT2) {
		if (lsm6dsox_pin_int2_route_get(&dev_ctx, NULL, &int2_route))
			ret |= BIT_ERR(8);
		int2_route.fifo_th = PROPERTY_DISABLE;
		if (lsm6dsox_pin_int2_route_set(&dev_ctx, NULL, int2_route))
			ret |= BIT_ERR(9);
	}

	return ret;
}

static s16 lsm6dsox_get_fifo_num(void)
{
	u8 wmflag = 0;
	u16 num = 0;
	if (lsm6dsox_fifo_wtm_flag_get(&dev_ctx, &wmflag))
		return -1;
	if (wmflag > 0) {
		if (lsm6dsox_fifo_data_level_get(&dev_ctx, &num))
			return -1;
	}
	return (s16)num;
}

static s16 lsm6dsox_get_gy_fifo_data(struct gyro_data *gyrodata)
{
	lsm6dsox_fifo_tag_t reg_tag;
	gyro3bit16_t gyrodat;
	lsm6dsox_fifo_sensor_tag_get(&dev_ctx, &reg_tag);
	if (reg_tag == LSM6DSOX_GYRO_NC_TAG) {
		memset(gyrodat.u8bit, 0x00, 3 * sizeof(s16));
		lsm6dsox_fifo_out_raw_get(&dev_ctx, gyrodat.u8bit);
		gyrodata->x =
					lsm6dsox_from_fs2000_to_mdps(gyrodat.i16bit[0]);
		gyrodata->y =
					lsm6dsox_from_fs2000_to_mdps(gyrodat.i16bit[1]);
		gyrodata->z =
					lsm6dsox_from_fs2000_to_mdps(gyrodat.i16bit[2]);
		return 0;
	} else {
		return -1;
	}
}

static int lsm6dsox_get_fifo_time_ns(u64 *time)
{
	lsm6dsox_fifo_tag_t reg_tag;
	gyro3bit16_t gyrodat;
	lsm6dsox_fifo_sensor_tag_get(&dev_ctx, &reg_tag);
	if (reg_tag == LSM6DSOX_TIMESTAMP_TAG) {
		memset(gyrodat.u8bit, 0x00, 3 * sizeof(s16));
		lsm6dsox_fifo_out_raw_get(&dev_ctx, gyrodat.u8bit);
		*time = gyrodat.u8bit[0] |
				(gyrodat.u8bit[1] << 8) |
				(gyrodat.u8bit[2] << 16) |
				(gyrodat.u8bit[3] << 24);
		*time *= 25000;
		return 0;
	}
	*time = 0;
	return -1;
}

static s32 lsm6dsox_read_acceleration(struct gyro_data *gyrodata)
{
	u8 reg;
	s16 data_raw_acceleration[3];
	/* Read output only if new xl value is available */
	lsm6dsox_xl_flag_data_ready_get(&dev_ctx, &reg);

	if (reg) {
		/* Read acceleration field data */
		memset(data_raw_acceleration, 0x00, 3 * sizeof(s16));
		lsm6dsox_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
		gyrodata->x =
			lsm6dsox_from_fs2_to_mg(data_raw_acceleration[0]);
		gyrodata->y =
			lsm6dsox_from_fs2_to_mg(data_raw_acceleration[1]);
		gyrodata->z =
			lsm6dsox_from_fs2_to_mg(data_raw_acceleration[2]);
		return 0;
	}
	return -1;
}

static s32 lsm6dsox_read_angular(struct gyro_data *gyrodata)
{
	u8 reg;
	s16 data_raw_angular_rate[3];
	/* Read output only if new xl value is available */
	lsm6dsox_gy_flag_data_ready_get(&dev_ctx, &reg);

	if (reg) {
		/* Read angular rate field data */
		memset(data_raw_angular_rate, 0x00, 3 * sizeof(s16));
		lsm6dsox_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
		gyrodata->x =
			lsm6dsox_from_fs2000_to_mdps(data_raw_angular_rate[0]);
		gyrodata->y =
			lsm6dsox_from_fs2000_to_mdps(data_raw_angular_rate[1]);
		gyrodata->z =
			lsm6dsox_from_fs2000_to_mdps(data_raw_angular_rate[2]);
		return 0;
	}
	return -1;
}

static float lsm6dsox_get_sample_speed(u8 speed_cnt)
{
	if (speed_cnt > LSM6DSOX_SPEED_CNT_MAX)
		return 0.0;
	return gyro_speed[speed_cnt];
}

struct gyro_ops lsm6dsox_ops = {
	.probe = lsm6dsox_probe,
	.remove = lsm6dsox_remove,
	.racc = lsm6dsox_read_acceleration,
	.rang = lsm6dsox_read_angular,
	.get_fifo_num = lsm6dsox_get_fifo_num,
	.get_fifo_dat = lsm6dsox_get_gy_fifo_data,
	.get_fifo_time_ns = lsm6dsox_get_fifo_time_ns,
	.read_fifo_start = lsm6dsox_fifo_start,
	.read_fifo_stop = lsm6dsox_fifo_stop,
	.get_speed = lsm6dsox_get_sample_speed,
};

