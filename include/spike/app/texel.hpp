#pragma once
#include "spike/app_context.hpp"
#include <memory>

enum class TexelContextFormat {
  DDS_Legacy,
  DDS,
  QOI_BMP,
  QOI,
};

struct TexelConf {
  TexelContextFormat outputFormat = TexelContextFormat::DDS_Legacy;
  bool cubemapToEquirectangular = true;
  bool processMipMaps = false;
  void ReflectorTag();
};

struct NewTexelContextImpl : NewTexelContext {
  NewTexelContextCreate ctx;
  AFileInfo pathOverride;
  AppContext *actx = nullptr;
  AppExtractContext *ectx = nullptr;
  NewTexelContextImpl(NewTexelContextCreate ctx_) : ctx(ctx_) {}

  virtual void Finish() = 0;

  TexelDataLayout ComputeTraditionalDataLayout(
      TexelInputFormatType *typeOverrides = nullptr) override;

  void ProcessContextData();
};

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx, AppContext *actx);

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx, AppExtractContext *ectx, const std::string &path);
