#include <cstring>  // strlen
#include "kd2_dummy_data.h"

static const char* DUMMY_JSON = R"json(
{
  "wind": { "dir": "VNV", "speed": 2.3, "deg": 315 },
  "temp": { "out": -22.3, "feels": -23.9 },
  "rain": [15, 55, 35],
  "forecast": "13.3 m/s  ONO\n11.4 m/s  NNV",
  "atm": { "pressure": 1000.3, "humidity": 90 },
  "updated_min": 60,
  "icon": "rain"
}
)json";


const char* kd2_dummy_json() {
  return DUMMY_JSON;
}

size_t kd2_dummy_json_len() {
  return strlen(DUMMY_JSON);
}
