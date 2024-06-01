#pragma once
#include "spike/app_context.hpp"
#include <memory>
#include <variant>

struct TexelConf {
  TexelContextFormat outputFormat = TexelContextFormat::DDS_Legacy;
  bool cubemapToEquirectangular = true;
  bool processMipMaps = false;
  void ReflectorTag();
};

struct TexelOutputContext : TexelOutput {
  AppContext *ctx;
  std::ostream *str = nullptr;

  void SendData(std::string_view data) override;
  void NewFile(std::string filePath) override;
};

struct TexelOutputExtractContext : TexelOutput {
  AppExtractContext *ctx;

  void SendData(std::string_view data) override { ctx->SendData(data); }
  void NewFile(std::string filePath) override { ctx->NewFile(filePath); }
};

using TexelOutputVariant =
    std::variant<TexelOutputContext, TexelOutputExtractContext>;

struct NewTexelContextImpl : NewTexelContext {
  NewTexelContextCreate ctx;
  AFileInfo pathOverride;
  TexelOutputVariant outVariant;
  TexelOutput *outCtx = nullptr;

  NewTexelContextImpl(NewTexelContextCreate ctx_) : ctx(ctx_) {}

  virtual void Finish() = 0;

  TexelDataLayout ComputeTraditionalDataLayout(
      TexelInputFormatType *typeOverrides = nullptr) override;

  void ProcessContextData();
};

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx, AppContext *actx);

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx, AppExtractContext *ectx,
                   const std::string &path);
