# Card Type Map

Use this map when adding or changing a card type. It points to the first file a
maintainer or tool should inspect, then calls out the shared behavior that often
needs matching changes.

All card types also share these core files:

- Contract source: `common/config/card_contract.json`
- Web parser/serializer: `src/webserver/modules/config_codec.js`
- Firmware parser: `components/espcontrol/button_grid_config.h`
- Firmware setup and binding: `components/espcontrol/button_grid_grid.h`
- Generated contract outputs:
  `src/webserver/modules/card_contract_generated.js`,
  `components/espcontrol/button_grid_contract_generated.h`, and
  `docs/generated/cards/capabilities.md`

## Public Documentation Map

Keep the public docs matched to the setup-page names users see. When adding or
changing a card type, update the relevant page and the sidebar in
`docs/.vitepress/config.mts`.

| Public card page | Covered saved types |
|---|---|
| `docs/card-types/index.md` | Human-facing chooser for all visible card groups |
| `docs/card-types/switches.md` | `(empty)` switch |
| `docs/card-types/lights.md` | `light_switch`, `light_brightness`, `light_temperature` |
| `docs/card-types/actions.md` | `action` and action modes |
| `docs/card-types/option-select.md` | Option-select action path and hidden `option_select` compatibility entry |
| `docs/card-types/webhooks.md` | `webhook` |
| `docs/card-types/buttons.md` | `push` |
| `docs/card-types/sensors.md` | `sensor` |
| `docs/card-types/doors-windows.md` | `door_window` |
| `docs/card-types/presence.md` | `presence` |
| `docs/card-types/sliders.md` | `slider` |
| `docs/card-types/fans.md` | `fan_switch`, `fan_speed`, `fan_oscillate`, `fan_direction`, `fan_preset` |
| `docs/card-types/covers.md` | `cover` |
| `docs/card-types/garage-doors.md` | `garage` |
| `docs/card-types/locks.md` | `lock` |
| `docs/card-types/alarms.md` | `alarm`, `alarm_action` |
| `docs/card-types/calendar.md` | `calendar`, `clock` |
| `docs/card-types/timezones.md` | `timezone` |
| `docs/card-types/weather.md` | `weather`, legacy `weather_forecast` behavior |
| `docs/card-types/weather-forecast.md` | Legacy redirect/explanation for `weather_forecast` |
| `docs/card-types/cameras.md` | `image` |
| `docs/card-types/media.md` | `media` |
| `docs/card-types/climate.md` | `climate` |
| `docs/card-types/internal-relays.md` | `internal` |
| `docs/card-types/screen-lock.md` | `screen_lock` |
| `docs/features/subpages.md` | `subpage` |

## Check Key

| Key | Command | Use when |
|---|---|---|
| Contract | `npm run check:card-contract-outputs` | Card metadata, labels, defaults, domains, picker grouping, generated contract files |
| Generated | `npm run check:generated` | Any source change that should rebuild generated web, firmware, or docs output |
| Codec | `npm run check:web-smoke` | Web settings, preview, saved config, import/export, compact config shape |
| Types | `npm run check:types` | TypeScript model or generated model surface changes |
| Parser | `npm run check:firmware-parser` | Firmware saved-config parsing, options preservation, aliases, legacy config |
| Runtime | `npm run check:firmware-card-runtime` | Shared card modes, service mappings, generated contract access |
| HA | `npm run check:firmware-ha-bindings` | Home Assistant subscriptions, actions, response callbacks, reconnect behavior |
| Modals | `npm run check:firmware-modals` | Full-screen overlays, modal lifecycle, nested modal behavior |
| Backup | `npm run check:backup-contract` | Backup import/export shape changes |
| Model | `npm run check:model-contract` | Web model contract changes |

## Matrix

