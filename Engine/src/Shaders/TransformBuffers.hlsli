
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
    matrix viewProjectionMatrix;
}

cbuffer ShadowBuffer : register(b12)
{
    matrix lightSpaceMatrix;
}