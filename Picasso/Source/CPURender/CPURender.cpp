
#include "CPURender.h"
#include "../Core/Core.h"
#include "../Renderer/FileHelper.h"

//CImage
#include "../CImage/CImg.h"
using namespace cimg_library;
using namespace std;
using namespace Microsoft::WRL;
//using namespace DirectX;

struct FColor
{
	FColor() :R(0), G(0), B(0) {}
	FColor(uint8_t r, uint8_t g, uint8_t b) :R(r), G(g), B(b) {}
	FColor(uint8_t val) :R(val), G(val), B(val) {}
	~FColor() {}
	uint8_t R, G, B;
};
bool IsWhite(const FColor& color)
{
	return color.R > 220 && color.G > 220 && color.B > 220;
}

class FVector2D
{
public:
	FVector2D(float x, float y) : X(x), Y(y) {}
	FVector2D() :X(0.0f), Y(0.0f) {}
	~FVector2D() {}
	FVector2D operator+ (const FVector2D& Ref) { FVector2D ret; ret.X = this->X + Ref.X; ret.Y = this->Y + Ref.Y; return ret; }
	FVector2D operator- (const FVector2D& Ref) { FVector2D ret; ret.X = this->X - Ref.X; ret.Y = this->Y - Ref.Y; return ret; }
	FVector2D operator+ (const FVector2D& Ref)const { FVector2D ret; ret.X = this->X + Ref.X; ret.Y = this->Y + Ref.Y; return ret; }
	FVector2D operator- (const FVector2D& Ref)const { FVector2D ret; ret.X = this->X - Ref.X; ret.Y = this->Y - Ref.Y; return ret; }
	FVector2D operator* (const FVector2D& Ref) { FVector2D ret; ret.X = this->X * Ref.X; ret.Y = this->Y * Ref.Y; return ret; }
	FVector2D operator* (const float& fval) { FVector2D ret; ret.X = this->X * fval; ret.Y = this->Y * fval; return ret; }
	FVector2D operator/ (const FVector2D& Ref) { FVector2D ret; ret.X = this->X / Ref.X; ret.Y = this->Y / Ref.Y; return ret; }
	FVector2D operator/ (const float& fval) { FVector2D ret; ret.X = this->X / fval; ret.Y = this->Y / fval; return ret; }
	void operator= (const FVector2D& Ref) { this->X = Ref.X; this->Y = Ref.Y; }
	bool operator== (const FVector2D& Ref)const { return this->X == Ref.X && this->Y == Ref.Y; }
	bool operator!= (const FVector2D& Ref)const { return this->X != Ref.X || this->Y != Ref.Y; }
	void Print() { cout << "X: " << X << "  " << "Y: " << Y << endl; }
	float X, Y;
};

class FVectorInt2D
{
public:
	FVectorInt2D(int x, int y) : X(x), Y(y) {}
	FVectorInt2D() :X(0), Y(0) {}
	~FVectorInt2D() {}
	FVectorInt2D operator+ (const FVectorInt2D& Ref) { FVectorInt2D ret; ret.X = this->X + Ref.X; ret.Y = this->Y + Ref.Y; return ret; }
	FVectorInt2D operator- (const FVectorInt2D& Ref) { FVectorInt2D ret; ret.X = this->X - Ref.X; ret.Y = this->Y - Ref.Y; return ret; }
	FVectorInt2D operator+ (const FVectorInt2D& Ref)const { FVectorInt2D ret; ret.X = this->X + Ref.X; ret.Y = this->Y + Ref.Y; return ret; }
	FVectorInt2D operator- (const FVectorInt2D& Ref)const { FVectorInt2D ret; ret.X = this->X - Ref.X; ret.Y = this->Y - Ref.Y; return ret; }
	FVectorInt2D operator* (const FVectorInt2D& Ref) { FVectorInt2D ret; ret.X = this->X * Ref.X; ret.Y = this->Y * Ref.Y; return ret; }
	FVectorInt2D operator* (const float& fval) { FVectorInt2D ret; ret.X = this->X * fval; ret.Y = this->Y * fval; return ret; }
	FVectorInt2D operator/ (const FVectorInt2D& Ref) { FVectorInt2D ret; ret.X = this->X / Ref.X; ret.Y = this->Y / Ref.Y; return ret; }
	FVectorInt2D operator/ (const float& fval) { FVectorInt2D ret; ret.X = this->X / fval; ret.Y = this->Y / fval; return ret; }
	void operator= (const FVectorInt2D& Ref) { this->X = Ref.X; this->Y = Ref.Y; }
	bool operator== (const FVectorInt2D& Ref)const { return this->X == Ref.X && this->Y == Ref.Y; }
	bool operator!= (const FVectorInt2D& Ref)const { return this->X != Ref.X || this->Y != Ref.Y; }
	void Print() { cout << "X: " << X << "  " << "Y: " << Y << endl; }
	int X, Y;
};
float distance(const FVectorInt2D& A, const FVectorInt2D& B)
{
	FVectorInt2D Dec = A - B;
	return sqrt(Dec.X * Dec.X + Dec.Y * Dec.Y);
}

