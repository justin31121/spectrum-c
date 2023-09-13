#ifndef DECODER_H
#define DECODER_H

// TODO: implement seeking / seeking-api
// TODO: Maybe add decoder_url_read / decoder_url_seek

// win32
//   mingw: -lavformat -lavcodec -lavutil -lswresample
//   msvc : avformat.lib avcodec.lib avutil.lib swresample.lib

// linux
//   gcc  : -lavformat -lavcodec -lavutil -lswresample

#include <stdbool.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

#ifndef DECODER_DEF
#  define DECODER_DEF static inline
#endif //DECODER_DEF

typedef enum {
  DECODER_FMT_NONE = 0,
  DECODER_FMT_U8,
  DECODER_FMT_S16,
  DECODER_FMT_S32,
  DECODER_FMT_FLT,
  DECODER_FMT_DBL,
  DECODER_FMT_U8P,
  DECODER_FMT_S16P,
  DECODER_FMT_S32P,
  DECODER_FMT_FLTP,
  DECODER_FMT_DBLP,
}Decoder_Fmt;

typedef struct Decoder Decoder;

typedef int (*Decoder_Read)(void *opaque, uint8_t* buffer, int buffer_size);
typedef int64_t (*Decoder_Seek)(void *opaque, int64_t offset, int whence);

typedef struct{
  const unsigned char *data;
  uint64_t size;
  uint64_t pos;
}Decoder_Memory;

struct Decoder{
  AVIOContext *av_io_context;    
  AVFormatContext *av_format_context;
  AVCodecContext *av_codec_context;
  SwrContext *swr_context;
  int stream_index;

  AVPacket *packet;
  AVFrame *frame;
  int64_t pts;

  float volume;
  float target_volume;

  int samples;
  int sample_size;

  bool continue_receive;
  bool continue_convert;
};

// Public
DECODER_DEF bool decoder_slurp(Decoder_Read read,
			       Decoder_Seek seek,
			       void *opaque,
			       Decoder_Fmt fmt,
			       float volume,
			       int *channels,
			       int *sample_rate,
			       unsigned char **samples,
			       unsigned int *samples_count);

DECODER_DEF bool decoder_slurp_file(const char *filepath,
				    Decoder_Fmt fmt,
				    float volume,
				    int *channels,
				    int *sample_rate,
				    unsigned char **samples,
				    unsigned int *samples_count);

DECODER_DEF bool decoder_slurp_memory(const char *memory,
				      size_t memory_len,
				      Decoder_Fmt fmt,
				      float volume,
				      int *channels,
				      int *sample_rate,
				      unsigned char **samples,
				      unsigned int *samples_count);

DECODER_DEF bool decoder_init(Decoder *decoder,
			      Decoder_Read read,
			      Decoder_Seek seek,
			      void *opaque,
			      Decoder_Fmt fmt,
			      float volume,
			      int samples,
			      int *channels,
			      int *sample_rate);
DECODER_DEF bool decoder_decode(Decoder *decoder, int *out_samples, unsigned char *out_buf);
DECODER_DEF void decoder_free(Decoder *decoder);
DECODER_DEF bool decoder_fmt_to_bits_per_sample(int *bits, Decoder_Fmt fmt);
DECODER_DEF bool decoder_fmt_to_libav_fmt(enum AVSampleFormat *av_fmt, Decoder_Fmt fmt);

// Protected
DECODER_DEF int64_t decoder_file_seek(void *opaque, int64_t offset, int whence);
DECODER_DEF int decoder_file_read(void *opaque, uint8_t *buf, int _buf_size);
  
DECODER_DEF int64_t decoder_memory_seek(void *opaque, int64_t offset, int whence);
DECODER_DEF int decoder_memory_read(void *opaque, uint8_t *buf, int _buf_size);

#ifdef DECODER_IMPLEMENTATION

DECODER_DEF bool decoder_slurp_memory(const char *memory,
				      size_t memory_len,
				      Decoder_Fmt fmt,
				      float volume,
				      int *channels,
				      int *sample_rate,
				      unsigned char **out_samples,
				      unsigned int *out_samples_count) {
  Decoder_Memory mem = {
    .data = (const unsigned char *) memory,
    .pos = 0,
    .size = memory_len,
  };

  return decoder_slurp(decoder_memory_read,
		       decoder_memory_seek,
		       &mem,
		       fmt,
		       volume,
		       channels,
		       sample_rate,
		       out_samples,
		       out_samples_count);

}

