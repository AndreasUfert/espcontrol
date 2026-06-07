#pragma once

// Internal implementation detail for button_grid.h. Include button_grid.h from device YAML.

// ── Fan card controls ─────────────────────────────────────────────────

constexpr int FAN_PRESET_MAX_OPTIONS = 32;

struct FanCardCtx {
  std::string type;
  std::string entity_id;
  std::string label;
  std::string friendly_name;
  std::string state;
  std::string direction;
  std::string preset_mode;
  std::vector<std::string> preset_modes;
  lv_obj_t *btn = nullptr;
  lv_obj_t *icon_lbl = nullptr;
  TransientStatusLabel *status_label = nullptr;
  const char *icon_off_glyph = nullptr;
  const char *icon_on_glyph = nullptr;
  uint32_t on_color = DEFAULT_SLIDER_COLOR;
  uint32_t off_color = DEFAULT_OFF_COLOR;
  uint32_t tertiary_color = DEFAULT_TERTIARY_COLOR;
  const lv_font_t *label_font = nullptr;
  const lv_font_t *icon_font = nullptr;
  int width_compensation_percent = 100;
  bool available = false;
  bool on = false;
  bool oscillating = false;
  bool oscillation_known = false;
  bool direction_known = false;
  bool has_custom_label = false;
  int percentage = 0;
  bool percentage_known = false;
  int percentage_step = 10;
  bool percentage_step_known = false;
  std::string label_display;
};

struct FanPresetClick {
  FanCardCtx *ctx = nullptr;
  std::string mode;
};

struct FanPresetUi {
  lv_obj_t *overlay = nullptr;
  lv_obj_t *panel = nullptr;
  lv_obj_t *close_btn = nullptr;
  lv_obj_t *title_lbl = nullptr;
  lv_obj_t *list = nullptr;
  lv_obj_t *empty_lbl = nullptr;
  FanCardCtx *active = nullptr;
  FanPresetClick option_clicks[FAN_PRESET_MAX_OPTIONS];
};

inline FanPresetUi &fan_preset_ui() {
  static FanPresetUi ui;
  return ui;
}

struct FanControlPanelUi {
  lv_obj_t *overlay = nullptr;
  lv_obj_t *panel = nullptr;
  lv_obj_t *close_btn = nullptr;
  lv_obj_t *title_lbl = nullptr;
  lv_obj_t *list = nullptr;
  lv_obj_t *onoff_btn = nullptr;
  lv_obj_t *speed_slider = nullptr;
  lv_obj_t *oscillate_row = nullptr;
  lv_obj_t *direction_row = nullptr;
  lv_obj_t *preset_chip = nullptr;
  lv_obj_t *preset_chip_lbl = nullptr;
  lv_obj_t *menu_overlay = nullptr;
  FanPresetClick preset_menu_clicks[FAN_PRESET_MAX_OPTIONS] = {};
  int preset_menu_click_count = 0;
  lv_coord_t content_w = 0;
  FanCardCtx *active = nullptr;
};

inline FanControlPanelUi &fan_control_panel_ui() {
  static FanControlPanelUi ui;
  return ui;
}

inline bool fan_non_speed_card_type(const std::string &type) {
  return type == "fan" ||
         type == "fan_switch" ||
         type == "fan_oscillate" ||
         type == "fan_direction" ||
         type == "fan_preset";
}

inline const char *fan_card_icon_name(const ParsedCfg &p) {
  if (!p.icon.empty() && p.icon != "Auto") return p.icon.c_str();
  return fan_card_default_icon_name(p.type);
}

inline const char *fan_card_icon_on_name(const ParsedCfg &p) {
  if (!p.icon_on.empty() && p.icon_on != "Auto") return p.icon_on.c_str();
  return p.type == "fan_switch" ? "Fan" : fan_card_icon_name(p);
}

inline std::string fan_card_label(const ParsedCfg &p) {
  if (!p.label.empty()) return p.label;
  if (p.type == "fan_switch") return espcontrol_i18n(std::string("Fan"));
  if (p.type == "fan_oscillate") return espcontrol_i18n(std::string("Oscillation"));
  if (p.type == "fan_direction") return espcontrol_i18n(std::string("Direction"));
  if (p.type == "fan_preset") return espcontrol_i18n(std::string("Preset"));
  return espcontrol_i18n(std::string("Fan"));
}

inline void setup_fan_card(BtnSlot &s, const ParsedCfg &p) {
  lv_label_set_text(s.icon_lbl, find_icon(fan_card_icon_name(p)));
  lv_label_set_text(s.text_lbl, fan_card_label(p).c_str());
  if (p.type != "fan_switch") apply_push_button_transition(s.btn);
}

