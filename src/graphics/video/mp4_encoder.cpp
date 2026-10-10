#include <graphics/video/video.hpp>

#include <algorithm>
#include <cstring>
#include <limits>
#include <wels/codec_api.h>

#define MINIMP4_IMPLEMENTATION
#include <minimp4.h>

namespace sinen {
struct VideoWriter::Mp4State {
  ISVCEncoder *encoder = nullptr;
  MP4E_mux_t *mux = nullptr;
  int track = -1;
  bool failed = false;
  SEncParamExt parameters{};
  Array<UInt8> yuv;

  ~Mp4State() {
    if (mux) {
      MP4E_close(mux);
    }
    if (encoder) {
      encoder->Uninitialize();
      WelsDestroySVCEncoder(encoder);
    }
  }
};

namespace {
int writeMp4(int64_t offset, const void *data, size_t size, void *token) {
  auto &stream = *static_cast<std::ofstream *>(token);
  if (offset < 0 ||
      size > static_cast<size_t>(std::numeric_limits<std::streamsize>::max())) {
    return 1;
  }
  stream.seekp(static_cast<std::streamoff>(offset));
  stream.write(static_cast<const char *>(data),
               static_cast<std::streamsize>(size));
  return stream ? 0 : 1;
}

UInt8 clampByte(int value) {
  return static_cast<UInt8>(std::clamp(value, 0, 255));
}
} // namespace

bool VideoWriter::openMp4() {
  mp4 = std::make_shared<Mp4State>();
  if (WelsCreateSVCEncoder(&mp4->encoder) != 0 || !mp4->encoder ||
      mp4->encoder->GetDefaultParams(&mp4->parameters) != 0) {
    setError("failed to create OpenH264 encoder");
    mp4->failed = true;
    return false;
  }
  auto &p = mp4->parameters;
  p.iUsageType = CAMERA_VIDEO_REAL_TIME;
  p.iPicWidth = static_cast<int>(info.width);
  p.iPicHeight = static_cast<int>(info.height);
  p.fMaxFrameRate = static_cast<float>(info.fps);
  p.iRCMode = RC_OFF_MODE;
  p.iSpatialLayerNum = 1;
  p.iTemporalLayerNum = 1;
  p.iMultipleThreadIdc = 1;
  p.bEnableFrameSkip = false;
  p.uiIntraPeriod = info.fps * 2;
  p.eSpsPpsIdStrategy = CONSTANT_ID;
  auto &layer = p.sSpatialLayers[0];
  layer.iVideoWidth = p.iPicWidth;
  layer.iVideoHeight = p.iPicHeight;
  layer.fFrameRate = p.fMaxFrameRate;
  layer.uiProfileIdc = PRO_BASELINE;
  layer.iDLayerQp = 26;
  layer.sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;
  if (mp4->encoder->InitializeExt(&p) != 0) {
    setError("failed to initialize OpenH264 encoder");
    mp4->failed = true;
    return false;
  }
  mp4->yuv.resize(static_cast<Size>(info.width) * info.height * 3 / 2);
  mp4->mux = MP4E_open(0, 0, &output, writeMp4);
  if (!mp4->mux) {
    setError("failed to initialize MP4 muxer");
    mp4->failed = true;
    return false;
  }
  MP4E_track_t track{};
  std::memcpy(track.language, "und", 4);
  track.track_media_kind = e_video;
  track.object_type_indication = MP4_OBJECT_TYPE_AVC;
  track.time_scale = info.fps;
  track.default_duration = 1;
  track.u.v.width = p.iPicWidth;
  track.u.v.height = p.iPicHeight;
  mp4->track = MP4E_add_track(mp4->mux, &track);
  if (mp4->track < 0) {
    setError("failed to create MP4 video track");
    mp4->failed = true;
    return false;
  }
  return true;
}

bool VideoWriter::addMp4Frame(const UInt8 *pixels, VideoPixelFormat format,
                              UInt32 strideBytes, int quality) {
  if (mp4->failed || info.frameCount == std::numeric_limits<UInt32>::max()) {
    setError("MP4 encoder cannot accept more frames");
    return false;
  }
  const bool bgr =
      format == VideoPixelFormat::BGR8 || format == VideoPixelFormat::BGRA8;
  const UInt32 channels =
      (format == VideoPixelFormat::RGB8 || format == VideoPixelFormat::BGR8)
          ? 3
          : 4;
  if (!strideBytes) {
    strideBytes = info.width * channels;
  }
  const Size planeSize = static_cast<Size>(info.width) * info.height;
  UInt8 *yPlane = mp4->yuv.data();
  UInt8 *uPlane = yPlane + planeSize;
  UInt8 *vPlane = uPlane + planeSize / 4;
  // BT.601 limited-range I420, averaging each 2x2 block for chroma.
  for (UInt32 y = 0; y < info.height; y += 2) {
    for (UInt32 x = 0; x < info.width; x += 2) {
      int red = 0, green = 0, blue = 0;
      for (UInt32 dy = 0; dy < 2; ++dy) {
        for (UInt32 dx = 0; dx < 2; ++dx) {
          const auto *src = pixels + static_cast<Size>(y + dy) * strideBytes +
                            static_cast<Size>(x + dx) * channels;
          const int r = src[bgr ? 2 : 0], g = src[1], b = src[bgr ? 0 : 2];
          yPlane[static_cast<Size>(y + dy) * info.width + x + dx] =
              clampByte(((66 * r + 129 * g + 25 * b + 128) >> 8) + 16);
          red += r;
          green += g;
          blue += b;
        }
      }
      const Size uv = static_cast<Size>(y / 2) * (info.width / 2) + x / 2;
      uPlane[uv] =
          clampByte(((-38 * red - 74 * green + 112 * blue + 512) >> 10) + 128);
      vPlane[uv] =
          clampByte(((112 * red - 94 * green - 18 * blue + 512) >> 10) + 128);
    }
  }
  const int qp = 51 - (quality * 51 / 100);
  if (mp4->parameters.sSpatialLayers[0].iDLayerQp != qp) {
    mp4->parameters.sSpatialLayers[0].iDLayerQp = qp;
    if (mp4->encoder->SetOption(ENCODER_OPTION_SVC_ENCODE_PARAM_EXT,
                                &mp4->parameters) != 0) {
      setError("failed to set H.264 encoding quality");
      mp4->failed = true;
      return false;
    }
  }
  SSourcePicture picture{};
  picture.iColorFormat = videoFormatI420;
  picture.iPicWidth = static_cast<int>(info.width);
  picture.iPicHeight = static_cast<int>(info.height);
  picture.iStride[0] = picture.iPicWidth;
  picture.iStride[1] = picture.iStride[2] = picture.iPicWidth / 2;
  picture.pData[0] = yPlane;
  picture.pData[1] = uPlane;
  picture.pData[2] = vPlane;
  picture.uiTimeStamp =
      static_cast<long long>(info.frameCount) * 1000 / info.fps;
  SFrameBSInfo encoded{};
  if (mp4->encoder->EncodeFrame(&picture, &encoded) != 0 ||
      encoded.eFrameType == videoFrameTypeSkip) {
    setError("failed to encode H.264 frame");
    mp4->failed = true;
    return false;
  }
  Array<UInt8> sample;
  int status = MP4E_STATUS_OK;
  for (int i = 0; i < encoded.iLayerNum && status == MP4E_STATUS_OK; ++i) {
    const auto &layer = encoded.sLayerInfo[i];
    const UInt8 *nal = layer.pBsBuf;
    for (int j = 0; j < layer.iNalCount && status == MP4E_STATUS_OK; ++j) {
      const int length = layer.pNalLengthInByte[j];
      if (length <= 4) {
        status = MP4E_STATUS_BAD_ARGUMENTS;
        break;
      }
      const UInt32 size = static_cast<UInt32>(length - 4);
      const int type = nal[4] & 31;
      if (type == 7) {
        status = MP4E_set_sps(mp4->mux, mp4->track, nal + 4, size);
      } else if (type == 8) {
        status = MP4E_set_pps(mp4->mux, mp4->track, nal + 4, size);
      } else {
        // One complete access unit per sample, with AVCC length prefixes.
        sample.push_back(static_cast<UInt8>(size >> 24));
        sample.push_back(static_cast<UInt8>(size >> 16));
        sample.push_back(static_cast<UInt8>(size >> 8));
        sample.push_back(static_cast<UInt8>(size));
        sample.insert(sample.end(), nal + 4, nal + length);
      }
      nal += length;
    }
  }
  if (status == MP4E_STATUS_OK && !sample.empty() &&
      sample.size() <= static_cast<Size>(std::numeric_limits<int>::max())) {
    status = MP4E_put_sample(
        mp4->mux, mp4->track, sample.data(), static_cast<int>(sample.size()), 1,
        encoded.eFrameType == videoFrameTypeIDR ? MP4E_SAMPLE_RANDOM_ACCESS
                                                : MP4E_SAMPLE_DEFAULT);
  } else {
    status = MP4E_STATUS_BAD_ARGUMENTS;
  }
  if (status != MP4E_STATUS_OK || !output) {
    setError("failed to write H.264 frame to MP4");
    mp4->failed = true;
    return false;
  }
  ++info.frameCount;
  return true;
}

bool VideoWriter::closeMp4() {
  bool ok = !mp4->failed;
  if (mp4->mux) {
    ok = (MP4E_close(mp4->mux) == MP4E_STATUS_OK) && ok;
    mp4->mux = nullptr;
  }
  // OpenH264 has no delayed frames to drain.
  mp4.reset();
  output.flush();
  ok = static_cast<bool>(output) && ok;
  output.close();
  ok = !output.fail() && ok;
  if (!ok && error.empty()) {
    setError("failed to finalize MP4 file");
  }
  return ok;
}
} // namespace sinen
