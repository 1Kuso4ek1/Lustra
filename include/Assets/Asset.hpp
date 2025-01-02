#pragma once

namespace dev
{

struct Asset
{
    virtual ~Asset() = default;
};

struct DummyAsset : public Asset
{
    int data = 0;
};

struct AnotherDummyAsset : public Asset
{
    int data = 0;
};

}