| Type | Web file | Firmware header | Stores options | Opens modal | HA subscriptions | Key checks |
|---|---|---|---|---|---|---|
| `(empty)` switch | `src/webserver/types/switch.js` | `button_grid_cards.h`, `button_grid_actions.h`, `button_grid_confirm.h` | Yes: large numbers, confirmation, on pattern | Conditional confirmation modal | Entity state, optional sensor, friendly name | Contract, Codec, Parser, HA, Modals |
| `action` | `src/webserver/types/action.js` | `button_grid_cards.h`, `button_grid_actions.h`, `button_grid_subscriptions.h` | Yes: large numbers, optional display state | No, except option-select action mode | Optional display-state entity | Contract, Codec, Parser, HA |
| `alarm` | `src/webserver/types/alarm.js` | `button_grid_alarm.h` | Yes: control/action mode, PIN rules, visible actions, display mode | Yes: alarm control and PIN modals | Alarm state, friendly name | Contract, Codec, Parser, HA, Modals |
| `alarm_action` | `src/webserver/types/alarm.js` | `button_grid_alarm.h` | Yes: inherited alarm PIN options | Yes: PIN modal when required | Alarm state for selected action | Contract, Codec, Parser, HA, Modals |
| `calendar` | `src/webserver/types/calendar.js` | `button_grid_cards.h` | Yes: date/time mode, large numbers | No | Date sensor source, when configured | Contract, Codec, Parser, HA |
| `clock` | `src/webserver/types/clock.js` | `button_grid_cards.h` | Yes: date/time mode, large numbers | No | None; local display time | Contract, Codec, Parser |
| `timezone` | `src/webserver/types/timezone.js` | `button_grid_cards.h` | Yes: date/time mode, large numbers | No | None; local timezone rendering | Contract, Codec, Parser |
| `climate` | `src/webserver/types/climate.js` | `button_grid_climate.h` | Yes: label display, number display, large numbers | Yes: climate control modal | Climate state and friendly name | Contract, Codec, Parser, HA, Modals |
| `cover` | `src/webserver/types/slider.js` | `button_grid_sliders.h`, `button_grid_actions.h` | Yes: mode and position | No | Cover state, friendly name, availability for command cards | Contract, Codec, Parser, HA, Runtime |
| `door_window` | `src/webserver/types/door_window.js` | `button_grid_cards.h`, `button_grid_subscriptions.h` | Yes: active color | No | Sensor state and friendly name | Contract, Codec, Parser, HA |
| `presence` | `src/webserver/types/presence.js` | `button_grid_cards.h`, `button_grid_subscriptions.h` | Yes: active color | No | Sensor state and friendly name | Contract, Codec, Parser, HA |
| `fan_speed` | `src/webserver/types/fan.js` | `button_grid_sliders.h` | No | No | Fan state, friendly name | Contract, Codec, Parser, HA, Runtime |
| `fan_switch` | `src/webserver/types/fan.js` | `button_grid_fan.h` | No | No | Fan state | Contract, Codec, Parser, HA, Runtime |
| `fan_oscillate` | `src/webserver/types/fan.js` | `button_grid_fan.h` | No | No | Fan state | Contract, Codec, Parser, HA, Runtime |
| `fan_direction` | `src/webserver/types/fan.js` | `button_grid_fan.h` | No | No | Fan state | Contract, Codec, Parser, HA, Runtime |
| `fan_preset` | `src/webserver/types/fan.js` | `button_grid_fan.h` | No | Yes: preset picker modal | Fan state and preset modes | Contract, Codec, Parser, HA, Modals, Runtime |
| `garage` | `src/webserver/types/garage.js` | `button_grid_cards.h`, `button_grid_actions.h` | Yes: command mode, label display | No | Cover state, friendly name, availability for command cards | Contract, Codec, Parser, HA, Runtime |
| `internal` | `src/webserver/types/internal.js` | `button_grid_cards.h`, `button_grid_actions.h` | Yes: switch or push mode | No | None; local relay watcher | Contract, Codec, Parser |
| `light_brightness` | `src/webserver/types/slider.js` | `button_grid_sliders.h` | No | No | Light state, friendly name | Contract, Codec, Parser, HA |
| `light_switch` | `src/webserver/types/switch.js` | `button_grid_cards.h`, `button_grid_actions.h` | No | No | Light state, friendly name | Contract, Codec, Parser, HA |
| `light_temperature` | `src/webserver/types/light_temperature.js` | `button_grid_sliders.h` | No | No | Light color-temperature state, friendly name | Contract, Codec, Parser, HA |
| `lock` | `src/webserver/types/lock.js` | `button_grid_cards.h`, `button_grid_actions.h`, `button_grid_subscriptions.h` | Yes: command mode | No | Lock state, friendly name, availability for command cards | Contract, Codec, Parser, HA, Runtime |
| `media` | `src/webserver/types/media.js` | `button_grid_media.h`, `button_grid_sliders.h`, `button_grid_actions.h` | Yes: mode, display options, volume max, large numbers | Yes for volume mode | Media state, attributes, friendly name | Contract, Codec, Parser, HA, Modals, Runtime |
| `option_select` | `src/webserver/types/action.js` | `button_grid_option_select.h` | No | Yes: option list modal | Select state, options, friendly name | Contract, Codec, Parser, HA, Modals |
| `push` | `src/webserver/types/push.js` | `button_grid_cards.h`, `button_grid_actions.h` | No | No | Sends HA action; no state subscription | Contract, Codec, Parser, HA |
| `screen_lock` | `src/webserver/types/screen_lock.js` | `button_grid_cards.h`, `button_grid_config.h` | No | No | None; local screen-lock registry | Contract, Codec, Parser |
| `webhook` | `src/webserver/types/webhook.js` | `button_grid_actions.h` | Yes: URL, method, headers | No | None; direct HTTP action | Contract, Codec, Parser |
| `sensor` | `src/webserver/types/sensor.js` | `button_grid_cards.h`, `button_grid_subscriptions.h` | Yes: large numbers, active color, text state labels | No | Sensor state and friendly name | Contract, Codec, Parser, HA |
| `slider` | `src/webserver/types/slider.js` | `button_grid_sliders.h` | No | No | Light or fan state, friendly name | Contract, Codec, Parser, HA |
| `subpage` | `src/webserver/types/subpage.js` | `button_grid_subpages.h`, `button_grid_grid.h` | Yes: kind, parent state, large numbers | No; navigates to subpage | Optional parent entity/sensor state and child indicators | Contract, Codec, Parser, HA |
| `weather` | `src/webserver/types/weather.js` | `button_grid_cards.h`, `button_grid_subscriptions.h` | Yes: current/forecast mode, large numbers | No | Current weather state or forecast request | Contract, Codec, Parser, HA |
| `image` | `src/webserver/types/image.js` | `button_grid_image.h` | Yes: label, icon, modal fit, refresh options | Yes: image modal | Camera/image state and source URL handling | Contract, Codec, Parser, HA, Modals |
| `weather_forecast` | `src/webserver/types/weather_forecast.js` | `button_grid_cards.h` | No; legacy precision drives day | No | Weather forecast request | Contract, Codec, Parser, HA |
| `todo` | No current web type; firmware-only/disabled path | `button_grid_todo.h` | Yes: count display, large numbers | Yes: todo list modal | Todo state plus `todo.get_items` response callback | Parser, HA, Modals |

## Staleness Notes

- This file is intentionally manual for now. If it starts drifting, generate the
  type, web file, and contract-backed columns from `common/config/card_contract.json`
  plus `registerButtonType(...)` calls.
- Rows with hidden or compatibility behavior still matter because old saved
  config can reach the firmware parser.
- The `Stores options` column means options are preserved somewhere in saved
  config, not necessarily that the card has explicit `options` metadata in the
  contract.
