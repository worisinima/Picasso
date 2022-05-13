
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

void CPURenderer::RenderImage()
{
	std::string redTexturePath = gSystemPath + "\\Content\\Textures\\SDFAlphaSource.bmp";
	vector<FColor>ReadTextureBulkData;

	CImg<uint8_t> SrcTexture(redTexturePath.c_str());
	int SrcTextureWidth = SrcTexture._width;
	int SrcTextureHeight = SrcTexture._height;
	cimg_forXY(SrcTexture, x, y)
	{
		FColor NewColor;
		NewColor.R = SrcTexture(x, y, 0);
		NewColor.G = SrcTexture(x, y, 1);
		NewColor.B = SrcTexture(x, y, 2);
		ReadTextureBulkData.push_back(std::move(NewColor));
	}

	for (int x = 0; x < SrcTextureWidth; x++)
	{
		for (int y = 0; y < SrcTextureHeight; y++)
		{
			FVectorInt2D Loc = FVectorInt2D(x, y);
			FColor CurColor = ReadTextureBulkData[y * SrcTextureHeight + x];
			if (IsWhite(CurColor))
			{
				
			}
			else
			{

			}

		}
	}

	CImg<uint8_t> DestTexture(SrcTextureWidth, SrcTextureHeight, 1, 3);
	cimg_forXY(DestTexture, x, y)
	{
		FColor NewColor;
		DestTexture(x, y, 0) = ReadTextureBulkData[y * SrcTextureHeight + x].R;
		DestTexture(x, y, 1) = ReadTextureBulkData[y * SrcTextureHeight + x].G;
		DestTexture(x, y, 2) = ReadTextureBulkData[y * SrcTextureHeight + x].B;
	}

	const string& Path = FileHelper::GetDesktopPath();
	DestTexture.save((Path + "\\Text.bmp").c_str());
}