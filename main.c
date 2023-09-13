#include <stdio.h>

#define WINDOW_IMPLEMENTATION
#include "window.h"

#define SPECTRUM_IMPLEMENTATION
#include "spectrum.h"

#define DECODER_IMPLEMENTATION
#include "decoder.h"

#define AUDIO_IMPLEMENTATION
#include "audio.h"

#define THREAD_IMPLEMENTATION
#include "thread.h"

#define VOLUME .05f

Spectrum spec = {0};

#define return_defer(n) do{ result = (n); goto defer; }while(0)

void *audio_thread(void *arg) {
  const char *filepath = arg;

  void *result = NULL;

  FILE *f = fopen(filepath, "rb");
  if(!f) {
    return_defer(NULL);
  }

  Decoder decoder;
  int channels = -1;
  int sample_rate;
  if(!decoder_init(&decoder,
		   decoder_file_read,
		   decoder_file_seek,
		   f,		     
		   DECODER_FMT_FLT, VOLUME, 1152,
		   &channels, &sample_rate)) {
    return_defer(NULL);
  }

  Audio audio = {0};
  if(!audio_init(&audio, AUDIO_FMT_FLT, channels, sample_rate)) {
    return_defer(NULL);
  }

  unsigned char buffer[2][1152 * 2 * 4 * 2];
  int current = 0;
  int samples = 0;

  int out_samples;
  while(decoder_decode(&decoder, &out_samples, buffer[current] + samples * audio.sample_size)) {
    samples += out_samples;
    if(samples > 1024) {
      audio_play(&audio, buffer[current], samples);
      current = 1 - current;

      float (*fs)[2] = (void *) buffer[current];
      for(int i=0;i<samples;i++) {
	//spectrum_push(&spec, (fs[i][0] / VOLUME + 1) / 2);
	spectrum_push(&spec, fs[i][0] / VOLUME );
      }
      
      samples = 0;
    }
    
  }

  audio_block(&audio);

 defer:
  if(audio.sample_size != 0) audio_free(&audio);
  if(f) fclose(f);
  if(channels != -1) decoder_free(&decoder);

  return result;
}

Vec4f vec_from_hsv(float hue, float saturation, float value) {

  float c = value * saturation;
  float x = (float) ( ((int) hue / 60) % 2 ) - 1.f;
  if(x < 0) x *= -1;
  x = c * (1 - x);
  float m = value - c;
  
  switch(((int) hue) / 60) {
  case 0: return vec4f((c+m)*255.f, (x+m)*255.f, (0+m)*255.f, 1.f);
  case 1: return vec4f((x+m)*255.f, (c+m)*255.f, (0+m)*255.f, 1.f);
  case 2: return vec4f((0+m)*255.f, (c+m)*255.f, (x+m)*255.f, 1.f);
  case 3: return vec4f((0+m)*255.f, (x+m)*255.f, (c+m)*255.f, 1.f);
  case 4: return vec4f((x+m)*255.f, (0+m)*255.f, (c+m)*255.f, 1.f);
  case 5: return vec4f((c+m)*255.f, (0+m)*255.f, (x+m)*255.f, 1.f);
  default: return vec4f(0, 0, 0, 0);
  }

}

int mai4n() {
  Window window;
  if(!window_init(&window, 160*5, 90*5, "Spectrum", 0)) {
    return 1;
  }

  Window_Event event;
  while(window.running) {
    while(window_peek(&window, &event)) {
      if(event.type == WINDOW_EVENT_KEYPRESS &&
	 event.as.key == 'q') {
	window.running = false;
      }
    }
    window_renderer_begin(window.width, window.height);

    float hue = 30.f;
    float saturation = 1.f;
    float value = 1.f;

    Vec4f c = vec_from_hsv(hue, saturation, value);

    draw_solid_rect(vec2f(0, 0), vec2f(window.width, window.height), c);

    window_renderer_end();
    window_swap_buffers(&window);
  }

  window_free(&window);
}

int main() {

  memset(&spec, 0, sizeof(spec));


  Window window;
  if(!window_init(&window, 160*5, 90*5, "Spectrum", 0)) {
    return 1;
  }

  Thread id;
  if(!thread_create(&id, audio_thread, "videoplayback.mp4")) {
    return 1;
  }

  Window_Event event;
  while(window.running) {
    while(window_peek(&window, &event)) {
      if(event.type == WINDOW_EVENT_KEYPRESS &&
	 event.as.key == 'q') {
	window.running = false;
      }
    }

    spectrum_analyze(&spec, 1.f / 60.f );

    float widthf = (float) window.width;
    float heightf = (float) window.height;

    window_renderer_begin(window.width, window.height);

    Vec4f c = vec4f(0, 1, 0, 1);

    float cell_width = widthf / spec.m;

    for(size_t i=0;i<spec.m;i++) {
      float t = spec.out_smooth[i];
      //float t = spec.out_log[i];

      draw_solid_rect(vec2f(i*cell_width, 0),
		      vec2f(cell_width, t * 2/3 * heightf),
		      c);
    }
    
    window_renderer_end();

    window_swap_buffers(&window);
  }

  window_free(&window);
  
  return 0;
}
