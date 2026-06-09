/**
 * Further configuration options for the ESP32.
 */

/** Network settings */
/** Device ID for identifying the ESP32 device on the network. */
const char *device_id = "BiteBound-ESP32-S3-001";

/** Game settings */
/** Default game duration in seconds. */
const int default_game_duration_sec = 120;

/** Default maximum number of cookies in the game. */
const int default_cookies_count = 10;

/** Display settings */
/** Display width in pixels. */
const int display_width = 240;

/** Display height in pixels. */
const int display_height = 280;

/** Default wall thickness in pixels. */
const int default_wall_thickness_px = 6;

/** Physics settings */
/** IMU sensitivity multiplier (IMU = Inertial Measurement Units) */
const float default_imu_sensitivity_multiplier = 1.25;

/** Bounce restitution coefficient.
 * This coefficient determines how much energy is conserved in a collision.
 * A value of 1 means a perfectly elastic collision (no energy loss), while
 * a value of 0 means a perfectly inelastic collision (objects stick together).
 * A value of 0.75 means that 75% of the energy is conserved in the collision,
 * which results in a realistic bounce effect for the ball in the game.
 * This value can be adjusted based on the desired game feel and physics behavior.
 */
const float default_bounce_restitution = 0.75;

/** Exponential moving average alpha.
 * This is the smoothing factor for the exponential moving average filter applied to the sensor data.
 * A higher alpha gives more weight to recent data points, while a lower alpha gives more weight to older data points.
 * new_value = alpha * new_measurement + (1 - alpha) * old_value
 */
const float default_ema_alpha = 0.25;

/** Deadzone threshold for sensor measurements.
 * This threshold defines the minimum change in sensor readings that will be considered significant.
 * If the change in sensor readings is below this threshold, it will be ignored to prevent noise from affecting the game.
 * This helps to create a more stable and enjoyable gaming experience by filtering out small, insignificant
 * movements of the device, e.g. when the player is holding the device still or making very slight movements.
 */
const float default_deadzone_threshold = 0.05;

/** Time settings */
/** NTP server address 1 for time synchronization. */
const char *ntp_server_1 = "pool.ntp.org";
/** NTP server address 2 for time synchronization. */
const char *ntp_server_2 = "time.nist.gov";
/** GMT offset in seconds (0 for UTC). */
const long gmt_offset_sec = 0;
/** Daylight saving time offset in seconds (0 if not used). */
const int daylight_offset_sec = 0;
/** Unix epoch time for January 1, 2020 (used as a threshold for time synchronization). */
const int time_epoch_2020_Jan_1 = 1577836800;