int LocToArrayIndex(const FVectorInt2D& PixleLoc, const int& height)
{
	int index = PixleLoc.Y * height + PixleLoc.X;
	if (index > 0 && index < height * height)
	{
		return index;
	}
	else
	{
		LOG("Wrong Pixle Location");
		return 0;
	}
}

FVector2D ConvertPixelLocToUV(const FVectorInt2D& Loc, const int& TextureWidth, const int& TextureHeight)
{
	return FVector2D((float)Loc.X / (float)TextureWidth, (float)Loc.Y / (float)TextureHeight);
}

struct PixleData
{
	FColor color;
	bool bIsInner;
	FVectorInt2D PixelLoc;
};

struct FCPUTexture
{
	FCPUTexture(){ width = 1; height = 1;}
	FCPUTexture(const int w, const int h, FColor ClearColor)
	{
		width = w;
		height = h;
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				PixleData NewData;
				NewData.color = ClearColor;
				NewData.bIsInner = false;
				NewData.PixelLoc = FVectorInt2D(x, y);
				CPUTextureBulkData.push_back(NewData);
			}
		}
	}

	PixleData SampleTexture(FVector2D UV)
	{
		FVectorInt2D PixleLoc = FVectorInt2D(UV.X * width, UV.Y * height);
		return CPUTextureBulkData[PixleLoc.Y * height + PixleLoc.X];
	}

	int width;
	int height;
	vector<PixleData>CPUTextureBulkData;
};

