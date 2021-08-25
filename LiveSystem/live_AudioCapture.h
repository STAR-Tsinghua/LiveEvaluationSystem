#ifndef LIVE_AUDIOCAPTURE_H
#define LIVE_AUDIOCAPTURE_H
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <util_log.h>
#include <live_xData.h>
#include <live_xDataThread.h>

#define DEVICE_FORMAT           ma_format_f32
#define DEVICE_CHANNELS         2
#define DEVICE_SAMPLE_RATE      44100
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  ma_encoder* pEncoder = (ma_encoder*)pDevice->pUserData;
  MA_ASSERT(pEncoder != NULL);

  ma_encoder_write_pcm_frames(pEncoder, pInput, frameCount);

  (void)pOutput;
}

// void data_callback_capture(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
// {
//   AudioCapture* pAudioCapture = (AudioCapture*)pDevice->pUserData;
//   MA_ASSERT(pAudioCapture != NULL);

//   if(pAudioCapture->isExited()) return;

//   int frameSize = ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels);
//   printf("frameCount: %d\n", frameCount);
//   for(int i = 0;i < frameCount; ++i) {
//     pAudioCapture->Push(XData(((char *)pInput + i * frameSize), frameSize, 0));
//   }

//   (void)pOutput;
// }

class AudioCapture: public XDataThread {
public:
  AudioCapture(){};
  ~AudioCapture(){};
  static void data_callback_capture(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    AudioCapture* pAudioCapture = (AudioCapture*)pDevice->pUserData;
    MA_ASSERT(pAudioCapture != NULL);

    if(pAudioCapture->isExited()) return;

    int frameSize = ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels);
    // printf("frameCount: %d, frameSize: %d, pointer: %p\n", frameCount, frameSize, pInput);
    for(int i = 0;i < frameCount; ++i) {
      pAudioCapture->Push(XData(((char *)pInput + i * frameSize), frameSize, GetCurTime()));
    }

    (void)pOutput;
  }
  bool init() {
    ma_result result;
    // encoderConfig = ma_encoder_config_init(ma_resource_format_wav, ma_format_f32, 2, 44100);

    // if(ma_encoder_init_file(filename, &encoderConfig, &encoder) != MA_SUCCESS) {
    //   printf("Failed to initialize output file.\n");
    //   return false;
    // }

    deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format   = DEVICE_FORMAT;
    deviceConfig.capture.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate       = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback     = AudioCapture::data_callback_capture;
    deviceConfig.pUserData        = this;

    result = ma_device_init(NULL, &deviceConfig, &device);
    if(result != MA_SUCCESS) {
      printf("Failed to initialize capture device.\n");
      return false;
    }

    this->maxList = 256;
    return true;
  }
  bool start() {
    ma_result result = ma_device_start(&device);
    if(result != MA_SUCCESS) {
      ma_device_uninit(&device);
      printf("Failed to start device.\n");
      return false;
    }
    return true;
  }
  void drop() {
    this->Stop();
    ma_device_uninit(&device);
    // ma_encoder_uninit(&encoder);
  }
  bool isExited() {
    return this->isExit;
  }
  // ma_encoder_config encoderConfig;
  ma_device_config deviceConfig;
  // ma_encoder encoder;
  ma_device device;
private:
  AudioCapture(AudioCapture&){};
};
#endif // LIVE_AUDIOCAPTURE_H
