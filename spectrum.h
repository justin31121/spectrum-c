#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef SPECTRUM_DEF
#  define SPECTRUM_DEF static inline
#endif // SPECTRUM_DEF

#define SPECTRUM_N 8192
#ifndef PI
#  define PI 3.141592653589793f
#endif //PI

typedef struct{
  float real;
  float imag;
}Spectrum_Complex;

SPECTRUM_DEF Spectrum_Complex spectrum_complex_add(Spectrum_Complex za, Spectrum_Complex zb);
SPECTRUM_DEF Spectrum_Complex spectrum_complex_sub(Spectrum_Complex za, Spectrum_Complex zb);
SPECTRUM_DEF Spectrum_Complex spectrum_complex_mul(Spectrum_Complex za, Spectrum_Complex zb);

typedef struct{
  float in_raw[SPECTRUM_N];
  float in_win[SPECTRUM_N];
  Spectrum_Complex out_raw[SPECTRUM_N];
  float out_log[SPECTRUM_N];
  float out_smooth[SPECTRUM_N];
  float out_smear[SPECTRUM_N];

  size_t m;
}Spectrum;

SPECTRUM_DEF void spectrum_push(Spectrum *s, float frame);
SPECTRUM_DEF void spectrum_analyze(Spectrum *s, float dt);

SPECTRUM_DEF void spectrum_fft(float in[], size_t stride, Spectrum_Complex out[], size_t n);
SPECTRUM_DEF float spectrum_amp(Spectrum_Complex z);

#ifdef SPECTRUM_IMPLEMENTATION


SPECTRUM_DEF Spectrum_Complex spectrum_complex_add(Spectrum_Complex za, Spectrum_Complex zb) {
  float a = za.real;
  float b = za.imag;
  float c = zb.real;
  float d = zb.imag;

  return (Spectrum_Complex) { .real = (a+c), .imag = (b+d)};
}

SPECTRUM_DEF Spectrum_Complex spectrum_complex_sub(Spectrum_Complex za, Spectrum_Complex zb) {
  float a = za.real;
  float b = za.imag;
  float c = zb.real;
  float d = zb.imag;

  return (Spectrum_Complex) { .real = (a-c), .imag = (b-d)};
  
}

SPECTRUM_DEF Spectrum_Complex spectrum_complex_mul(Spectrum_Complex za, Spectrum_Complex zb) {
  float a = za.real;
  float b = za.imag;
  float c = zb.real;
  float d = zb.imag;

  return (Spectrum_Complex) { .real = (a*c - b*d), .imag = (a*d + b*c)};
}

/////////////////////////////////////////////////////////////////////////////////

SPECTRUM_DEF void spectrum_push(Spectrum *s, float frame) {
  memmove(s->in_raw, s->in_raw + 1, (SPECTRUM_N - 1)*sizeof(s->in_raw[0]));
  s->in_raw[SPECTRUM_N-1] = frame;
}

SPECTRUM_DEF void spectrum_analyze(Spectrum *s, float dt) {
  // Apply the Hann Window on the Input - https://en.wikipedia.org/wiki/Hann_function
  for (size_t i = 0; i < SPECTRUM_N; ++i) {
    float t = (float)i/(SPECTRUM_N - 1);
    float hann = 0.5 - 0.5*cosf(2*PI*t);
    s->in_win[i] = s->in_raw[i]*hann;
  }

  // FFT
  spectrum_fft(s->in_win, 1, s->out_raw, SPECTRUM_N);

  // "Squash" into the Logarithmic Scale
  float step = 1.06;
  float lowf = 1.0f;
  size_t m = 0;
  float max_amp = 1.0f;
  for (float f = lowf; (size_t) f < SPECTRUM_N/2; f = ceilf(f*step)) {
    float f1 = ceilf(f*step);
    float a = 0.0f;
    for (size_t q = (size_t) f; q < SPECTRUM_N/2 && q < (size_t) f1; ++q) {
      float b = spectrum_amp(s->out_raw[q]);
      if (b > a) a = b;
    }
    if (max_amp < a) max_amp = a;
    s->out_log[m++] = a;
  }

  // Normalize Frequencies to 0..1 range
  for (size_t i = 0; i < m; ++i) {
    s->out_log[i] /= max_amp;
  }

  // Smooth out and smear the values
  for (size_t i = 0; i < m; ++i) {
    float smoothness = 9;
    if(isnan(s->out_smooth[i])) s->out_smooth[i] = 0;
    s->out_smooth[i] += (s->out_log[i] - s->out_smooth[i])*smoothness*dt;
    float smearness = 3;
    s->out_smear[i] += (s->out_smooth[i] - s->out_smear[i])*smearness;
  }

  s->m = m;
}

SPECTRUM_DEF void spectrum_fft(float in[], size_t stride, Spectrum_Complex out[], size_t n) {
  assert(n > 0);

  if (n == 1) {
    out[0].real = in[0];
    out[0].imag = 0.0f;
    return;
  }

  spectrum_fft(in, stride*2, out, n/2);
  spectrum_fft(in + stride, stride*2,  out + n/2, n/2);

  for (size_t k = 0; k < n/2; ++k) {
    float t = (float)k/n;
    float x = -2*PI*t;
    Spectrum_Complex v =
      spectrum_complex_mul((Spectrum_Complex) { .real=cosf(x), .imag=sinf(x) }, out[k + n/2]);
    Spectrum_Complex e = out[k];
    out[k]       = spectrum_complex_add(e, v);
    out[k + n/2] = spectrum_complex_sub(e, v);
  }
}

SPECTRUM_DEF float spectrum_amp(Spectrum_Complex z) {
  float a = z.real;
  float b = z.imag;
  return logf(a*a + b*b);
}


#endif // SPECTRUM_IMPLEMENTATION

#endif // SPECTRUM_H
