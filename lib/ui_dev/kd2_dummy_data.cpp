#include <cstring>  // strlen
#include "kd2_dummy_data.h"

static const char* DUMMY_JSON = R"json(
{
  "wind": { "dir": "VNV", "speed": 20.3, "deg": 315 },
  "temp": { "out": -22.3, "feels": -23.9 },
  "rain": [15, 55, 35],
  "atm": { "pressure": 1000.3, "humidity": 90 },
  "updated_min": 0,
  "icon": "rain",
  "forecast": "13.3 m/s  ONO\n11.4 m/s  NNV"
}
)json";


const char* kd2_dummy_json() {
  return DUMMY_JSON;
}

size_t kd2_dummy_json_len() {
  return strlen(DUMMY_JSON);
}
