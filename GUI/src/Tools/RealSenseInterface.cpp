#include "RealSenseInterface.h"
#include <functional>

#ifdef WITH_REALSENSE
RealSenseInterface::RealSenseInterface(int inWidth,int inHeight,int inFps)
  : width(inWidth),
  height(inHeight),
  fps(inFps),
  initSuccessful(true)
{

  rs2::config cfg;
  cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
  cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_RGB8, fps);

  auto dev_list = ctx.query_devices();
  if(dev_list.size() == 0)
  {
    errorText = "No device connected.";
    initSuccessful = false;
    return;
  }

  rs2::device device = dev_list.front();
  dev = &device;

  auto sens = dev->query_sensors();
  for(auto const &sen: sens){
    if(sen.is<rs2::depth_sensor>()){
      depth_sensor = sen;
    }else{
      video_sensor = sen;
    }
  }

  auto sps = depth_sensor.get_stream_profiles();
  for(auto const &sp: sps){
    if (sp.is_default()){
        depth_sensor.open(sp);
        break;
    }
  }
  

  sps = video_sensor.get_stream_profiles();
  for(auto const &sp: sps){
    if(sp.is_default()){
        video_sensor.open(sp);
        break;
    }
  }

  latestDepthIndex.assign(-1);
  latestRgbIndex.assign(-1);
  for(int i = 0; i < numBuffers; i++)
  {
    uint8_t * newImage = (uint8_t *)calloc(width * height * 3,sizeof(uint8_t));
    rgbBuffers[i] = std::pair<uint8_t *,int64_t>(newImage,0);
  }

  for(int i = 0; i < numBuffers; i++)
  {
    uint8_t * newDepth = (uint8_t *)calloc(width * height * 2,sizeof(uint8_t));
    uint8_t * newImage = (uint8_t *)calloc(width * height * 3,sizeof(uint8_t));
    frameBuffers[i] = std::pair<std::pair<uint8_t *,uint8_t *>,int64_t>(std::pair<uint8_t *,uint8_t *>(newDepth,newImage),0);
  }
  std::cout << "Here3" <<std::endl;

  setAutoExposure(true);
  setAutoWhiteBalance(true);

  rgbCallback = new RGBCallback(lastRgbTime,
    latestRgbIndex,
    rgbBuffers);

  depthCallback = new DepthCallback(lastDepthTime,
    latestDepthIndex,
    latestRgbIndex,
    rgbBuffers,
    frameBuffers);

  depth_sensor.start(*depthCallback);
  video_sensor.start(*rgbCallback);
}

RealSenseInterface::~RealSenseInterface()
{
  if(initSuccessful)
  {
    depth_sensor.stop();
    video_sensor.stop();

    for(int i = 0; i < numBuffers; i++)
    {
      free(rgbBuffers[i].first);
    }

    for(int i = 0; i < numBuffers; i++)
    {
      free(frameBuffers[i].first.first);
      free(frameBuffers[i].first.second);
    }

    delete rgbCallback;
    delete depthCallback;
  }
}

void RealSenseInterface::setAutoExposure(bool value)
{
  video_sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE,value);
}

void RealSenseInterface::setAutoWhiteBalance(bool value)
{
  video_sensor.set_option(RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE,value);
}

bool RealSenseInterface::getAutoExposure()
{
  return video_sensor.get_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE);
}

bool RealSenseInterface::getAutoWhiteBalance()
{
  return video_sensor.get_option(RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE);
}
#else

RealSenseInterface::RealSenseInterface(int inWidth,int inHeight,int inFps)
  : width(inWidth),
  height(inHeight),
  fps(inFps),
  initSuccessful(false)
{
  errorText = "Compiled without Intel RealSense library";
}

RealSenseInterface::~RealSenseInterface()
{
}

void RealSenseInterface::setAutoExposure(bool value)
{
}

void RealSenseInterface::setAutoWhiteBalance(bool value)
{
}

bool RealSenseInterface::getAutoExposure()
{
  return false;
}

bool RealSenseInterface::getAutoWhiteBalance()
{
  return false;
}
#endif
