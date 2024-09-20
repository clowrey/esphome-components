#include "axs15231_display.h"
#include "axs15231_defines.h"

#include "esphome/core/log.h"
#include "esphome/components/display/display_color_utils.h"

#ifdef USE_ESP_IDF

namespace esphome {
namespace axs15231 {

namespace {
  constexpr static const char *const TAG = "axs15231.display";

  /**
   * @brief LCD panel initialization commands.
   *
   */
  typedef struct {
      int cmd;                /*<! The specific LCD command */
      const void *data;       /*<! Buffer that holds the command specific data */
      size_t data_bytes;      /*<! Size of `data` in memory, in bytes */
      unsigned int delay_ms;  /*<! Delay in milliseconds after this command */
  } axs15231b_lcd_init_cmd_t;

  static const axs15231b_lcd_init_cmd_t vendor_specific_init_default[] = {
      {0xBB, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5}, 8, 0},
      {0xA0, (uint8_t[]){0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x64, 0x3F, 0x20, 0x05, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}, 17, 0},
      {0xA2, (uint8_t[]){0x30, 0x04, 0x0A, 0x3C, 0xEC, 0x54, 0xC4, 0x30, 0xAC, 0x28, 0x7F, 0x7F, 0x7F, 0x20, 0xF8, 0x10, 0x02, 0xFF, 0xFF, 0xF0, 0x90, 0x01, 0x32, 0xA0, 0x91, 0xC0, 0x20, 0x7F, 0xFF, 0x00, 0x54}, 31, 0},
      {0xD0, (uint8_t[]){0x30, 0xAC, 0x21, 0x24, 0x08, 0x09, 0x10, 0x01, 0xAA, 0x14, 0xC2, 0x00, 0x22, 0x22, 0xAA, 0x03, 0x10, 0x12, 0x40, 0x14, 0x1E, 0x51, 0x15, 0x00, 0x40, 0x10, 0x00, 0x03, 0x3D, 0x12}, 30, 0},
      {0xA3, (uint8_t[]){0xA0, 0x06, 0xAA, 0x08, 0x08, 0x02, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55}, 22, 0},
      {0xC1, (uint8_t[]){0x33, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00, 0x41, 0x00, 0x53, 0xFF, 0xFF, 0xFF, 0x4F, 0x52, 0x00, 0x4F, 0x52, 0x00, 0x45, 0x3B, 0x0B, 0x02, 0x0D, 0x00, 0xFF, 0x40}, 30, 0},
      {0xC3, (uint8_t[]){0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01}, 11, 0},
      {0xC4, (uint8_t[]){0x00, 0x24, 0x33, 0x90, 0x50, 0xea, 0x64, 0x32, 0xC8, 0x64, 0xC8, 0x32, 0x90, 0x90, 0x11, 0x06, 0xDC, 0xFA, 0x04, 0x03, 0x80, 0xFE, 0x10, 0x10, 0x00, 0x0A, 0x0A, 0x44, 0x50}, 29, 0},
      {0xC5, (uint8_t[]){0x18, 0x00, 0x00, 0x03, 0xFE, 0x78, 0x33, 0x20, 0x30, 0x10, 0x88, 0xDE, 0x0D, 0x08, 0x0F, 0x0F, 0x01, 0x78, 0x33, 0x20, 0x10, 0x10, 0x80}, 23, 0},
      {0xC6, (uint8_t[]){0x05, 0x0A, 0x05, 0x0A, 0x00, 0xE0, 0x2E, 0x0B, 0x12, 0x22, 0x12, 0x22, 0x01, 0x00, 0x00, 0x3F, 0x6A, 0x18, 0xC8, 0x22}, 20, 0},
      {0xC7, (uint8_t[]){0x50, 0x32, 0x28, 0x00, 0xa2, 0x80, 0x8f, 0x00, 0x80, 0xff, 0x07, 0x11, 0x9F, 0x6f, 0xff, 0x26, 0x0c, 0x0d, 0x0e, 0x0f}, 20, 0},
      {0xC9, (uint8_t[]){0x33, 0x44, 0x44, 0x01}, 4, 0},
      {0xCF, (uint8_t[]){0x34, 0x1E, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1E, 0x68, 0xF7, 0x00, 0x65, 0x0C, 0x22, 0xC4, 0x0C, 0x77, 0x22, 0x44, 0xAA, 0x55, 0x04, 0x04, 0x12, 0xA0, 0x08}, 27, 0},
      {0xD5, (uint8_t[]){0x3E, 0x3E, 0x88, 0x00, 0x44, 0x04, 0x78, 0x33, 0x20, 0x78, 0x33, 0x20, 0x04, 0x28, 0xD3, 0x47, 0x03, 0x03, 0x03, 0x03, 0x86, 0x00, 0x00, 0x00, 0x30, 0x52, 0x3f, 0x40, 0x40, 0x96}, 30, 0},
      {0xD6, (uint8_t[]){0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE, 0x95, 0x00, 0x01, 0x83, 0x75, 0x36, 0x20, 0x75, 0x36, 0x20, 0x3F, 0x03, 0x03, 0x03, 0x10, 0x10, 0x00, 0x04, 0x51, 0x20, 0x01, 0x00}, 30, 0},
      {0xD7, (uint8_t[]){0x0a, 0x08, 0x0e, 0x0c, 0x1E, 0x18, 0x19, 0x1F, 0x00, 0x1F, 0x1A, 0x1F, 0x3E, 0x3E, 0x04, 0x00, 0x1F, 0x1F, 0x1F}, 19, 0},
      {0xD8, (uint8_t[]){0x0B, 0x09, 0x0F, 0x0D, 0x1E, 0x18, 0x19, 0x1F, 0x01, 0x1F, 0x1A, 0x1F}, 12, 0},
      {0xD9, (uint8_t[]){0x00, 0x0D, 0x0F, 0x09, 0x0B, 0x1F, 0x18, 0x19, 0x1F, 0x01, 0x1E, 0x1A, 0x1F}, 13, 0},
      {0xDD, (uint8_t[]){0x0C, 0x0E, 0x08, 0x0A, 0x1F, 0x18, 0x19, 0x1F, 0x00, 0x1E, 0x1A, 0x1F}, 12, 0},
      {0xDF, (uint8_t[]){0x44, 0x73, 0x4B, 0x69, 0x00, 0x0A, 0x02, 0x90}, 8, 0},
      {0xE0, (uint8_t[]){0x19, 0x20, 0x0A, 0x13, 0x0E, 0x09, 0x12, 0x28, 0xD4, 0x24, 0x0C, 0x35, 0x13, 0x31, 0x36, 0x2f, 0x03}, 17, 0},
      {0xE1, (uint8_t[]){0x38, 0x20, 0x09, 0x12, 0x0E, 0x08, 0x12, 0x28, 0xC5, 0x24, 0x0C, 0x34, 0x12, 0x31, 0x36, 0x2f, 0x27}, 17, 0},
      {0xE2, (uint8_t[]){0x19, 0x20, 0x0A, 0x11, 0x09, 0x06, 0x11, 0x25, 0xD4, 0x22, 0x0B, 0x33, 0x12, 0x2D, 0x32, 0x2f, 0x03}, 17, 0},
      {0xE3, (uint8_t[]){0x38, 0x20, 0x0A, 0x11, 0x09, 0x06, 0x11, 0x25, 0xC4, 0x21, 0x0A, 0x32, 0x11, 0x2C, 0x32, 0x2f, 0x27}, 17, 0},
      {0xE4, (uint8_t[]){0x19, 0x20, 0x0D, 0x14, 0x0D, 0x08, 0x12, 0x2A, 0xD4, 0x26, 0x0E, 0x35, 0x13, 0x34, 0x39, 0x2f, 0x03}, 17, 0},
      {0xE5, (uint8_t[]){0x38, 0x20, 0x0D, 0x13, 0x0D, 0x07, 0x12, 0x29, 0xC4, 0x25, 0x0D, 0x35, 0x12, 0x33, 0x39, 0x2f, 0x27}, 17, 0},
      {0xBB, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0},
      {0x13, (uint8_t[]){0x00}, 0, 0},
      {0x11, (uint8_t[]){0x00}, 0, 200},
      {0x29, (uint8_t[]){0x00}, 0, 200},
      {0x2C, (uint8_t[]){0x00, 0x00, 0x00, 0x00}, 4, 0},
      {0x22, (uint8_t[]){0x00}, 0, 200},//All Pixels off
  };