void CPURenderer::RenderImage()
{
	//Begin load texture
	std::string redTexturePath = gSystemPath + "\\Content\\Textures\\SDFAlphaSource.bmp";
	
	FCPUTexture DestTexture(256, 256, FColor(0, 0, 0));

	CImg<uint8_t> LoadSrcTexture(redTexturePath.c_str());
	FCPUTexture SorcTexture(LoadSrcTexture._width, LoadSrcTexture._height, FColor(0, 0, 0));
	cimg_forXY(LoadSrcTexture, x, y)
	{
		PixleData& NewData = SorcTexture.CPUTextureBulkData[SorcTexture.width * y + x];
		NewData.color.R = LoadSrcTexture(x, y, 0);
		NewData.color.G = LoadSrcTexture(x, y, 1);
		NewData.color.B = LoadSrcTexture(x, y, 2);
		NewData.PixelLoc = FVectorInt2D(x, y);
		if (NewData.color.R > 220 && NewData.color.G > 220 && NewData.color.B > 220)
		{
			NewData.bIsInner = true;
		}
		else
		{
			NewData.bIsInner = false;
		}
	}
	//End load texture

	float MaxRadius = 15;
	for (int i = 0; i < DestTexture.CPUTextureBulkData.size(); i++)
	{
		float MinDistance = MaxRadius * 2;

		PixleData& CurColor = DestTexture.CPUTextureBulkData[i];
		const FVectorInt2D& CurPixelLoc = CurColor.PixelLoc;
		FVector2D CurrentPixleUV = ConvertPixelLocToUV(CurPixelLoc, DestTexture.width, DestTexture.height);
		if (SorcTexture.SampleTexture(CurrentPixleUV).bIsInner == true)
		{
			bool bEnd = false;
			for (int x = -MaxRadius; x < MaxRadius; x++)
			{
				for (int y = -MaxRadius; y < MaxRadius; y++)
				{
					FVectorInt2D loc = CurPixelLoc + FVectorInt2D(x, y);
					loc.X = loc.X >= DestTexture.width ? DestTexture.width - 1 : loc.X;
					loc.X = loc.X < 0 ? 0 : loc.X;
					loc.Y = loc.Y >= DestTexture.height ? DestTexture.height - 1 : loc.Y;
					loc.Y = loc.Y < 0 ? 0 : loc.Y;

					FVector2D UV = ConvertPixelLocToUV(loc, DestTexture.width, DestTexture.height);

					if (SorcTexture.SampleTexture(UV).bIsInner == false)
					{
						float NewMinDis = distance(CurPixelLoc, loc);
						MinDistance = NewMinDis < MinDistance ? NewMinDis : MinDistance;
						if (MinDistance <= 1)
						{
							bEnd = true;
							break;
						}
					}
				}
				if (bEnd == true)
				{
					break;
				}
			}

			float Gradient = (float)MinDistance / (float)MaxRadius;
			Gradient = Gradient >= 1.0f ? 1.0f : Gradient;
			CurColor.color = FColor(Gradient * 255, Gradient * 255, Gradient * 255);
		}
		else
		{
			bool bEnd = false;
			for (int x = -MaxRadius; x < MaxRadius; x++)
			{
				for (int y = -MaxRadius; y < MaxRadius; y++)
				{
					FVectorInt2D loc = CurPixelLoc + FVectorInt2D(x, y);
					loc.X = loc.X >= DestTexture.width ? DestTexture.width - 1 : loc.X;
					loc.X = loc.X < 0 ? 0 : loc.X;
					loc.Y = loc.Y >= DestTexture.height ? DestTexture.height - 1 : loc.Y;
					loc.Y = loc.Y < 0 ? 0 : loc.Y;

					FVector2D UV = ConvertPixelLocToUV(loc, DestTexture.width, DestTexture.height);

					if (SorcTexture.SampleTexture(UV).bIsInner == true)
					{
						float NewMinDis = distance(CurPixelLoc, loc);
						MinDistance = NewMinDis < MinDistance ? NewMinDis : MinDistance;
						if (MinDistance <= 1)
						{
							bEnd = true;
							break;
						}
					}
				}
				if (bEnd == true)
				{
					break;
				}
			}

			float Gradient = (float)MinDistance / (float)MaxRadius;
			Gradient = Gradient >= 1.0f ? 1.0f : Gradient;
			Gradient = 1.0f - Gradient;
			CurColor.color = FColor(Gradient * 255, Gradient * 255, Gradient * 255);
		}

		LOG("Finish Image Percentage: " + std::to_string((float)i / (float)DestTexture.CPUTextureBulkData.size() * 100) + "%");
	}

	//Save the texture
	CImg<uint8_t> DestSaveTexture(DestTexture.width, DestTexture.height, 1, 3);
	cimg_forXY(DestSaveTexture, x, y)
	{
		FColor NewColor;
		DestSaveTexture(x, y, 0) = DestTexture.CPUTextureBulkData[y * DestTexture.height + x].color.R;
		DestSaveTexture(x, y, 1) = DestTexture.CPUTextureBulkData[y * DestTexture.height + x].color.G;
		DestSaveTexture(x, y, 2) = DestTexture.CPUTextureBulkData[y * DestTexture.height + x].color.B;
	}
	const string& Path = FileHelper::GetDesktopPath();
	DestSaveTexture.save((Path + "\\Text.bmp").c_str());
}