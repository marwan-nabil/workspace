union v2_lane
{
	struct
	{
		f32_lane X;
		f32_lane Y;
	};
	f32_lane E[2];
};

union v3_lane
{
	struct
	{
		f32_lane X;
		f32_lane Y;
		f32_lane Z;
	};
	struct
	{
		f32_lane Red;
		f32_lane Green;
		f32_lane Blue;
	};
	struct
	{
		v2_lane XY;
		f32_lane Ignored0;
	};
	struct
	{
		f32_lane Ignored1;
		v2_lane YZ;
	};
	f32_lane E[3];
};

union v4_lane
{
	struct
	{
		f32_lane X;
		f32_lane Y;
		f32_lane Z;
		f32_lane W;
	};
	struct
	{
		f32_lane Red;
		f32_lane Green;
		f32_lane Blue;
		f32_lane Alpha;
	};
	f32_lane E[4];
};

struct coordinate_set_lane
{
    v3_lane X;
    v3_lane Y;
    v3_lane Z;
};

#define GatherF32(BasePointer, Member, Indices) \
    GatherF32Implementation(&((BasePointer)->Member), sizeof(*(BasePointer)), Indices)

#define GatherV3(BasePointer, Member, Indices) \
    GatherV3Implementation(&((BasePointer)->Member), sizeof(*(BasePointer)), Indices)