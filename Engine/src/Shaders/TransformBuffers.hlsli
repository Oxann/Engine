
cbuffer PerObject : register(b10)
{
#ifdef MODELMATRIX
    matrix model;
#endif

#ifdef MODELVIEWMATRIX
    matrix modelView;
#endif
    
#ifdef MODELVIEWPROJECTIONMATRIX
    matrix modelViewProjection;
#endif
    
#ifdef NORMALMATRIX
    matrix normalMatrix;
#endif
}

cbuffer PerFrame : register(b11)
{
#ifdef VIEWMATRIX
    matrix viewMatrix;
#endif

#ifdef VIEWPROJECTIONMATRIX
    matrix viewProjectionMatrix;
#endif
}