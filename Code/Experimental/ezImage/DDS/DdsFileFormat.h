#pragma once

#include <ImageFileFormat.h>

class ezDdsFormat : public ezImageFileFormatBase
{
public:
  virtual ezResult ReadImage(ezStreamReaderBase& stream, ezImage& image, ezStringBuilder& errorOut) const EZ_OVERRIDE;
  virtual ezResult WriteImage(ezStreamWriterBase& stream, const ezImage& image, ezStringBuilder& errorOut) const EZ_OVERRIDE;

  virtual bool IsKnownExtension(const char* szExtension) const EZ_OVERRIDE;
};
