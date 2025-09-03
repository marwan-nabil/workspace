inline v3_lane
V3LaneFromV3(v3 A, v3 B, v3 C, v3 D)
{
    v3_lane Result =
    {
        F32LaneFromF32(A.X, B.X, C.X, D.X),
        F32LaneFromF32(A.Y, B.Y, C.Y, D.Y),
        F32LaneFromF32(A.Z, B.Z, C.Z, D.Z)
    };
    return Result;
}