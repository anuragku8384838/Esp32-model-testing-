
#include <Arduino.h>
#include <Wire.h>
#include <I2S.h>
#include <edge-impulse-sdk/classifier/ei_run_classifier.h>

#define I2S_WS  15
#define I2S_SD  32
#define I2S_SCK 14

ei_impulse_result_t result;

void setup() {
  Serial.begin(115200);

  // Setup I2S Microphone
  I2S.begin(I2S_PHILIPS_MODE, 16000, 32);
  I2S.setPins(I2S_SCK, I2S_WS, I2S_SD);

  if (!I2S.begin()) {
    Serial.println("Failed to initialize I2S!");
    while (1);
  }

  Serial.println("Edge Impulse model test with INMP441 on ESP32");
}

void loop() {
  const int sample_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  static int16_t buffer[sample_length];
  int bytesRead = I2S.read((void*)buffer, sample_length * sizeof(int16_t));

  if (bytesRead <= 0) {
    Serial.println("Failed to read data from I2S.");
    delay(1000);
    return;
  }

  signal_t signal;
  int err = numpy::signal_from_buffer(buffer, sample_length, &signal);
  if (err != 0) {
    ei_printf("Failed to create signal from buffer (%d)
", err);
    return;
  }

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) {
    ei_printf("run_classifier returned: %d
", res);
    return;
  }

  ei_printf("Predictions:
");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("%s: %.5f
", result.classification[ix].label, result.classification[ix].value);
  }

  delay(2000);
}
