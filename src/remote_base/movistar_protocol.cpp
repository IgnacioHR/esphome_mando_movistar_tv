#include "movistar_protocol.h"
#include "esphome/core/log.h"
#include <cinttypes>

#include "raw_protocol.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.movistar";

// Confirmadas
static const uint32_t HEADER_HIGH_US = 950;
static const uint32_t HEADER_LOW_US = 950;
static const uint32_t HEADER2_HIGH_US = 647;
static const uint32_t HEADER2_LOW_US = 1265;
static const uint32_t BIT_HIGH_US = 318;
static const uint32_t BIT_HIGH2_US = 648;
static const uint32_t BIT_1_MARK = 1585;
static const uint32_t BIT_0_MARK = 959;
static const uint32_t BIT_A_MARK = 639;
static const uint32_t BIT_B_MARK = 318;
static const uint32_t BIT_Y_MARK = 380;
static const uint32_t BIT_Z_MARK = 624;

void dumpReceiveData(const RemoteReceiveData src) {
  char buffer[256];
  uint32_t buffer_offset = 0;
  buffer_offset += sprintf(buffer, "Received Raw: ");

  for (int32_t i = 0; i < src.size() - 1; i++) {
    const int32_t value = src[i];
    const uint32_t remaining_length = sizeof(buffer) - buffer_offset;
    int written;

    if (i + 1 < src.size() - 1) {
      written = snprintf(buffer + buffer_offset, remaining_length, "%d, ", value);
    } else {
      written = snprintf(buffer + buffer_offset, remaining_length, "%d", value);
    }

    if (written < 0 || written >= int(remaining_length)) {
      // write failed, flush...
      buffer[buffer_offset] = '\0';
      ESP_LOGD(TAG, "%s", buffer);
      buffer_offset = 0;
      written = sprintf(buffer, "  ");
      if (i + 1 < src.size()) {
        written += sprintf(buffer + written, "%d, ", value);
      } else {
        written += sprintf(buffer + written, "%d", value);
      }
    }

    buffer_offset += written;
  }
  if (buffer_offset != 0) {
    ESP_LOGD(TAG, "%s", buffer);
  }
}

void dumpTransmitData(const RemoteTransmitData src) {
  char buffer[256];
  uint32_t buffer_offset = 0;
  buffer_offset += sprintf(buffer, "Transmit Raw: ");

  for (int32_t i = 0; i < src.get_data().size() - 1; i++) {
    const int32_t value = src.get_data()[i];
    const uint32_t remaining_length = sizeof(buffer) - buffer_offset;
    int written;

    if (i + 1 < src.get_data().size() - 1) {
      written = snprintf(buffer + buffer_offset, remaining_length, "%d, ", value);
    } else {
      written = snprintf(buffer + buffer_offset, remaining_length, "%d", value);
    }

    if (written < 0 || written >= int(remaining_length)) {
      // write failed, flush...
      buffer[buffer_offset] = '\0';
      ESP_LOGD(TAG, "%s", buffer);
      buffer_offset = 0;
      written = sprintf(buffer, "  ");
      if (i + 1 < src.get_data().size()) {
        written += sprintf(buffer + written, "%d, ", value);
      } else {
        written += sprintf(buffer + written, "%d", value);
      }
    }

    buffer_offset += written;
  }
  if (buffer_offset != 0) {
    ESP_LOGD(TAG, "%s", buffer);
  }
}

void checkTransmitData(const MovistarData data, RemoteReceiveData src) {
  uint32_t totalus = 0;
  std::vector<int32_t> *raw = src.get_raw_data();
  std::for_each(raw->begin(), raw->end(), [&](int32_t n) { totalus += n > 0 ? n : -n; });
  ESP_LOGV(TAG, "Time %dus", totalus);
  RemoteTransmitData chk;
  MovistarProtocol p;
  p.encode(&chk, data);
  uint32_t totalus2 = 0;
  std::for_each(chk.begin(), chk.end(), [&](int32_t n) { totalus2 += n > 0 ? n : -n; });
  ESP_LOGV(TAG, "Time2 %dus", totalus2);

  dumpReceiveData(src);
  dumpTransmitData(chk);
}

