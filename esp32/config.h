#ifndef CONFIG_H
#define CONFIG_H

/**
 * Further configuration options for the ESP32.
 */

/** ----- Network settings ----- */
/** Device ID for identifying the ESP32 device on the network. */
const char *const device_id = "BiteBound-ESP32-S3-001";

/** Device hardware description. */
const char *const device_hardware = "Waveshare ESP32-S3 1.69inch";

/** Device firmware version. */
const char *const device_firmware_version = "1.0.0";

/** ----- MQTT settings ----- */
/** HiveMQ Cloud broker port. */
const int mqtt_port = 8883;

/** MQTT keep alive interval in seconds. */
const int mqtt_keep_alive_sec = 60;

/** MQTT topic for sending commands to the ESP32 device. */
const char *const mqtt_command_topic = "mauc2026/group_03/game/command";

/** MQTT topic for sending telemetry data from the ESP32 device. */
const char *const mqtt_telemetry_topic = "mauc2026/group_03/game/telemetry";

/** MQTT topic for sending test data from the ESP32 device. */
const char *const mqtt_test_topic = "mauc2026/group_03/test";

/** Quality of Service level for MQTT messages. */
const int mqtt_qos = 1;

/** Retain flag for MQTT messages. */
const bool mqtt_retain = false;

/** ----- Game settings ----- */
/** Default game duration in seconds. */
const int default_game_duration_sec = 120;

/** Default maximum number of cookies in the game. */
const int default_cookies_count = 10;

/** Default maximum number of visible cookies in the game. */
const int default_max_visible_cookies = 4;

/** ----- Display settings ----- */
/** Display width in pixels. */
const int display_width = 240;

/** Display height in pixels. */
const int display_height = 280;

/** Default wall thickness in pixels. */
const int default_wall_thickness_px = 6;

/** Top HUD header height in pixels. */
const int ui_header_height = 20;

/** Maximum number of cookies allowed to be rendered on the screen at once. */
const int max_rendered_cookies = 20;

/** ----- Color Palette (RGB565 Format) ----- */
const uint16_t color_background = 0x0000;  // Black
const uint16_t color_wall_type_1 = 0x7BEF; // Slate Grey
const uint16_t color_wall_type_2 = 0x001F; // Royal Blue
const uint16_t color_sphere = 0xF800;      // Vibrant Red
const uint16_t color_cookie = 0xFFE0;      // Gold / Yellow
const uint16_t color_ui_text = 0xFFFF;     // Crisp White

/** ----- Physics settings ----- */
/** IMU sensitivity multiplier (IMU = Inertial Measurement Units) */
const float default_imu_sensitivity_multiplier = 1.25f;

/** Bounce restitution coefficient.
 * This coefficient determines how much energy is conserved in a collision.
 * A value of 1 means a perfectly elastic collision (no energy loss), while
 * a value of 0 means a perfectly inelastic collision (objects stick together).
 * A value of 0.75 means that 75% of the energy is conserved in the collision,
 * which results in a realistic bounce effect for the ball in the game.
 * This value can be adjusted based on the desired game feel and physics behavior.
 */
const float default_bounce_restitution = 0.75f;

/** Exponential moving average alpha.
 * This is the smoothing factor for the exponential moving average filter applied to the sensor data.
 * A higher alpha gives more weight to recent data points, while a lower alpha gives more weight to older data points.
 * new_value = alpha * new_measurement + (1 - alpha) * old_value
 */
const float default_ema_alpha = 0.25f;

/** Deadzone threshold for sensor measurements.
 * This threshold defines the minimum change in sensor readings that will be considered significant.
 * If the change in sensor readings is below this threshold, it will be ignored to prevent noise from affecting the game.
 * This helps to create a more stable and enjoyable gaming experience by filtering out small, insignificant
 * movements of the device, e.g. when the player is holding the device still or making very slight movements.
 */
const float default_deadzone_threshold = 0.05f;

/** Maximum speed for the ball [px/s] to travel per second.*/
const float default_max_speed = 400.0f;

/** Continuous drag [1/s].
 * Default 0 means NO permanent damping, so the ball
 * never stalls on its own; energy is only lost on collisions.
 * Otherwise, a positive value will apply a continuous drag force to the ball,
 * gradually slowing it down over time.
 */
const float default_linear_damping = 0.0f;

/** Maximum number of contact resolution iterations per sub-step (corner safety).*/
const int default_max_contact_iterations = 4;

/** Standard earth gravity in m/s^2. */
const float default_earth_gravity_g = 1.00f;

/** ----- Time settings ----- */
/** NTP server address 1 for time synchronization. */
const char *const ntp_server_1 = "pool.ntp.org";
/** NTP server address 2 for time synchronization. */
const char *const ntp_server_2 = "time.nist.gov";
/** GMT offset in seconds (0 for UTC). */
const long gmt_offset_sec = 0;
/** Daylight saving time offset in seconds (0 if not used). */
const int daylight_offset_sec = 0;
/** Unix epoch time for January 1, 2020 (used as a threshold for time synchronization). */
const int time_epoch_2020_Jan_1 = 1577836800;

/** ----- Hardware Pins ----- */
/** I2C SDA Pin for IMU and Touch. */
const int pin_i2c_sda = 11;
/** I2C SCL Pin for IMU and Touch. */
const int pin_i2c_scl = 10;
/** ADC Pin for Battery reading. */
const int pin_battery_adc = 1;
/** Boot Button Pin. */
const int pin_boot_button = 0;
/** PWR Button Pin. SYS_OUT pulled up, pulled low when pressed; SYS_EN controls battery power hold.*/
const int pin_power_button = 40;

/** ----- Analog & Battery Settings ----- */
/** MCU ADC Reference Voltage. */
const float adc_voltage_reference = 3.3f;
/** ESP32-S3 12-bit ADC max value (2^12 - 1). */
const float adc_max_resolution = 4095.0f;
/** Voltage divider multiplier for battery reading (1:1 resistor configuration gives 2.0f). */
const float battery_voltage_multiplier = 2.0f;
/** Minimal simulated battery voltage. */
const float battery_min_voltage_mock = 3.0f;
/** Maximal simulated battery voltage. */
const float battery_max_voltage_mock = 4.2f;

#endif // CONFIG_H