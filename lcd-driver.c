#include "asm/bug.h"
#include "linux/device.h"
#include "linux/device/devres.h"
#include "linux/gfp_types.h"
#include "linux/jiffies.h"
#include "linux/timer.h"
#include "linux/timer_types.h"
#include "linux/types.h"
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
// #include <linux/cdev.h>
// #include <linux/fs.h>
// #include <linux/delay.h>
// #include <linux/string.h>

static const struct i2c_device_id g_lcd_driver_idtable[] = { { "lcd-device",
							       0 },
							     {} };

MODULE_DEVICE_TABLE(i2c, g_lcd_driver_idtable);

struct device_data {
	struct i2c_client *i2c_client;
	struct timer_list timer;
};

static bool g_backlight_enabled = true;

// D7, D6, D5, D4, BT, E, RW, RS
#define ENABLE 0b00000100
#define STATE (g_backlight_enabled << 3)

// TODO Tune
#define PRE_ENABLE_DELAY 50
#define ENABLE_PULSE_TIME 400
#define POST_ENABLE_DELAY 50
#define CYCLE_END_DELAY 50

// D7, D6, D5, D4, D3, D2, D1, D0
#define LCD_CMD_4_BIT_MODE 0b00100000
#define LCD_CMD_CLEAR 0b00000001

#define LCD_CMD_HOME 0b00000010

// TODO Entry mode options
#define LCD_CMD_ENTRY_MODE 0b00000100
#define LCD_CMD_ENTRY_MODE_INCREMENT 0b00000001
#define LCD_CMD_ENTRY_MODE_SHIFT 0b00000010

// TODO Unfinished
#define LCD_CMD_DISPLAY_CONTROL 0b00011100

static void lcd_write(struct i2c_client *client, uint8_t cmd, bool rs_enabled) {
	printk("%i", cmd);
	cmd |= STATE | rs_enabled;
	i2c_smbus_write_byte(client, cmd);
	ndelay(PRE_ENABLE_DELAY);
	cmd |= ENABLE;
	i2c_smbus_write_byte(client, cmd);
	ndelay(ENABLE_PULSE_TIME);
	cmd ^= ENABLE;
	i2c_smbus_write_byte(client, cmd);
	ndelay(POST_ENABLE_DELAY);
	i2c_smbus_write_byte(client, STATE);
	ndelay(CYCLE_END_DELAY);
}

static void lcd_write_nibble(struct i2c_client *client, uint8_t nibble,
			     bool rs_enabled, bool is_high)
{
	uint8_t cmd = 0;
	if (is_high) {
		nibble &= 0b11110000;
	} else {
		nibble <<= 4;
	}
	cmd |= nibble;
	lcd_write(client, cmd, rs_enabled);
}
static void lcd_write_byte(struct i2c_client *client, uint8_t data, bool rs_enabled)
{
	lcd_write_nibble(client, data, rs_enabled, true);
	lcd_write_nibble(client, data, rs_enabled, false);
}

static void lcd_run_cmd(struct i2c_client *client, uint8_t data) {
	lcd_write_byte(client, data, false);
}

static void lcd_write_char(struct i2c_client *client, uint8_t data) {
	lcd_write_byte(client, data, true);
	fsleep(1000);
}
static void lcd_init(struct i2c_client *client)
{
	lcd_write(client, 0b00110000, false);
	fsleep(25000);
	lcd_write(client, 0b00110000, false);
	fsleep(5000);
	lcd_write(client, 0b00110000, false);
	fsleep(500);
	lcd_write(client, LCD_CMD_4_BIT_MODE, false);
	fsleep(8000);
	lcd_write_byte(client, 0b00100000, false);
	fsleep(5000);
	lcd_write_byte(client, 0b00001000, false);
	fsleep(5000);
	lcd_write_byte(client, 0b00000001, false);
	fsleep(5000);
	lcd_write_byte(client, 0b00000110, false);
	fsleep(5000);
}
static void lcd_clear(struct i2c_client *client) {
	lcd_run_cmd(client, LCD_CMD_CLEAR);
	fsleep(3000);
}
static void lcd_home(struct i2c_client *client) {
	lcd_run_cmd(client, LCD_CMD_HOME);
	fsleep(3000);
}
static void lcd_set_display_control(struct i2c_client *client) {
	lcd_run_cmd(client, LCD_CMD_DISPLAY_CONTROL);
	fsleep(3000);
}
static void lcd_set_entry_mode(struct i2c_client *client) {
	// TODO Entry mode options
	lcd_run_cmd(client, LCD_CMD_ENTRY_MODE | LCD_CMD_ENTRY_MODE_INCREMENT);
	fsleep(1000);
}

static void lcd_write_string(struct i2c_client *client, char *string) {
	for (char *c = string; *c != '\0'; c++) {
		lcd_write_char(client, (uint8_t)(*c));
	}
}

static int driver_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	dev_info(dev, "Probed.");

	struct device_data *data =
		devm_kzalloc(dev, sizeof(struct device_data), GFP_KERNEL);
	if (IS_ERR(data)) {
		return PTR_ERR(data);
	}

	data->i2c_client = client;

	lcd_init(client);
	// lcd_clear(client);
	// lcd_home(client);
	// lcd_set_entry_mode(client);
	lcd_set_display_control(client);
	lcd_write_string(client, "Sveikas, Lukai!");

	dev_set_drvdata(dev, data);

	return 0;
}

static void driver_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct device_data *data = dev_get_drvdata(dev);
	timer_shutdown_sync(&data->timer);
	lcd_clear(client);
	dev_info(dev, "Removed");
	return;
}

static struct i2c_driver g_lcd_driver = {
        .driver = {
                .name = "lcd-driver",
        },
        .id_table = g_lcd_driver_idtable,
        .probe = driver_probe,
        .remove = driver_remove,
};

int init_module()
{
	int result;
	result = i2c_add_driver(&g_lcd_driver);
	if (result) {
		return 1;
	}
	return 0;
}

void cleanup_module()
{
        i2c_del_driver(&g_lcd_driver);
	return;
}

MODULE_LICENSE("GPL");
