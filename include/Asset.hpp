#pragma once

namespace dev
{

class Asset
{
public:
    virtual ~Asset() = default;
};

class DummyAsset : public Asset
{
public:
    int data = 0;
};

class AnotherDummyAsset : public Asset
{
public:
    int data = 0;
};

}