DECODER_DEF bool decoder_slurp_file(const char *filepath,
				    Decoder_Fmt fmt,
				    float volume,
				    int *channels,
				    int *sample_rate,
				    unsigned char **out_samples,
				    unsigned int *out_samples_count) {
  FILE *f = fopen(filepath, "rb");
  if(!f) {
    return false;
  }
  
  if(!decoder_slurp(decoder_file_read,
		    decoder_file_seek,
		    f,
		    fmt,
		    volume,
		    channels,
		    sample_rate,
		    out_samples,
		    out_samples_count)) {
    fclose(f);
    return false;
  }

  fclose(f);
  return true;
}

DECODER_DEF bool decoder_slurp(Decoder_Read read,
			       Decoder_Seek seek,
			       void *opaque,
			       Decoder_Fmt fmt,
			       float volume,
			       int *channels,
			       int *sample_rate,
			       unsigned char **out_samples,
			       unsigned int *out_samples_count) {
  Decoder decoder;
  if(!decoder_init(&decoder, read, seek, opaque,
		   fmt, 1152, volume, channels, sample_rate)) {
    return false;
  }

  unsigned int samples_count = 0;
  unsigned int samples_cap = 5 * (*sample_rate) * (*channels);
  unsigned char *samples = malloc(samples_cap * decoder.sample_size);
  if(!samples) {
    decoder_free(&decoder);
    return false;
  }

  unsigned char decoded_samples[1152 * 4];
  int decoded_samples_count;
  while(decoder_decode(&decoder, &decoded_samples_count, decoded_samples)) {

    unsigned int new_samples_cap = samples_cap;
    while(samples_count + decoded_samples_count > new_samples_cap) {
      new_samples_cap *= 2;
    }
    if(new_samples_cap != samples_cap) {
      samples_cap = new_samples_cap;
      samples = realloc(samples, samples_cap * decoder.sample_size);
      if(!samples) {
	decoder_free(&decoder);
	return false;
      }
    }
    
    memcpy(samples + samples_count * decoder.sample_size,
	   decoded_samples,
	   decoded_samples_count * decoder.sample_size);

    samples_count += (unsigned int) decoded_samples_count;
  }

  *out_samples = samples;
  *out_samples_count = samples_count;

  decoder_free(&decoder);  
  return true;
}