  typedef struct
  {
      uint8_t cmd;
      uint8_t data[36];
      uint8_t len;
  } lcd_cmd_t;

  const static lcd_cmd_t AXS_QSPI_INIT_NEW[] = {
    {AXS_LCD_DISPOFF, {0x00}, 0x40},
    {AXS_LCD_SLPIN,   {0x00}, 0x80},
    {AXS_LCD_SLPOUT,  {0x00}, 0x80},
    {AXS_LCD_DISPON,  {0x00}, 0x00},
  };

  // store a 16 bit value in a buffer, big endian.
  static inline void put16_be(uint8_t *buf, uint16_t value) {
    buf[0] = value >> 8;
    buf[1] = value;
  }
}  // anonymous namespace

void AXS15231Display::update() {
  if (this->prossing_update_) {
    this->need_update_ = true;
    return;
  }

  this->prossing_update_ = true;
  do {
    this->need_update_ = false;
    this->do_update_();
  } while (this->need_update_);
  this->prossing_update_ = false;
  this->display_();
}

float AXS15231Display::get_setup_priority() const {
  return setup_priority::HARDWARE;
}

void AXS15231Display::setup() {
  ESP_LOGCONFIG(TAG, "setting up axs15231");

  ESP_LOGI(TAG, "setup pins");
  this->setup_pins_();
  ESP_LOGI(TAG, "setup lcd");
  this->init_lcd_();

  ESP_LOGI(TAG, "set madctl");
  this->set_madctl_();
  this->invalidate_();

  ESP_LOGI(TAG, "init internal buffer");
  this->init_internal_(this->get_buffer_length_());
  if (this->buffer_ == nullptr) {
    this->mark_failed();
  }

  ESP_LOGI(TAG, "set brightness");
  this->write_command_(AXS_LCD_WRDISBV, &this->brightness_, 1);

  this->setup_complete_ = true;
  ESP_LOGCONFIG(TAG, "axs15231 setup complete");
}

bool AXS15231Display::can_proceed() {
  return this->setup_complete_;
}

void AXS15231Display::dump_config() {
  ESP_LOGCONFIG("", "AXS15231 Display");
  ESP_LOGCONFIG(TAG, "  Height: %u", this->height_);
  ESP_LOGCONFIG(TAG, "  Width: %u", this->width_);
  LOG_PIN("  CS Pin: ", this->cs_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGCONFIG(TAG, "  SPI Data rate: %dMHz", (unsigned) (this->data_rate_ / 1000000));
#ifdef USE_POWER_SUPPLY
  ESP_LOGCONFIG(TAG, "  Power Supply Configured: yes");
#endif
}

void AXS15231Display::fill(Color color) {
  uint16_t new_color = 0;
  this->x_low_ = 0;
  this->y_low_ = 0;
  this->x_high_ = this->get_width_internal() - 1;
  this->y_high_ = this->get_height_internal() - 1;

  new_color = display::ColorUtil::color_to_565(color);
  if (((uint8_t) (new_color >> 8)) == ((uint8_t) new_color)) {
    // Upper and lower is equal can use quicker memset operation
    memset(this->buffer_, (uint8_t) new_color, this->get_buffer_length_());
  } else {
    // Slower set of both buffers
    for (uint32_t i = 0; i < this->get_buffer_length_(); i = i + 2) {
      this->buffer_[i] = (uint8_t) (new_color >> 8);
      this->buffer_[i + 1] = (uint8_t) new_color;
    }
  }
}

display::DisplayType AXS15231Display::get_display_type() {
  return display::DisplayType::DISPLAY_TYPE_COLOR;
}

uint32_t AXS15231Display::get_buffer_length_() {
  return this->get_width_internal() * this->get_height_internal() * 2;
}

int AXS15231Display::get_width_internal() {
  return this->width_;
}

int AXS15231Display::get_height_internal() {
  return this->height_;
}

void AXS15231Display::set_reset_pin(GPIOPin *reset_pin) {
  this->reset_pin_ = reset_pin;
}

void AXS15231Display::set_backlight_pin(GPIOPin *backlight_pin) {
  this->backlight_pin_ = backlight_pin;
}

void AXS15231Display::set_width(uint16_t width) {
  this->width_ = width;
}

void AXS15231Display::set_dimensions(uint16_t width, uint16_t height) {
  this->width_ = width;
  this->height_ = height;
}

void AXS15231Display::set_mirror_x(bool mirror_x) {
  this->mirror_x_ = mirror_x;
}

void AXS15231Display::set_mirror_y(bool mirror_y) {
  this->mirror_y_ = mirror_y;
}

void AXS15231Display::set_swap_xy(bool swap_xy) {
  this->swap_xy_ = swap_xy;
}

void AXS15231Display::set_brightness(uint8_t brightness) {
  this->brightness_ = brightness;

  if (this->setup_complete_) {
    this->write_command_(AXS_LCD_WRDISBV, &this->brightness_, 1);
  }
}

void AXS15231Display::set_offsets(int16_t offset_x, int16_t offset_y) {
  this->offset_x_ = offset_x;
  this->offset_y_ = offset_y;
}

void AXS15231Display::setup_pins_() {
  if (this->backlight_pin_ != nullptr) {
    this->backlight_pin_->setup();
    this->backlight_pin_->digital_write(true);
  }

  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
  }

  this->spi_setup();

  this->reset_();
}

void AXS15231Display::set_madctl_() {
// custom x/y transform and color order
  uint8_t mad = MADCTL_RGB;
  // TODO(buglloc): MADCTL_MV is broken
  if (this->swap_xy_)
    mad |= MADCTL_MV;
  if (this->mirror_x_)
    mad |= MADCTL_MX;
  if (this->mirror_y_)
    mad |= MADCTL_MY;

  this->write_command_(AXS_LCD_MADCTL, &mad, 1);
  ESP_LOGD(TAG, "wrote MADCTL 0x%02X", mad);
}

void AXS15231Display::init_lcd_() {
  const axs15231b_lcd_init_cmd_t *lcd_init = vendor_specific_init_default;
  for (int i = 0; i < sizeof(vendor_specific_init_default) / sizeof(axs15231b_lcd_init_cmd_t); ++i) {
    this->write_command_(lcd_init[i].cmd, (uint8_t *)lcd_init[i].data, lcd_init[i].len);
    if (lcd_init[i].delay_ms)
        delay(lcd_init[i].delay_ms);
  }
//  const lcd_cmd_t *lcd_init = AXS_QSPI_INIT_NEW;
//  for (int i = 0; i < sizeof(AXS_QSPI_INIT_NEW) / sizeof(lcd_cmd_t); ++i) {
//    this->write_command_(lcd_init[i].cmd, (uint8_t *)lcd_init[i].data, lcd_init[i].len & 0x3f);
//    if (lcd_init[i].len & 0x80)
//        delay(150);
//    if (lcd_init[i].len & 0x40)
//        delay(20);
//  }
}

void AXS15231Display::reset_() {
  if (this->reset_pin_  == nullptr) {
    return;
  }

  this->reset_pin_->digital_write(true);
  delay(20);
  this->reset_pin_->digital_write(false);
  delay(20);
  this->reset_pin_->digital_write(true);
  delay(20);
}

void AXS15231Display::write_command_(uint8_t cmd, const uint8_t *bytes, size_t len) {
  this->enable();
  this->write_cmd_addr_data(8, 0x02, 24, cmd << 8, bytes, len);
  this->disable();
}

void AXS15231Display::write_command_(uint8_t cmd, uint8_t data) { this->write_command_(cmd, &data, 1); }

void AXS15231Display::write_command_(uint8_t cmd) { this->write_command_(cmd, &cmd, 0); }

void AXS15231Display::set_addr_window_(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  uint8_t buf[4];
  x1 += this->offset_x_;
  x2 += this->offset_x_;
  y1 += this->offset_y_;
  y2 += this->offset_y_;
  put16_be(buf, x1);
  put16_be(buf + 2, x2);
  this->write_command_(AXS_LCD_CASET, buf, sizeof(buf));
  put16_be(buf, y1);
  put16_be(buf + 2, y2);
  this->write_command_(AXS_LCD_RASET, buf, sizeof(buf));
}

void AXS15231Display::display_() {
  if ((this->x_high_ < this->x_low_) || (this->y_high_ < this->y_low_)) {
    return;
  }

  // we will only update the changed rows to the display
  size_t const w = this->x_high_ - this->x_low_ + 1;
  size_t const h = this->y_high_ - this->y_low_ + 1;
  size_t const x_pad = this->get_width_internal() - w - this->x_low_;
  this->set_addr_window_(this->x_low_, this->y_low_, this->x_high_, this->y_high_);

  this->enable();

  if (this->x_low_ == 0 && this->y_low_ == 0 && x_pad == 0) {
    this->write_cmd_addr_data(8, 0x32, 24, 0x2C00, this->buffer_, w * h * 2, 4);
  } else {
    this->write_cmd_addr_data(8, 0x32, 24, 0x2C00, nullptr, 0, 4);
    size_t stride = this->x_low_ + w + x_pad;
    for (int y = 0; y != h; y++) {
      size_t offset = ((y + this->y_low_) * stride + this->x_low_);
      this->write_cmd_addr_data(0, 0, 0, 0, this->buffer_ + offset * 2, w * 2, 4);
    }
  }

  this->disable();

  this->invalidate_();
}

void AXS15231Display::invalidate_() {
  // invalidate watermarks
  this->x_low_ = this->width_;
  this->y_low_ = this->height_;
  this->x_high_ = 0;
  this->y_high_ = 0;
}

void AXS15231Display::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || x >= this->get_width_internal() || y < 0 || y >= this->get_height_internal()) {
    ESP_LOGW(TAG, "tring to draw invalid pixel: x(0 <= %d < %d) && y(0 <= %d < %d)", x, this->get_width_internal(), y,
               this->get_height_internal());
    return;
  }

  uint32_t pos = (y * width_) + x;
  uint16_t new_color;
  bool updated = false;

  pos = pos * 2;
  new_color = display::ColorUtil::color_to_565(color, display::ColorOrder::COLOR_ORDER_RGB);
  if (this->buffer_[pos] != (uint8_t) (new_color >> 8)) {
    this->buffer_[pos] = (uint8_t) (new_color >> 8);
    updated = true;
  }
  pos = pos + 1;
  new_color = new_color & 0xFF;

  if (this->buffer_[pos] != new_color) {
    this->buffer_[pos] = new_color;
    updated = true;
  }

  if (updated) {
    // low and high watermark may speed up drawing from buffer
    if (x < this->x_low_)
      this->x_low_ = x;
    if (y < this->y_low_)
      this->y_low_ = y;
    if (x > this->x_high_)
      this->x_high_ = x;
    if (y > this->y_high_)
      this->y_high_ = y;
  }
}

#endif
}  // namespace axs15231
}  // namespace esphome
