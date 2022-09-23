
cbuffer PerObjectBuffer : register(b10)
{
    matrix model;
    matrix modelView;
    matrix modelViewProjection;
    matrix normalMatrix;
}

cbuffer PerFrameBuffer : register(b11)
{
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix viewProjectionMatrix;
}

cbuffer ShadowBuffer : register(b12)
{
    matrix lightSpaceMatrix;
}

cbuffer BoneBuffer : register(b13)
{
    matrix bones[80];
}