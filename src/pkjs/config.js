module.exports = [
  {
    type: "heading",
    defaultValue: "Configuration",
  },
  {
    type: "section",
    items: [
      {
        type: "heading",
        defaultValue: "General",
      },
      {
        type: "radiogroup",
        messageKey: "XCLOCK_MODE",
        label: "xclock Display Mode",
        options: [
          {
            "label": "Both",
            "value": "0"
          },
          {
            "label": "Digital",
            "value": "1"
          },
          {
            "label": "Analog",
            "value": "2"
          }
        ],
        defaultValue: "0"
      },
      {
        type: "color",
        messageKey: "BG_COLOR",
        defaultValue: "00AAFF",
        label: "Background Color"
      },
      {
        type: "color",
        messageKey: "CLOCK_HANDS_COLOR",
        defaultValue: "00AAFF",
        label: "xclock Hands Color"
      },
      {
        type: "radiogroup",
        messageKey: "DISTANCE_UNIT",
        label: "Distance Unit",
        options: [
          {
            "label": "Metric",
            "value": "0"
          },
          {
            "label": "Imperial",
            "value": "1"
          }
        ],
        defaultValue: "1"
      },
    ],
  },
  {
    type: "submit",
    defaultValue: "Save Settings",
  },
];