void MovistarProtocol::encode(RemoteTransmitData *dst, const MovistarData &data) {
  // dst->set_carrier_frequency(38000);
  dst->set_carrier_frequency(57600);
  dst->reserve((8 + data.command.length()) * 2U);

  dst->item(HEADER_HIGH_US, HEADER_LOW_US);
  dst->item(HEADER2_HIGH_US, HEADER2_LOW_US);
  dst->item(BIT_HIGH_US, BIT_A_MARK);
  dst->item(BIT_HIGH_US, BIT_0_MARK);
  if (data.state) {
    dst->item(BIT_HIGH_US, BIT_0_MARK);
    dst->item(BIT_HIGH_US, BIT_0_MARK);
    dst->item(BIT_HIGH_US, BIT_0_MARK);
    dst->item(BIT_HIGH_US, BIT_0_MARK);
  } else {
    dst->item(BIT_HIGH_US, BIT_1_MARK);
    dst->item(BIT_HIGH_US, BIT_B_MARK);
    dst->item(BIT_HIGH_US, BIT_0_MARK);
    dst->item(BIT_HIGH_US, BIT_0_MARK);
  }

  std::for_each(data.command.begin(), data.command.end(), [&](char ch) {
    switch (ch) {
      case '1':
        dst->item(BIT_HIGH_US, BIT_1_MARK);
        break;

      case '0':
        dst->item(BIT_HIGH_US, BIT_0_MARK);
        break;

      case 'A':
        dst->item(BIT_HIGH_US, BIT_A_MARK);
        break;

      case 'B':
        dst->item(BIT_HIGH_US, BIT_B_MARK);
        break;

      case 'D':
        dst->item(BIT_HIGH2_US, BIT_0_MARK);
        break;

      case 'E':
        dst->item(BIT_HIGH2_US, BIT_1_MARK);
        break;

      case 'Y':
        dst->mark(BIT_Y_MARK);
        dst->space(0);
        break;

      case 'Z':
        dst->mark(BIT_Z_MARK);
        dst->space(0);
        break;

      default:
        break;
    }
  });
  // dumpTransmitData(*dst);
}
optional<MovistarData> MovistarProtocol::decode(RemoteReceiveData src) {
  MovistarData out;
  if (!src.expect_item(HEADER_HIGH_US, HEADER_LOW_US)) {
    ESP_LOGVV(TAG, "Unknown HEADER_HIGH_US HEADER_LOW_US %d %d - Received %d %d", HEADER_HIGH_US, HEADER2_LOW_US,
              src[0], src[1]);
    return {};
  }
  if (!src.expect_item(HEADER2_HIGH_US, HEADER2_LOW_US)) {
    ESP_LOGVV(TAG, "Unknown HEADER2_HIGH_US HEADER2_LOW_US %d %d - Received %d %d", HEADER2_HIGH_US, HEADER2_LOW_US,
              src[2], src[3]);
    return {};
  }
  if (!src.expect_item(BIT_HIGH_US, BIT_A_MARK)) {
    ESP_LOGVV(TAG, "Missing A after headers");
    return {};
  }
  if (!src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
    ESP_LOGVV(TAG, "Missing ZERO after A mark");
    return {};
  }
  if (src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
    // ESP_LOGV(TAG, "PULSE");
    out.state = true;
    if (!src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
      ESP_LOGVV(TAG, "Missing 1st OFF after PULSE");
      return {};
    }
    if (!src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
      ESP_LOGVV(TAG, "Missing 2nd OFF after PULSE");
      return {};
    }
    if (!src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
      ESP_LOGVV(TAG, "Missing 3rd OFF after PULSE");
      return {};
    }
  } else if (src.expect_item(BIT_HIGH_US, BIT_1_MARK)) {
    // ESP_LOGV(TAG, "RELEASE");
    out.state = false;
    if (!src.expect_item(BIT_HIGH_US, BIT_B_MARK)) {
      ESP_LOGVV(TAG, "Missing B after RELEASE");
      return {};
    }
    if (!src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
      ESP_LOGVV(TAG, "Missing 1st OFF after PULSE");
      return {};
    }
    if (!src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
      ESP_LOGVV(TAG, "Missing 2nd OFF after PULSE");
      return {};
    }
  } else {
    ESP_LOGV(TAG, "NEITHER PULSE OR RELEASE??");
    return {};
  }
  while (src.size() - src.get_index() > 0) {
    // ESP_LOGV(TAG, "Data %s", movistardata);
    int32_t a = src[src.get_index()];
    int32_t b = src[src.get_index() + 1];
    if (src.expect_item(BIT_HIGH_US, BIT_1_MARK)) {
      out.command += '1';
    } else if (src.expect_item(BIT_HIGH_US, BIT_0_MARK)) {
      out.command += '0';
    } else if (src.expect_item(BIT_HIGH_US, BIT_A_MARK)) {
      out.command += 'A';
    } else if (src.expect_item(BIT_HIGH_US, BIT_B_MARK)) {
      out.command += 'B';
    } else if (src.expect_item(BIT_HIGH2_US, BIT_0_MARK)) {
      out.command += 'D';
    } else if (src.expect_item(BIT_HIGH2_US, BIT_1_MARK)) {
      out.command += 'E';
    } else if (src.get_index() == (src.size() - 2) && src.expect_mark(BIT_Y_MARK)) {
      out.command += 'Y';
      // checkTransmitData(out, src);
      return out;
    } else if (src.get_index() == (src.size() - 2) && src.expect_mark(BIT_Z_MARK)) {
      out.command += 'Z';
      // checkTransmitData(out, src);
      return out;
    } else {
      ESP_LOGVV(TAG, "UNKNOWN %d %d - up till now %s", a, b, out.command.c_str());
      return {};
    }
  }
  ESP_LOGV(TAG, "LOOP END - ERROR");
  return {};
}
void MovistarProtocol::dump(const MovistarData &data) {
  ESP_LOGD(TAG, "Received Movistar: state=%s command=%s", data.state ? "Press" : "Release", data.command.c_str());
}

}  // namespace remote_base
}  // namespace esphome
