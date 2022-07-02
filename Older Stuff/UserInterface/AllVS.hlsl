static float4 verts[6] = {
    { -1, 1, 0, 1 },
    { 1, 1, 0, 1 },
    { -1, -1, 0, 1 },
    
    { 1, 1, 0, 1 },
    { 1, -1, 0, 1 },
    { -1, -1, 0, 1 },
};

float4 main( uint idx : SV_VertexID) : SV_POSITION
{
    return verts[idx];
}