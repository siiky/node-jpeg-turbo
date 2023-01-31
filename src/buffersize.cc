#include "exports.h"
using namespace Nan;
using namespace v8;

static char errStr[NJT_MSG_LENGTH_MAX] = "No error";
#define _throw(m) {snprintf(errStr, NJT_MSG_LENGTH_MAX, "%s", m); retval=-1; goto bailout;}

NAN_METHOD(BufferSize) {
  auto ctx = Nan::GetCurrentContext();
  int retval = 0;

  uint32_t dstLength = 0;
  uint32_t height = 0;
  uint32_t width = 0;
  uint32_t jpegSubsamp = NJT_DEFAULT_SUBSAMPLING;
  Local<Value> widthObject;
  Local<Value> heightObject;
  Local<Value> sampObject;
  Local<Object> options;

  // Input
  Callback *callback = NULL;

  // Try to find callback here, so if we want to throw something we can use callback's err
  if (info[info.Length() - 1]->IsFunction()) {
    callback = new Callback(info[info.Length() - 1].As<Function>());
  }

  if ((NULL != callback && info.Length() < 2) || (NULL == callback && info.Length() < 1)) {
    _throw("Too few arguments");
  }

  // Options
  options = info[0].As<Object>();
  if (!options->IsObject()) {
    _throw("Options must be an Object");
  }

  // Subsampling
  sampObject = options->Get(ctx, New("subsampling").ToLocalChecked()).ToLocalChecked();
  if (!sampObject->IsUndefined()) {
    if (!sampObject->IsUint32()) {
      _throw("Invalid subsampling method");
    }
    jpegSubsamp = sampObject->Uint32Value(ctx).ToChecked();
  }

  switch (jpegSubsamp) {
    case SAMP_444:
    case SAMP_422:
    case SAMP_420:
    case SAMP_GRAY:
    case SAMP_440:
      break;
    default:
      _throw("Invalid subsampling method");
  }

  // Width
  widthObject = options->Get(ctx, New("width").ToLocalChecked()).ToLocalChecked();
  if (widthObject->IsUndefined()) {
    _throw("Missing width");
  }
  if (!widthObject->IsUint32()) {
    _throw("Invalid width value");
  }
  width = widthObject->Uint32Value(ctx).ToChecked();

  // Height
  heightObject = options->Get(ctx, New("height").ToLocalChecked()).ToLocalChecked();
  if (heightObject->IsUndefined()) {
    _throw("Missing height");
  }
  if (!heightObject->IsUint32()) {
    _throw("Invalid height value");
  }
  height = heightObject->Uint32Value(ctx).ToChecked();

  // Finally, calculate the buffer size
  dstLength = tjBufSize(width, height, jpegSubsamp);

  // How to return length
  if (NULL != callback) {
    Local<Value> argv[] = {
      Null(),
      New(dstLength)
    };
    callback->Call(2, argv, nullptr);
  }
  else {
    info.GetReturnValue().Set(New(dstLength));
  }


  bailout:
  if (retval != 0) {
    if (NULL == callback) {
      ThrowError(TypeError(errStr));
    }
    else {
      Local<Value> argv[] = {
        New(errStr).ToLocalChecked()
      };
      callback->Call(1, argv, nullptr);
    }
    return;
  }
}
