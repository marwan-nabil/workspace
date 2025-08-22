inline v3_lane
V3LaneFromV3(v3 A, v3 B, v3 C, v3 D, v3 E, v3 F, v3 G, v3 H)
{
    v3_lane Result =
    {
        F32LaneFromF32(A.X, B.X, C.X, D.X, E.X, F.X, G.X, H.X),
        F32LaneFromF32(A.Y, B.Y, C.Y, D.Y, E.Y, F.Y, G.Y, H.Y),
        F32LaneFromF32(A.Z, B.Z, C.Z, D.Z, E.Z, F.Z, G.Z, H.Z)
    };
    return Result;
}