DECODER_DEF bool decoder_init(Decoder *decoder,
			      Decoder_Read read,
			      Decoder_Seek seek,
			      void *opaque,
			      Decoder_Fmt fmt,
			      float volume,
			      int samples,
			      int *channels,
			      int *sample_rate) {

  decoder->av_io_context = NULL;
  decoder->av_format_context = NULL;
  decoder->av_codec_context = NULL;
  decoder->swr_context = NULL;
  decoder->packet = NULL;
  decoder->frame = NULL;
  decoder->target_volume = -1.f;
  decoder->volume = volume;

  decoder->samples = samples;
  enum AVSampleFormat av_sample_format;
  if(!decoder_fmt_to_libav_fmt(&av_sample_format, fmt)) {
      return false;
  }

  decoder->av_io_context = avio_alloc_context(NULL, 0, 0, opaque, read, NULL, seek);
  if(!decoder->av_io_context) {
    decoder_free(decoder);
    return false;
  }

  decoder->av_format_context = avformat_alloc_context();
  if(!decoder->av_format_context) {
    decoder_free(decoder);
    return false;
  }

  decoder->av_format_context->pb = decoder->av_io_context;
  decoder->av_format_context->flags = AVFMT_FLAG_CUSTOM_IO;
  if (avformat_open_input(&decoder->av_format_context, "", NULL, NULL) != 0) {
    decoder_free(decoder);
    return false;
  }

  if(avformat_find_stream_info(decoder->av_format_context, NULL) < 0) {
    decoder_free(decoder);
    return false;
  }

  decoder->stream_index = -1;

  const AVCodec *av_codec = NULL;
  AVCodecParameters *av_codec_parameters = NULL;
  for(size_t i=0;i<decoder->av_format_context->nb_streams;i++) {
    av_codec_parameters = decoder->av_format_context->streams[i]->codecpar;
    if(av_codec_parameters->codec_type == AVMEDIA_TYPE_AUDIO) {
      decoder->stream_index = (int) i;
      av_codec = avcodec_find_decoder(av_codec_parameters->codec_id);
      if(!av_codec) {
	decoder_free(decoder);
	return false;
      }
      break;
    }
  }
  if(av_codec == NULL) {
    decoder_free(decoder);
    return false;
  }
  
  decoder->av_codec_context = avcodec_alloc_context3(av_codec);
  if(!decoder) {
    decoder_free(decoder);
    return false;
  }

  if(avcodec_parameters_to_context(decoder->av_codec_context, av_codec_parameters) < 0) {
    decoder_free(decoder);
    return false;
  }

  *sample_rate = (int) decoder->av_codec_context->sample_rate;

  if(avcodec_open2(decoder->av_codec_context, av_codec, NULL) < 0) {
    decoder_free(decoder);
    return false;
  }
  
  decoder->swr_context = swr_alloc();
  if(!decoder->swr_context) {
    decoder_free(decoder);
    return false;
  }
  
  char chLayoutDescription[128];

  // I have received different compile errors/warnings, due to 
  // different libav-versions. Use one of the following implementations:

  // BEGIN FIRST
  int sts = av_channel_layout_describe(&av_codec_parameters->ch_layout, chLayoutDescription, sizeof(chLayoutDescription));
  if(sts < 0) {
    return false;
  }

  if(strcmp(chLayoutDescription, "stereo") == 0) {
    *channels = 2;
  } else if(strcmp(chLayoutDescription, "mono") == 0) {
    *channels = 1;    
  } else if(strcmp(chLayoutDescription, "2 channels") == 0) {
    memcpy(chLayoutDescription, "stereo", 7);
    *channels = 2;
  } else if(strcmp(chLayoutDescription, "1 channel")) {
  memcpy(chLayoutDescription, "mono", 5);
    *channels = 1;
  } else {
    return false;
  }
  // END FIRST

  /*
  // BEGIN SECOND
  uint64_t ch_layout = av_codec_parameters->channel_layout;
  if(ch_layout & AV_CH_LAYOUT_MONO) {
    memcpy(chLayoutDescription, "mono", 5);
    *channels = 2;
  } else if (ch_layout & AV_CH_LAYOUT_STEREO) {
    memcpy(chLayoutDescription, "stereo", 7);
    *channels = 2;
  } else {
    return false;
  }
  // END SECOND
  */

  av_opt_set(decoder->swr_context, "in_channel_layout", chLayoutDescription, 0);
  av_opt_set(decoder->swr_context, "out_channel_layout", chLayoutDescription, 0);
  av_opt_set_int(decoder->swr_context, "in_sample_fmt", decoder->av_codec_context->sample_fmt, 0);
  av_opt_set_int(decoder->swr_context, "in_sample_rate", decoder->av_codec_context->sample_rate, 0);
  av_opt_set_int(decoder->swr_context, "out_sample_fmt", av_sample_format, 0);
  av_opt_set_int(decoder->swr_context, "out_sample_rate", decoder->av_codec_context->sample_rate, 0);
  av_opt_set_double(decoder->swr_context, "rmvol", volume, 0);
  
  decoder->target_volume = volume;
  decoder->volume = volume;
    
  if(swr_init(decoder->swr_context) < 0) {
    decoder_free(decoder);
    return false;
  }
  //swr_set_quality(decoder->swr_context, 7);
  //swr_set_resample_mode(decoder->swr_context, SWR_FILTER_TYPE_CUBIC);

  int bits_per_sample;
  if(!decoder_fmt_to_bits_per_sample(&bits_per_sample, fmt)) {
    return false;
  }
  decoder->sample_size = *channels * bits_per_sample / 8;
    
  decoder->packet = av_packet_alloc();
  if(!decoder->packet) {
    decoder_free(decoder);
    return false;
  }
  
  decoder->frame = av_frame_alloc();
  if(!decoder->frame) {
    decoder_free(decoder);
    return false;
  }
  
  return true;    
}

DECODER_DEF void decoder_free(Decoder *decoder) {
  
  decoder->continue_receive = false;
  decoder->continue_convert = false;

  if(decoder->frame) {
    av_frame_free(&decoder->frame);
    decoder->frame = NULL;    
  }
  
  if(decoder->packet) {
    av_packet_free(&decoder->packet);
    decoder->packet = NULL;    
  }

  if(decoder->swr_context) {
    swr_free(&decoder->swr_context);
    decoder->swr_context = NULL;    
  }

  if(decoder->av_codec_context) {
    avcodec_close(decoder->av_codec_context);
    avcodec_free_context(&decoder->av_codec_context);
    decoder->av_codec_context = NULL;    
  }

  if(decoder->av_format_context) {
    avformat_close_input(&decoder->av_format_context);
    decoder->av_format_context = NULL;    
  }

  if(decoder->av_io_context) {
    avio_context_free(&decoder->av_io_context);
    decoder->av_io_context = NULL;
  }
}