inline std::string fan_trim(const std::string &value) {
  size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) start++;
  size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) end--;
  return value.substr(start, end - start);
}

inline std::string fan_lower(const std::string &value) {
  std::string out = value;
  for (char &c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return out;
}

inline bool fan_parse_bool_value(const std::string &value, bool &out) {
  std::string lower = fan_lower(fan_trim(value));
  if (lower == "true" || lower == "on" || lower == "yes" || lower == "1") {
    out = true;
    return true;
  }
  if (lower == "false" || lower == "off" || lower == "no" || lower == "0") {
    out = false;
    return true;
  }
  return false;
}

inline bool fan_token_is_header(const std::string &value) {
  std::string lower = fan_lower(value);
  return lower == "presetmode" || lower == "presetmodes" ||
         lower == "direction" || lower == "oscillating";
}

inline std::vector<std::string> fan_parse_options(esphome::StringRef value) {
  std::string raw = string_ref_limited(value, HA_TEXT_SENSOR_STATE_MAX_LEN);
  std::vector<std::string> out;
  std::string cur;
  bool quoted = false;
  char quote_char = 0;
  for (char ch : raw) {
    if ((ch == '\'' || ch == '"')) {
      if (quoted && ch == quote_char) {
        quoted = false;
        quote_char = 0;
      } else if (!quoted) {
        quoted = true;
        quote_char = ch;
      } else {
        cur.push_back(ch);
      }
      continue;
    }
    if (!quoted && (ch == '[' || ch == ']')) continue;
    if (!quoted && ch == ',') {
      std::string item = fan_trim(cur);
      if (!item.empty() && !fan_token_is_header(item)) {
        std::string lower = fan_lower(item);
        bool duplicate = false;
        for (const auto &existing : out) {
          if (fan_lower(existing) == lower) {
            duplicate = true;
            break;
          }
        }
        if (!duplicate) out.push_back(item);
      }
      cur.clear();
      continue;
    }
    cur.push_back(ch);
  }
  std::string item = fan_trim(cur);
  if (!item.empty() && !fan_token_is_header(item)) {
    std::string lower = fan_lower(item);
    bool duplicate = false;
    for (const auto &existing : out) {
      if (fan_lower(existing) == lower) {
        duplicate = true;
        break;
      }
    }
    if (!duplicate) out.push_back(item);
  }
  return out;
}

inline std::string fan_option_label(const std::string &value) {
  if (value.empty()) return espcontrol_i18n(std::string("None"));
  return espcontrol_i18n(sentence_cap_text(value));
}

inline bool fan_preset_active(const std::string &value) {
  std::string lower = fan_lower(fan_trim(value));
  return !lower.empty() && lower != "none" && lower != "off";
}

inline bool fan_control_supported(FanCardCtx *ctx) {
  if (!ctx || !ctx->available) return false;
  if (ctx->type == "fan") return true;
  if (ctx->type == "fan_oscillate") return ctx->oscillation_known;
  if (ctx->type == "fan_direction") return ctx->direction_known;
  if (ctx->type == "fan_preset") return !ctx->preset_modes.empty();
  return true;
}

inline void fan_apply_card_visual(FanCardCtx *ctx) {
  if (!ctx || !ctx->btn) return;
  bool supported = fan_control_supported(ctx);
  apply_control_availability(ctx->btn, ctx->btn, supported);

  bool active = false;
  if (ctx->type == "fan_switch" || ctx->type == "fan") active = ctx->on;
  else if (ctx->type == "fan_oscillate") active = ctx->oscillating;
  else if (ctx->type == "fan_direction") active = ctx->direction == "reverse";
  else if (ctx->type == "fan_preset") active = fan_preset_active(ctx->preset_mode);

  set_card_checked_state(ctx->btn, active);

  if (ctx->icon_lbl) {
    if ((ctx->type == "fan_switch" || ctx->type == "fan") && active && ctx->icon_on_glyph) {
      lv_label_set_text(ctx->icon_lbl, ctx->icon_on_glyph);
    } else if (ctx->icon_off_glyph) {
      lv_label_set_text(ctx->icon_lbl, ctx->icon_off_glyph);
    }
  }
}

inline std::string fan_status_text(FanCardCtx *ctx) {
  if (!ctx || !ctx->available) return espcontrol_i18n(std::string("Unavailable"));
  if (ctx->type == "fan_switch") {
    return ctx->on ? espcontrol_i18n(std::string("On")) : espcontrol_i18n(std::string("Off"));
  }
  if (ctx->type == "fan_oscillate") {
    if (!ctx->oscillation_known) return espcontrol_i18n(std::string("Unsupported"));
    return ctx->oscillating ? espcontrol_i18n(std::string("Oscillating")) : espcontrol_i18n(std::string("Still"));
  }
  if (ctx->type == "fan_direction") {
    if (!ctx->direction_known) return espcontrol_i18n(std::string("Unsupported"));
    return fan_option_label(ctx->direction);
  }
  if (ctx->type == "fan_preset") {
    if (ctx->preset_modes.empty()) return espcontrol_i18n(std::string("Unsupported"));
    return fan_preset_active(ctx->preset_mode) ? fan_option_label(ctx->preset_mode) : espcontrol_i18n(std::string("Preset"));
  }
  if (ctx->type == "fan") {
    if (!ctx->on) return espcontrol_i18n(std::string("Off"));
    if (ctx->percentage_known) return std::to_string(ctx->percentage) + "%";
    return espcontrol_i18n(std::string("On"));
  }
  return espcontrol_i18n(std::string("Fan"));
}

inline void fan_control_panel_close() {
  FanControlPanelUi &ui = fan_control_panel_ui();
  control_modal_close_nested_menu();
  control_modal_delete_overlay(ControlModalKind::FAN_CONTROL, ui.overlay);
  ui = FanControlPanelUi();
}

inline void fan_control_preset_menu_close() {
  FanControlPanelUi &ui = fan_control_panel_ui();
  control_modal_delete_nested_overlay(ui.menu_overlay);
}

inline void fan_control_panel_update(FanCardCtx *ctx) {
  FanControlPanelUi &ui = fan_control_panel_ui();
  if (ui.active != ctx) return;
  if (ui.onoff_btn)
    lv_obj_set_style_bg_color(ui.onoff_btn,
      lv_color_hex(ctx->on ? ctx->on_color : (uint32_t)DARK_BACKGROUND_SECONDARY), LV_PART_MAIN);
  if (ui.speed_slider && !(lv_obj_get_state(ui.speed_slider) & LV_STATE_PRESSED)) {
    int val = (ctx->on && ctx->percentage_known && ctx->percentage > 0) ? ctx->percentage : 1;
    lv_slider_set_value(ui.speed_slider, val, LV_ANIM_OFF);
    lv_obj_set_style_bg_opa(ui.speed_slider, ctx->on ? LV_OPA_COVER : LV_OPA_40,
      static_cast<lv_style_selector_t>(LV_PART_INDICATOR));
  }
  if (ui.oscillate_row)
    lv_obj_set_style_bg_color(ui.oscillate_row,
      lv_color_hex(ctx->oscillating ? ctx->on_color : (uint32_t)DARK_BACKGROUND_SECONDARY), LV_PART_MAIN);
  if (ui.direction_row)
    lv_obj_set_style_bg_color(ui.direction_row,
      lv_color_hex(ctx->direction == "reverse" ? ctx->on_color : (uint32_t)DARK_BACKGROUND_SECONDARY), LV_PART_MAIN);
  if (ui.preset_chip_lbl) {
    bool preset_active = fan_preset_active(ctx->preset_mode);
    std::string text = preset_active
      ? fan_option_label(ctx->preset_mode) : espcontrol_i18n(std::string("Preset"));
    lv_label_set_text(ui.preset_chip_lbl, text.c_str());
  }
}

inline void fan_refresh_card(FanCardCtx *ctx) {
  if (!ctx) return;
  fan_apply_card_visual(ctx);
  if (ctx->type == "fan") fan_control_panel_update(ctx);
  bool persistent = ctx->type == "fan_direction" || ctx->type == "fan_preset" ||
                    (ctx->type == "fan" && ctx->label_display == "status");
  transient_status_label_show_if_changed(ctx->status_label, fan_status_text(ctx), !persistent);
}

inline void send_fan_action(const std::string &entity_id,
                            const char *service,
                            const char *data_key = nullptr,
                            const char *data_value = nullptr) {
  ha_send_entity_action(entity_id, service, data_key, data_value);
}

inline void send_fan_switch_action(FanCardCtx *ctx) {
  if (!ctx) return;
  send_fan_action(ctx->entity_id, ctx->on ? "fan.turn_off" : "fan.turn_on");
}

inline void send_fan_oscillate_action(FanCardCtx *ctx) {
  if (!ctx) return;
  send_fan_action(ctx->entity_id, "fan.oscillate", "oscillating", ctx->oscillating ? "false" : "true");
}

inline void send_fan_direction_action(FanCardCtx *ctx) {
  if (!ctx) return;
  const char *next = ctx->direction == "reverse" ? "forward" : "reverse";
  send_fan_action(ctx->entity_id, "fan.set_direction", "direction", next);
}

inline void send_fan_preset_action(FanCardCtx *ctx, const std::string &mode) {
  if (!ctx || mode.empty()) return;
  send_fan_action(ctx->entity_id, "fan.set_preset_mode", "preset_mode", mode.c_str());
}

inline void fan_control_preset_menu_open() {
  FanControlPanelUi &ui = fan_control_panel_ui();
  FanCardCtx *ctx = ui.active;
  if (!ctx || ctx->preset_modes.empty()) return;

  ui.preset_menu_click_count = 0;
  lv_coord_t menu_w = ui.content_w * 55 / 100;
  if (menu_w < 160) menu_w = 160;
  ControlModalNestedShell shell = control_modal_open_nested_menu(
    menu_w, 14, fan_control_preset_menu_close);
  ui.menu_overlay = shell.overlay;
  lv_obj_t *box = shell.panel;
  lv_obj_set_style_pad_row(box, 0, LV_PART_MAIN);

  lv_coord_t option_h = ctx->label_font ? ctx->label_font->line_height + 24 : 60;
  if (option_h < 48) option_h = 48;

  for (const auto &mode : ctx->preset_modes) {
    if (ui.preset_menu_click_count >= FAN_PRESET_MAX_OPTIONS) break;
    bool selected = (mode == ctx->preset_mode);
    ui.preset_menu_clicks[ui.preset_menu_click_count] = {ctx, mode};
    lv_obj_t *btn = lv_btn_create(box);
    lv_obj_set_width(btn, lv_pct(100));
    lv_obj_set_height(btn, option_h);
    lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(btn, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(btn, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_left(btn, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_right(btn, 12, LV_PART_MAIN);
    control_modal_apply_pressed_fill(btn);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, fan_option_label(mode).c_str());
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(label,
      lv_color_hex(selected ? ctx->on_color : (uint32_t)DARK_TEXT_SOFT), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    if (ctx->label_font) lv_obj_set_style_text_font(label, ctx->label_font, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
      FanPresetClick *click = (FanPresetClick *)lv_event_get_user_data(e);
      if (click && click->ctx) send_fan_preset_action(click->ctx, click->mode);
      fan_control_preset_menu_close();
    }, LV_EVENT_CLICKED, &ui.preset_menu_clicks[ui.preset_menu_click_count]);
    ui.preset_menu_click_count++;
  }
}

inline void fan_preset_close() {
  FanPresetUi &ui = fan_preset_ui();
  control_modal_delete_overlay(ControlModalKind::FAN_PRESET, ui.overlay);
  ui = FanPresetUi();
}

inline void fan_preset_close_if_unsupported(FanCardCtx *ctx) {
  FanPresetUi &ui = fan_preset_ui();
  if (ui.active == ctx && !fan_control_supported(ctx)) fan_preset_close();
}

inline void fan_preset_open(FanCardCtx *ctx) {
  if (!fan_control_supported(ctx)) return;
  ControlModalShell shell = control_modal_open_shell(
    ControlModalKind::FAN_PRESET, ctx->btn, ctx->width_compensation_percent,
    ctx->icon_font, "\U000F0156", true, fan_preset_close);
  FanPresetUi &ui = fan_preset_ui();
  ui.active = ctx;
  ui.overlay = shell.overlay;
  ui.panel = shell.panel;
  ui.close_btn = shell.close_btn;

  ControlModalLayout &layout = shell.layout;
  lv_coord_t content_w = shell.content_w;
  lv_coord_t gap = control_modal_scaled_px(12, layout.short_side);
  if (gap < 8) gap = 8;
  lv_coord_t row_h = control_modal_scaled_px(48, layout.short_side);
  if (row_h < 34) row_h = 34;
  lv_coord_t row_radius = row_h / 3;
  lv_coord_t title_y = layout.inset + layout.back_size / 2;
  lv_coord_t list_y = layout.inset + layout.back_size + gap;
  lv_coord_t list_h = layout.panel_h - list_y - layout.inset;
  if (list_h < row_h) list_h = row_h;

  ui.title_lbl = control_modal_create_title(
    ui.panel, espcontrol_i18n("Preset"), content_w - layout.back_size - gap,
    ctx->label_font, ctx->width_compensation_percent);
  lv_obj_align(ui.title_lbl, LV_ALIGN_TOP_MID, 0, title_y - layout.back_size / 2);

  ui.list = control_modal_create_scroll_list(ui.panel, content_w, list_h, gap);
  lv_obj_align(ui.list, LV_ALIGN_TOP_LEFT, layout.inset, list_y);

  int count = ctx->preset_modes.size() > FAN_PRESET_MAX_OPTIONS
    ? FAN_PRESET_MAX_OPTIONS
    : static_cast<int>(ctx->preset_modes.size());
  std::string current = fan_lower(fan_trim(ctx->preset_mode));
  for (int i = 0; i < count; i++) {
    const std::string &mode = ctx->preset_modes[i];
    bool selected = fan_lower(fan_trim(mode)) == current;
    lv_obj_t *btn = control_modal_create_list_row(
      ui.list, fan_option_label(mode), selected, row_h, row_radius,
      ctx->on_color, DARK_BACKGROUND_SECONDARY,
      ctx->label_font, ctx->width_compensation_percent);
    ui.option_clicks[i].ctx = ctx;
    ui.option_clicks[i].mode = mode;
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
      FanPresetClick *click = (FanPresetClick *)lv_event_get_user_data(e);
      if (click && click->ctx) send_fan_preset_action(click->ctx, click->mode);
      fan_preset_close();
    }, LV_EVENT_CLICKED, &ui.option_clicks[i]);
  }

  if (count == 0) {
    ui.empty_lbl = lv_label_create(ui.list);
    lv_label_set_text(ui.empty_lbl, espcontrol_i18n("No presets"));
    lv_label_set_long_mode(ui.empty_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(ui.empty_lbl, lv_pct(100));
    lv_obj_set_style_text_color(ui.empty_lbl, lv_color_hex(DARK_TEXT_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_text_align(ui.empty_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    if (ctx->label_font) lv_obj_set_style_text_font(ui.empty_lbl, ctx->label_font, LV_PART_MAIN);
  }

  lv_obj_move_foreground(ui.overlay);
}

inline void fan_control_panel_open(FanCardCtx *ctx) {
  if (!ctx || !ctx->available) return;
  ControlModalShell shell = control_modal_open_shell(
    ControlModalKind::FAN_CONTROL, ctx->btn, ctx->width_compensation_percent,
    ctx->icon_font, "\U000F0141", false, fan_control_panel_close);
  FanControlPanelUi &ui = fan_control_panel_ui();
  ui.active = ctx;
  ui.overlay = shell.overlay;
  ui.panel = shell.panel;
  ui.close_btn = shell.close_btn;

  ControlModalLayout &layout = shell.layout;
  lv_coord_t content_w = shell.content_w;
  ui.content_w = content_w;
  lv_coord_t gap = control_modal_scaled_px(12, layout.short_side);
  if (gap < 8) gap = 8;
  lv_coord_t row_h = control_modal_scaled_px(48, layout.short_side);
  if (row_h < 34) row_h = 34;
  lv_coord_t row_radius = row_h / 3;
  lv_coord_t title_y = layout.inset + layout.back_size / 2;
  lv_coord_t list_y = layout.inset + layout.back_size + gap;
  lv_coord_t list_h = layout.panel_h - list_y - layout.inset;
  if (list_h < row_h) list_h = row_h;

  ui.title_lbl = control_modal_create_title(
    ui.panel, ctx->label, content_w - layout.back_size - gap,
    ctx->label_font, ctx->width_compensation_percent);
  lv_obj_align(ui.title_lbl, LV_ALIGN_TOP_MID, 0, title_y - layout.back_size / 2);

  ui.list = control_modal_create_scroll_list(ui.panel, content_w, list_h, gap);
  lv_obj_align(ui.list, LV_ALIGN_TOP_LEFT, layout.inset, list_y);

  // Round power button
  {
    lv_obj_t *btn_wrap = lv_obj_create(ui.list);
    lv_obj_set_size(btn_wrap, content_w, layout.btn_size);
    lv_obj_set_style_bg_opa(btn_wrap, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn_wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn_wrap, 0, LV_PART_MAIN);
    lv_obj_clear_flag(btn_wrap, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(btn_wrap, LV_OBJ_FLAG_CLICKABLE);
    ui.onoff_btn = control_modal_create_round_button(
      btn_wrap, layout.btn_size, find_icon("Power"), ctx->icon_font,
      DARK_CONTROL_NEUTRAL,
      ctx->on ? ctx->on_color : (uint32_t)DARK_BACKGROUND_SECONDARY,
      ctx->width_compensation_percent);
    lv_obj_center(ui.onoff_btn);
    lv_obj_add_event_cb(ui.onoff_btn, [](lv_event_t *e) {
      FanCardCtx *c = (FanCardCtx *)lv_event_get_user_data(e);
      if (c) send_fan_switch_action(c);
    }, LV_EVENT_CLICKED, ctx);
  }

  // Speed slider
  {
    lv_coord_t track_h = control_modal_scaled_px(8, layout.short_side);
    if (track_h < 6) track_h = 6;
    lv_coord_t knob_pad = control_modal_scaled_px(9, layout.short_side);
    if (knob_pad < 7) knob_pad = 7;
    lv_coord_t slider_w = content_w - gap * 2;
    lv_coord_t wrap_h = track_h + knob_pad * 2 + gap * 2;

    lv_obj_t *speed_wrap = lv_obj_create(ui.list);
    lv_obj_set_size(speed_wrap, content_w, wrap_h);
    lv_obj_set_style_bg_color(speed_wrap, lv_color_hex(DARK_BACKGROUND_SECONDARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(speed_wrap, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(speed_wrap, row_radius, LV_PART_MAIN);
    lv_obj_set_style_border_width(speed_wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(speed_wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(speed_wrap, 0, LV_PART_MAIN);
    lv_obj_clear_flag(speed_wrap, LV_OBJ_FLAG_SCROLLABLE);

    ui.speed_slider = lv_slider_create(speed_wrap);
    lv_obj_set_size(ui.speed_slider, slider_w, track_h);
    lv_slider_set_range(ui.speed_slider, 1, 100);
    int slider_val = (ctx->on && ctx->percentage_known && ctx->percentage > 0)
                     ? ctx->percentage : 1;
    lv_slider_set_value(ui.speed_slider, slider_val, LV_ANIM_OFF);
    lv_obj_center(ui.speed_slider);

    // Track
    lv_obj_set_style_bg_color(ui.speed_slider, lv_color_hex(DARK_TRACK_BACKGROUND), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui.speed_slider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(ui.speed_slider, track_h / 2, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui.speed_slider, 0, LV_PART_MAIN);
    // Indicator
    lv_obj_set_style_bg_color(ui.speed_slider, lv_color_hex(ctx->on_color),
      static_cast<lv_style_selector_t>(LV_PART_INDICATOR));
    lv_obj_set_style_bg_opa(ui.speed_slider, ctx->on ? LV_OPA_COVER : LV_OPA_40,
      static_cast<lv_style_selector_t>(LV_PART_INDICATOR));
    lv_obj_set_style_radius(ui.speed_slider, track_h / 2,
      static_cast<lv_style_selector_t>(LV_PART_INDICATOR));
    // Knob
    lv_obj_set_style_bg_color(ui.speed_slider, lv_color_hex(DARK_TEXT_PRIMARY),
      static_cast<lv_style_selector_t>(LV_PART_KNOB));
    lv_obj_set_style_bg_opa(ui.speed_slider, LV_OPA_COVER,
      static_cast<lv_style_selector_t>(LV_PART_KNOB));
    lv_obj_set_style_pad_all(ui.speed_slider, knob_pad,
      static_cast<lv_style_selector_t>(LV_PART_KNOB));
    lv_obj_set_style_radius(ui.speed_slider, (track_h + knob_pad * 2) / 2,
      static_cast<lv_style_selector_t>(LV_PART_KNOB));
    lv_obj_set_style_border_width(ui.speed_slider, 0,
      static_cast<lv_style_selector_t>(LV_PART_KNOB));
    lv_obj_set_style_shadow_width(ui.speed_slider, 0,
      static_cast<lv_style_selector_t>(LV_PART_KNOB));

    lv_obj_add_event_cb(ui.speed_slider, [](lv_event_t *e) {
      lv_obj_t *sl = static_cast<lv_obj_t *>(lv_event_get_target(e));
      FanCardCtx *c = (FanCardCtx *)lv_event_get_user_data(e);
      if (!sl || !c) return;
      int val = lv_slider_get_value(sl);
      std::string s = std::to_string(val);
      send_fan_action(c->entity_id, "fan.set_percentage", "percentage", s.c_str());
    }, LV_EVENT_RELEASED, ctx);
  }

  // Oscillation row (conditional)
  if (ctx->oscillation_known) {
    ui.oscillate_row = control_modal_create_list_row(
      ui.list, espcontrol_i18n("Oscillation"),
      ctx->oscillating, row_h, row_radius,
      ctx->on_color, DARK_BACKGROUND_SECONDARY,
      ctx->label_font, ctx->width_compensation_percent);
    lv_obj_add_event_cb(ui.oscillate_row, [](lv_event_t *e) {
      FanCardCtx *c = (FanCardCtx *)lv_event_get_user_data(e);
      if (c) send_fan_oscillate_action(c);
    }, LV_EVENT_CLICKED, ctx);
  }

  // Direction row (conditional)
  if (ctx->direction_known) {
    ui.direction_row = control_modal_create_list_row(
      ui.list,
      ctx->direction == "reverse" ? espcontrol_i18n("Reverse") : espcontrol_i18n("Forward"),
      ctx->direction == "reverse", row_h, row_radius,
      ctx->on_color, DARK_BACKGROUND_SECONDARY,
      ctx->label_font, ctx->width_compensation_percent);
    lv_obj_add_event_cb(ui.direction_row, [](lv_event_t *e) {
      FanCardCtx *c = (FanCardCtx *)lv_event_get_user_data(e);
      if (c) send_fan_direction_action(c);
    }, LV_EVENT_CLICKED, ctx);
  }

  // Single preset chip (conditional) — opens popup menu on click
  if (!ctx->preset_modes.empty()) {
    lv_coord_t chip_h = control_modal_scaled_px(70, layout.short_side);
    if (chip_h < 50) chip_h = 50;
    lv_coord_t chip_w = content_w * 55 / 100;
    if (chip_w < 160) chip_w = 160;

    lv_obj_t *chip_wrap = lv_obj_create(ui.list);
    lv_obj_set_size(chip_wrap, content_w, chip_h);
    lv_obj_set_style_bg_opa(chip_wrap, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(chip_wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(chip_wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(chip_wrap, 0, LV_PART_MAIN);
    lv_obj_clear_flag(chip_wrap, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(chip_wrap, LV_OBJ_FLAG_CLICKABLE);

    bool preset_active = fan_preset_active(ctx->preset_mode);
    ui.preset_chip = lv_btn_create(chip_wrap);
    lv_obj_set_size(ui.preset_chip, chip_w, chip_h);
    lv_obj_center(ui.preset_chip);
    lv_obj_set_style_radius(ui.preset_chip, chip_h / 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui.preset_chip,
      lv_color_hex(DARK_BACKGROUND_SECONDARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui.preset_chip, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui.preset_chip, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(ui.preset_chip, 0, LV_PART_MAIN);
    control_modal_apply_pressed_fill(ui.preset_chip);

    std::string chip_text = preset_active
      ? fan_option_label(ctx->preset_mode) : espcontrol_i18n(std::string("Preset"));
    ui.preset_chip_lbl = lv_label_create(ui.preset_chip);
    lv_label_set_text(ui.preset_chip_lbl, chip_text.c_str());
    lv_label_set_long_mode(ui.preset_chip_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(ui.preset_chip_lbl, lv_color_hex(DARK_TEXT_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_text_align(ui.preset_chip_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    if (ctx->label_font) lv_obj_set_style_text_font(ui.preset_chip_lbl, ctx->label_font, LV_PART_MAIN);
    lv_obj_center(ui.preset_chip_lbl);

    lv_obj_add_event_cb(ui.preset_chip, [](lv_event_t *) {
      fan_control_preset_menu_open();
    }, LV_EVENT_CLICKED, nullptr);
  }

  lv_obj_move_foreground(ui.overlay);
}

inline void fan_card_handle_click(FanCardCtx *ctx) {
  if (!fan_control_supported(ctx)) return;
  if (ctx->type == "fan") fan_control_panel_open(ctx);
  else if (ctx->type == "fan_switch") send_fan_switch_action(ctx);
  else if (ctx->type == "fan_oscillate") send_fan_oscillate_action(ctx);
  else if (ctx->type == "fan_direction") send_fan_direction_action(ctx);
  else if (ctx->type == "fan_preset") fan_preset_open(ctx);
}

inline FanCardCtx *create_fan_card_context(
    BtnSlot &slot, const ParsedCfg &p,
    uint32_t on_color, uint32_t off_color, uint32_t tertiary_color,
    const lv_font_t *label_font,
    const lv_font_t *icon_font,
    int width_compensation_percent) {
  FanCardCtx *ctx = new FanCardCtx();
  ctx->type = p.type;
  ctx->entity_id = p.entity;
  ctx->label = fan_card_label(p);
  ctx->has_custom_label = !p.label.empty();
  ctx->btn = slot.btn;
  ctx->icon_lbl = slot.icon_lbl;
  ctx->icon_off_glyph = find_icon(fan_card_icon_name(p));
  ctx->icon_on_glyph = find_icon(fan_card_icon_on_name(p));
  ctx->on_color = on_color;
  ctx->off_color = off_color;
  ctx->tertiary_color = tertiary_color;
  ctx->label_font = label_font;
  ctx->icon_font = icon_font;
  ctx->width_compensation_percent = width_compensation_percent;
  ctx->label_display = cfg_option_value(p.options, "label_display");
  ctx->status_label = create_transient_status_label(slot.text_lbl, ctx->label);
  lv_obj_set_user_data(slot.btn, ctx);
  fan_refresh_card(ctx);
  return ctx;
}

inline void subscribe_fan_card_state(FanCardCtx *ctx) {
  if (!ctx || ctx->entity_id.empty()) return;
  register_ha_control_availability(ctx->btn, ctx->btn);
  auto refresh = [ctx]() { fan_refresh_card(ctx); };
  ha_subscribe_state(
    ctx->entity_id,
    std::function<void(esphome::StringRef)>(
      [ctx, refresh](esphome::StringRef state) {
        ctx->state = fan_lower(fan_trim(string_ref_limited(state, HA_SHORT_STATE_MAX_LEN)));
        ctx->available = !ha_state_unavailable_ref(state);
        ctx->on = ctx->available && is_entity_on_ref(state);
        refresh();
        fan_preset_close_if_unsupported(ctx);
      })
  );
  ha_subscribe_attribute(
    ctx->entity_id, std::string("friendly_name"),
    std::function<void(esphome::StringRef)>(
      [ctx](esphome::StringRef value) {
        ctx->friendly_name = string_ref_limited(value, HA_FRIENDLY_NAME_MAX_LEN);
        if (!ctx->has_custom_label) {
          transient_status_label_set_steady(ctx->status_label, ctx->friendly_name);
        }
      })
  );
  ha_subscribe_attribute(
    ctx->entity_id, std::string("oscillating"),
    std::function<void(esphome::StringRef)>(
      [ctx, refresh](esphome::StringRef value) {
        bool parsed = false;
        std::string text = string_ref_limited(value, HA_SHORT_STATE_MAX_LEN);
        ctx->oscillation_known = fan_parse_bool_value(text, parsed);
        if (ctx->oscillation_known) ctx->oscillating = parsed;
        refresh();
      })
  );
  ha_subscribe_attribute(
    ctx->entity_id, std::string("direction"),
    std::function<void(esphome::StringRef)>(
      [ctx, refresh](esphome::StringRef value) {
        ctx->direction = fan_lower(fan_trim(string_ref_limited(value, HA_SHORT_STATE_MAX_LEN)));
        ctx->direction_known = ctx->direction == "forward" || ctx->direction == "reverse";
        refresh();
      })
  );
  ha_subscribe_attribute(
    ctx->entity_id, std::string("preset_mode"),
    std::function<void(esphome::StringRef)>(
      [ctx, refresh](esphome::StringRef value) {
        ctx->preset_mode = fan_lower(fan_trim(string_ref_limited(value, HA_SHORT_STATE_MAX_LEN)));
        refresh();
      })
  );
  ha_subscribe_attribute(
    ctx->entity_id, std::string("preset_modes"),
    std::function<void(esphome::StringRef)>(
      [ctx, refresh](esphome::StringRef value) {
        ctx->preset_modes = fan_parse_options(value);
        refresh();
        fan_preset_close_if_unsupported(ctx);
      })
  );
  if (ctx->type == "fan") {
    ha_subscribe_attribute(
      ctx->entity_id, std::string("percentage"),
      std::function<void(esphome::StringRef)>(
        [ctx, refresh](esphome::StringRef value) {
          int pct = 0;
          ctx->percentage_known = slider_parse_pct(value, pct);
          if (ctx->percentage_known) ctx->percentage = pct;
          refresh();
        })
    );
    ha_subscribe_attribute(
      ctx->entity_id, std::string("percentage_step"),
      std::function<void(esphome::StringRef)>(
        [ctx](esphome::StringRef value) {
          int step = 0;
          if (slider_parse_pct(value, step) && step > 0) {
            ctx->percentage_step = step;
            ctx->percentage_step_known = true;
          }
        })
    );
  }
}
