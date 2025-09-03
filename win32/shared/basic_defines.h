#pragma once

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define OffsetOf(DataType, Member) ((size_t)&(((DataType *)0)->Member))

#define KiloBytes(Value) ((Value)*1024LL)
#define MegaBytes(Value) ((Value)*KiloBytes(1024LL))
#define GigaBytes(Value) ((Value)*MegaBytes(1024LL))
#define TeraBytes(Value) ((Value)*GigaBytes(1024LL))

#ifdef ENABLE_ASSERTIONS
#   define Assert(Expression) {if(!(Expression)){ *(int *)0 = 0; }}
#   define AssertIsBit(Value) Assert(!((Value) & (~1ull)))
#   define AssertFits(Value, FittingMask) Assert(!((Value) & (~(FittingMask))))
#   define InvalidCodepath Assert(!"InvalidCodepath")
#else
#   define Assert(Expression)
#   define AssertIsBit(Value)
#   define AssertFits(Value, FittingMask)
#   define InvalidCodepath
#endif // ENABLE_ASSERTIONS