DECODER_DEF bool decoder_decode(Decoder *decoder, int *out_samples, unsigned char *buffer) {
  *out_samples = 0;
  if(!decoder->continue_convert) {
    
    if(!decoder->continue_receive) {
      if(av_read_frame(decoder->av_format_context, decoder->packet) < 0) {
	decoder->continue_receive = false;
	decoder->continue_convert = false;
	return false;
      }
      if(decoder->packet->stream_index != decoder->stream_index) {
	decoder->continue_receive = false;
	decoder->continue_convert = false;

	av_packet_unref(decoder->packet);
	return true;
      }
    
      decoder->continue_receive = true;

      if(avcodec_send_packet(decoder->av_codec_context, decoder->packet) < 0) {
	//fprintf(stderr, "ERROR: fatal error in avcodec_send_packet\n");
	//exit(1);
	return false;
      }
    }  

    if(avcodec_receive_frame(decoder->av_codec_context, decoder->frame) >= 0) {

      if(decoder->target_volume != decoder->volume) {
	av_opt_set_double(decoder->swr_context, "rmvol", decoder->target_volume, 0);
	double volume;
	swr_init(decoder->swr_context);
	av_opt_get_double(decoder->swr_context, "rmvol", 0, &volume);
	decoder->volume = (float) volume;
      }

      decoder->pts = decoder->frame->pts;
      
      *out_samples = swr_convert(decoder->swr_context, &buffer, decoder->samples,
				 (const unsigned char **) (decoder->frame->data),
				 decoder->frame->nb_samples);
      
      if(*out_samples > 0) {
	decoder->continue_convert = true;
      } else {
	decoder->continue_convert = false;

	av_frame_unref(decoder->frame);
      }
    } else {
      *out_samples = 0;
      
      decoder->continue_convert = false;
      decoder->continue_receive = false;

      av_packet_unref(decoder->packet);
    }

    return true;
  }
      
  *out_samples = swr_convert(decoder->swr_context, &buffer, decoder->samples, NULL, 0);

  if(*out_samples > 0) {
    decoder->continue_convert = true;
  } else {
    decoder->continue_convert = false;    
    av_packet_unref(decoder->packet);
  }

  return true;

}


DECODER_DEF bool decoder_fmt_to_libav_fmt(enum AVSampleFormat *av_fmt, Decoder_Fmt fmt) {
  switch(fmt) {
  case DECODER_FMT_S16: {
    *av_fmt = AV_SAMPLE_FMT_S16;
    return true;
  } break;
  case DECODER_FMT_S32: {
    *av_fmt = AV_SAMPLE_FMT_S32;
    return true;
  } break;
  case DECODER_FMT_FLT: {
    *av_fmt = AV_SAMPLE_FMT_FLT;
    return true;
  } break;
  default: {
    return false;
  } 
  }
}

DECODER_DEF bool decoder_fmt_to_bits_per_sample(int *bits, Decoder_Fmt fmt) {
  switch(fmt) {
  case DECODER_FMT_S16: {
    *bits = 16;
    return true;
  } break;
  case DECODER_FMT_S32: {
    *bits = 32;
    return true;
  } break;
  case DECODER_FMT_FLT: {
    *bits = 32;
    return true;
  } break;
  default: {
    return false;
  } 
  }  
}

DECODER_DEF int decoder_memory_read(void *opaque, uint8_t *buf, int _buf_size) {
  Decoder_Memory *memory = (Decoder_Memory *) opaque;

  size_t buf_size = (size_t) _buf_size;

  if (buf_size > memory->size - memory->pos) {
    buf_size = memory->size - memory->pos;
  }

  if (buf_size <= 0) {
    return AVERROR_EOF;
  }

  memcpy(buf, memory->data + memory->pos, buf_size);
  memory->pos += buf_size;

  return (int )buf_size;
}

DECODER_DEF int64_t decoder_memory_seek(void *opaque, int64_t offset, int whence) {

  Decoder_Memory *memory = (Decoder_Memory *) opaque;

  switch (whence) {
  case SEEK_SET:
    memory->pos = offset;
    break;
  case SEEK_CUR:
    memory->pos += offset;
    break;
  case SEEK_END:
    memory->pos = memory->size + offset;
    break;
  case AVSEEK_SIZE:
    return (int64_t) memory->size;
  default:
    return AVERROR_INVALIDDATA;
  }

  if (memory->pos > memory->size) {
    return AVERROR(EIO);
  }
    
  return memory->pos;
}

DECODER_DEF int decoder_file_read(void *opaque, uint8_t *buf, int buf_size) {
  FILE *f = (FILE *)opaque;

  size_t bytes_read = fread(buf, 1, buf_size, f);

  if (bytes_read == 0) {
    if(feof(f)) return AVERROR_EOF;
    else return AVERROR(errno);
  }
  
  return (int) bytes_read;
}

DECODER_DEF int64_t decoder_file_seek(void *opaque, int64_t offset, int whence) {
  
  FILE *f = (FILE *)opaque;
  
  if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
    return AVERROR_INVALIDDATA;
  }

  if(fseek(f, (long) offset, whence)) {
    return AVERROR(errno);
  }

  return ftell(f);
}

#endif //DECODER_IMPLEMENTATION

#endif //DECODER_H
