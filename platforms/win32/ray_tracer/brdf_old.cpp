static void
LoadNullBrdfTable(brdf_table *Result, f64 *DummySample)
{
    Result->Dimensions[0] = 1;
    Result->Dimensions[1] = 1;
    Result->Dimensions[2] = 1;
    Result->Samples = DummySample;
    Result->TotalSampleCount = 1;
}

static void
LoadBrdfTableFromFile(char *FileName, brdf_table *Result)
{
    FILE *File = fopen(FileName, "rb");

    fread(Result->Dimensions, sizeof(Result->Dimensions), 1, File);
    Result->TotalSampleCount =
        Result->Dimensions[0] *
        Result->Dimensions[1] *
        Result->Dimensions[2];

    Assert
    (
        Result->TotalSampleCount ==
        (
            BRDF_RESOLUTION_THETA_HALF *
            BRDF_RESOLUTION_THETA_DIFF *
            BRDF_RESOLUTION_PHI_DIFF / 2
        )
    );

    u32 TotalReadSize = Result->TotalSampleCount * sizeof(f64) * 3;
    Result->Samples = (f64 *)malloc(TotalReadSize);

    fread(Result->Samples, TotalReadSize, 1, File);
    fclose(File);
}

inline u32 ThetaHalfIndex(f64 ThetaHalf, u32 Stride)
{
    if (ThetaHalf <= 0.0f)
    {
        return 0;
    }

    f64 IndexF64 = SquareRoot(ThetaHalf / (0.5f * PI32)) * (f64)Stride;
    u32 Index = Clamp(RoundF32ToU32((f32)IndexF64), 0, Stride - 1);
    return Index;
}

inline u32 ThetaDiffIndex(f64 ThetaDiff, u32 Stride)
{
    f64 IndexF64 = (ThetaDiff / (0.5f * PI32)) * (f64)Stride;
    u32 Index = Clamp(RoundF32ToU32((f32)IndexF64), 0, (Stride - 1));
    return Index;
}

inline u32 PhiDiffIndex(f64 PhiDiff, u32 Stride)
{
    if (PhiDiff < 0.0f)
    {
        PhiDiff += PI32;
    }

    f64 IndexF64 = (PhiDiff / PI32) * Stride;
    u32 Index = Clamp(RoundF32ToU32((f32)IndexF64), 0, (Stride - 1));
    return Index;
}

static v3
BrdfTableLookupSingleVector
(
    brdf_table *Table,
    v3 Tangent, v3 BiTangent, v3 Normal,
    v3 Incoming, v3 Outgoing
)
{
    v3 Halfway = Normalize((Incoming + Outgoing) / 2.0f);

    f64 ThetaIncoming = AcosF64(InnerProduct(Incoming, Normal));
    f64 ThetaHalf = AcosF64(InnerProduct(Halfway, Normal));
    f64 ThetaDiff = ThetaIncoming - ThetaHalf;

    f64 PhiIncoming = Atan2F64
    (
        InnerProduct(Incoming, BiTangent),
        InnerProduct(Incoming, Tangent)
    );
    f64 PhiHalf = Atan2F64
    (
        InnerProduct(Halfway, BiTangent),
        InnerProduct(Halfway, Tangent)
    );
    f64 PhiDiff = PhiIncoming - PhiHalf;

    u32 SampleIndex =
        PhiDiffIndex(PhiDiff, Table->Dimensions[2]) +
        ThetaDiffIndex(ThetaDiff, Table->Dimensions[1]) * Table->Dimensions[2] +
        ThetaHalfIndex(ThetaHalf, Table->Dimensions[0]) * Table->Dimensions[2] * Table->Dimensions[1];

    u32 GreenSamplesStartOffset =
        Table->Dimensions[0] *
        Table->Dimensions[1] *
        Table->Dimensions[2];

    u32 BlueSamplesStartOffset =
        Table->Dimensions[0] *
        Table->Dimensions[1] *
        2 * Table->Dimensions[2];

    v3 Result = V3
    (
        (f32)Table->Samples[SampleIndex] * RED_SCALE,
        (f32)Table->Samples[GreenSamplesStartOffset + SampleIndex] * GREEN_SCALE,
        (f32)Table->Samples[BlueSamplesStartOffset + SampleIndex] * BLUE_SCALE
    );
    return Result;
}

static v3_lane
BrdfTableLookup
(
    material *Materials, u32_lane MaterialIndex,
    v3_lane Tangent, v3_lane BiTangent, v3_lane Normal,
    v3_lane Incoming, v3_lane Outgoing
)
{
    v3_lane Result = {};

    for (u32 SubElementIndex = 0; SubElementIndex < SIMD_NUMBEROF_LANES; SubElementIndex++)
    {
        brdf_table *BrdfTable = &Materials[U32FromU32Lane(MaterialIndex, SubElementIndex)].BrdfTable;
        v3 TangentSingle = V3FromV3Lane(Tangent, SubElementIndex);
        v3 BiTangentSingle = V3FromV3Lane(BiTangent, SubElementIndex);
        v3 NormalSingle = V3FromV3Lane(Normal, SubElementIndex);
        v3 IncomingSingle = V3FromV3Lane(Incoming, SubElementIndex);
        v3 OutgoingSingle = V3FromV3Lane(Outgoing, SubElementIndex);

        v3 Color = BrdfTableLookupSingleVector
        (
            BrdfTable,
            TangentSingle, BiTangentSingle, NormalSingle,
            IncomingSingle, OutgoingSingle
        );

        ((f32 *)&Result.X)[SubElementIndex] = Color.Red;
        ((f32 *)&Result.Y)[SubElementIndex] = Color.Green;
        ((f32 *)&Result.Z)[SubElementIndex] = Color.Blue;
    }

    return Result;
}