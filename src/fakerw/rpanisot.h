#pragma once

RwInt8      RpAnisotGetMaxSupportedMaxAnisotropy(void);
RwTexture    *RpAnisotTextureSetMaxAnisotropy(RwTexture *tex, RwInt8 val);
RwInt8       RpAnisotTextureGetMaxAnisotropy(RwTexture *tex);
RwBool       RpAnisotPluginAttach(void);
