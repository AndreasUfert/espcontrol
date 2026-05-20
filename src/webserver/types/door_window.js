// Read-only door/window card: shows a binary sensor with subtype-specific icons.
registerButtonType("door_window", {
  label: "Doors & Windows",
  allowInSubpage: true,
  hideLabel: true,
  onSelect: function (b) {
    b.entity = "";
    b.sensor = "";
    b.unit = "";
    b.precision = "door";
    b.icon = doorWindowClosedIcon(b.precision);
    b.icon_on = doorWindowOpenIcon(b.precision);
    b.options = setConfigOption("", SENSOR_ACTIVE_COLOR_OPTION, true);
  },
  renderSettings: function (panel, b, slot, helpers) {
    b.entity = "";
    b.unit = "";
    b.precision = normalizeDoorWindowSubtype(b.precision);
    b.options = normalizeDoorWindowOptions(b.options);
    if (!b.icon || b.icon === "Auto") b.icon = doorWindowClosedIcon(b.precision);
    if (!b.icon_on || b.icon_on === "Auto") b.icon_on = doorWindowOpenIcon(b.precision);

    var subtype = helpers.segmentControl([
      ["door", "Door"],
      ["window", "Window"],
    ], b.precision, function (value) {
      setSubtype(value, true);
    });
    panel.appendChild(helpers.fieldWithControl("Type", null, subtype.segment));

    var sensorField = helpers.entityField(
      "Sensor Entity", helpers.idPrefix + "sensor", b.sensor,
      "e.g. binary_sensor.patio_door",
      ["binary_sensor", "sensor"], "sensor", true,
      "Add a door or window sensor before saving.");
    panel.appendChild(sensorField.field);

    panel.appendChild(helpers.textField(
      "Label", helpers.idPrefix + "label", b.label, "e.g. Patio Door", "label", true).field);

    var closedIconPicker = helpers.iconPickerField(
      helpers.idPrefix + "closed-icon-picker", helpers.idPrefix + "icon",
      b.icon || doorWindowClosedIcon(b.precision), function (opt) {
        b.icon = opt;
        helpers.saveField("icon", opt);
      }, "Closed Icon"
    );
    panel.appendChild(closedIconPicker);

    var openIconPicker = helpers.iconPickerField(
      helpers.idPrefix + "open-icon-picker", helpers.idPrefix + "icon-on",
      b.icon_on || doorWindowOpenIcon(b.precision), function (opt) {
        b.icon_on = opt;
        helpers.saveField("icon_on", opt);
      }, "Open Icon"
    );
    panel.appendChild(openIconPicker);

    var advancedToggleSection = helpers.toggleSection(
      "Advanced",
      helpers.idPrefix + "door-window-advanced-toggle",
      !doorWindowActiveColorEnabled(b)
    );
    var advancedToggle = advancedToggleSection.toggle;
    var advancedSection = advancedToggleSection.section;
    panel.appendChild(advancedToggle.row);
    if (!doorWindowActiveColorEnabled(b)) advancedSection.classList.add("sp-visible");

    var activeColorToggle = helpers.toggleRow(
      "Use On Colour When Open",
      helpers.idPrefix + "door-window-active-color",
      doorWindowActiveColorEnabled(b)
    );
    advancedSection.appendChild(activeColorToggle.row);
    panel.appendChild(advancedSection);

    advancedToggle.input.addEventListener("change", function () {
      advancedSection.classList.toggle("sp-visible", this.checked);
      if (this.checked) return;
      activeColorToggle.input.checked = true;
      setDoorWindowActiveColorEnabled(b, true);
      helpers.saveField("options", b.options);
    });

    activeColorToggle.input.addEventListener("change", function () {
      setDoorWindowActiveColorEnabled(b, this.checked);
      helpers.saveField("options", b.options);
    });

    function syncIconPicker(picker, value) {
      var preview = picker.querySelector(".sp-icon-picker-preview");
      if (preview) preview.className = "sp-icon-picker-preview mdi mdi-" + iconSlug(value);
      var input = picker.querySelector(".sp-icon-picker-input");
      if (input) input.value = value;
    }

    function setSubtype(value, persist) {
      var previousClosed = doorWindowClosedIcon(b.precision);
      var previousOpen = doorWindowOpenIcon(b.precision);
      b.precision = normalizeDoorWindowSubtype(value);
      subtype.buttons.door.classList.toggle("active", b.precision === "door");
      subtype.buttons.window.classList.toggle("active", b.precision === "window");

      if (!b.icon || b.icon === "Auto" || b.icon === previousClosed) {
        b.icon = doorWindowClosedIcon(b.precision);
        syncIconPicker(closedIconPicker, b.icon);
      }
      if (!b.icon_on || b.icon_on === "Auto" || b.icon_on === previousOpen) {
        b.icon_on = doorWindowOpenIcon(b.precision);
        syncIconPicker(openIconPicker, b.icon_on);
      }
      if (!persist) return;
      helpers.saveField("precision", b.precision);
      helpers.saveField("icon", b.icon);
      helpers.saveField("icon_on", b.icon_on);
    }

    setSubtype(b.precision, false);
  },
  renderPreview: function (b, helpers) {
    var subtype = normalizeDoorWindowSubtype(b.precision);
    var icon = b.icon && b.icon !== "Auto" ? b.icon : doorWindowClosedIcon(subtype);
    var label = b.label || b.sensor || (subtype === "window" ? "Window" : "Door");
    return {
      iconHtml: '<span class="sp-btn-icon mdi mdi-' + iconSlug(icon) + '"></span>',
      labelHtml:
        '<span class="sp-btn-label-row"><span class="sp-btn-label">' + helpers.escHtml(label) + '</span>' +
        '<span class="sp-type-badge mdi mdi-' + (subtype === "window" ? "window-closed" : "door") + '"></span></span>',
    };
  },
});
