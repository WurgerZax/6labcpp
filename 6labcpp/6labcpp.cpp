#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <list>
struct DOT30
{
	uint32_t data;
	DOT30(uint32_t&& d) : data(d) {}
	constexpr operator long double()
	{
		long double res = data >> 30;
		res += static_cast<long double>(data & 0x7fffffff) / 0x80000000;
		return res;
	}
	constexpr operator double()
	{
		return static_cast<double>(this->operator long double());
	}
};

struct RGBAquad
{
	uint8_t r, g, b, a;
};
class RGB
{
	const uint32_t& maskRed, & maskGreen, & maskBlue, maskAlpha;
	uint8_t shiftRed, shiftGreen, shiftBlue, shiftAlpha;
	public:
	RGB(const uint32_t& r, const uint32_t& g, const uint32_t& b, const uint32_t& a)
		: maskRed(r), maskGreen(g), maskBlue(b), maskAlpha(a)
	{
		shiftRed = 0;
		auto tempRed = ~maskRed;
		while (tempRed & 1)
		{
			++shiftRed;
			tempRed >>= 1;
		}

		shiftGreen = 0;
		auto tempGreen = ~maskGreen;
		while (tempGreen & 1)
		{
			++shiftGreen;
			tempGreen >>= 1;
		}

		shiftBlue = 0;
		auto tempBlue = ~maskBlue;
		while (tempBlue & 1)
		{
			++shiftBlue;
			tempBlue >>= 1;
		}

		shiftAlpha = 0;
		auto tempAlpha = ~maskAlpha;
		while (tempAlpha & 1)
		{
			++shiftAlpha;
			tempAlpha >>= 1;
		}
	}
	void makeRGBAquad(RGBAquad& res, uint32_t data)
	{
		res.r = (data & maskRed) >> shiftRed;
		res.g = (data & maskGreen) >> shiftGreen;
		res.b = (data & maskBlue) >> shiftBlue;
		res.a = (data & maskAlpha) >> shiftAlpha;
	}
	uint32_t saveRGBAquad(const RGBAquad& pixel)
	{
		return
			(static_cast<uint32_t>(pixel.r) << shiftRed) |
			(static_cast<uint32_t>(pixel.g) << shiftGreen) |
			(static_cast<uint32_t>(pixel.b) << shiftBlue) |
			(static_cast<uint32_t>(pixel.a) << shiftAlpha);
	}
};

using RGBAdata = std::vector<std::vector<RGBAquad>>;

bool finish(const char* message)
{
	std::cerr << message << std::endl;
	exit(0);
}

constexpr uint16_t get16bit(char buffer[])
{
	return
		static_cast<uint16_t>(static_cast<uint8_t>(buffer[0])) |
		static_cast<uint16_t>(static_cast<uint8_t>(buffer[1])) << 8;
}
constexpr uint32_t get32bit(char buffer[])
{
	return
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[0])) |
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[1])) << 8 |
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[2])) << 16 |
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[3])) << 24;
}
void set16bit(uint16_t data, char buffer[])
{
	buffer[0] = static_cast<char>(data);
	buffer[1] = static_cast<char>(data >> 8);
}
void set32bit(uint32_t data, char buffer[])
{
	buffer[0] = static_cast<char>(data);
	buffer[1] = static_cast<char>(data >> 8);
	buffer[2] = static_cast<char>(data >> 16);
	buffer[3] = static_cast<char>(data >> 24);
}

RGBAdata applyBlurMatrix(const RGBAdata& data, int8_t mat[3][3])
{
	RGBAdata res = data;

	int8_t total = 0;
	for (size_t i = 0; i < 3; ++i) for (size_t j = 0; j < 3; ++j) total += mat[i][j];

	for (size_t row = 1, endRow = data.size() - 1; row < endRow; ++row)
	{
		for (size_t col = 1, endCol = data[row].size() - 1; col < endCol; ++col)
		{
			res[row][col].r =
				(
					static_cast<int16_t>(data[row + 1][col - 1].r) * mat[0][0] +
					static_cast<int16_t>(data[row + 1][col].r) * mat[0][1] +
					static_cast<int16_t>(data[row + 1][col + 1].r) * mat[0][2] +
					static_cast<int16_t>(data[row][col - 1].r) * mat[0][0] +
					static_cast<int16_t>(data[row][col].r) * mat[0][1] +
					static_cast<int16_t>(data[row][col + 1].r) * mat[0][2] +
					static_cast<int16_t>(data[row - 1][col - 1].r) * mat[0][0] +
					static_cast<int16_t>(data[row - 1][col].r) * mat[0][1] +
					static_cast<int16_t>(data[row - 1][col + 1].r) * mat[0][2]
					) / total;
			res[row][col].g =
				(
					static_cast<int16_t>(data[row + 1][col - 1].g) * mat[0][0] +
					static_cast<int16_t>(data[row + 1][col].g) * mat[0][1] +
					static_cast<int16_t>(data[row + 1][col + 1].g) * mat[0][2] +
					static_cast<int16_t>(data[row][col - 1].g) * mat[0][0] +
					static_cast<int16_t>(data[row][col].g) * mat[0][1] +
					static_cast<int16_t>(data[row][col + 1].g) * mat[0][2] +
					static_cast<int16_t>(data[row - 1][col - 1].g) * mat[0][0] +
					static_cast<int16_t>(data[row - 1][col].g) * mat[0][1] +
					static_cast<int16_t>(data[row - 1][col + 1].g) * mat[0][2]
					) / total;
			res[row][col].b =
				(
					static_cast<int16_t>(data[row + 1][col - 1].b) * mat[0][0] +
					static_cast<int16_t>(data[row + 1][col].b) * mat[0][1] +
					static_cast<int16_t>(data[row + 1][col + 1].b) * mat[0][2] +
					static_cast<int16_t>(data[row][col - 1].b) * mat[0][0] +
					static_cast<int16_t>(data[row][col].b) * mat[0][1] +
					static_cast<int16_t>(data[row][col + 1].b) * mat[0][2] +
					static_cast<int16_t>(data[row - 1][col - 1].b) * mat[0][0] +
					static_cast<int16_t>(data[row - 1][col].b) * mat[0][1] +
					static_cast<int16_t>(data[row - 1][col + 1].b) * mat[0][2]
					) / total;
			res[row][col].a =
				(
					static_cast<int16_t>(data[row + 1][col - 1].a) * mat[0][0] +
					static_cast<int16_t>(data[row + 1][col].a) * mat[0][1] +
					static_cast<int16_t>(data[row + 1][col + 1].a) * mat[0][2] +
					static_cast<int16_t>(data[row][col - 1].a) * mat[0][0] +
					static_cast<int16_t>(data[row][col].a) * mat[0][1] +
					static_cast<int16_t>(data[row][col + 1].a) * mat[0][2] +
					static_cast<int16_t>(data[row - 1][col - 1].a) * mat[0][0] +
					static_cast<int16_t>(data[row - 1][col].a) * mat[0][1] +
					static_cast<int16_t>(data[row - 1][col + 1].a) * mat[0][2]
					) / total;
		}
	}
	return res;
}

int main(int argc, char* argv[])
{

	std::ifstream fin("in.bmp", std::ios::binary);

	char buffer[4];

	// BITMAP HEADER

	fin.read(buffer, 2) || finish("cannot read");
	(buffer[0] == 'B' && buffer[1] == 'M') || finish("not a BMP");

	fin.read(buffer, 4) || finish("cannot read");
	const size_t fileSize = get32bit(buffer);
	std::cout << "The size of the file is " << fileSize << " bytes" << std::endl;

	fin.read(buffer, 4) || finish("cannot read");

	fin.read(buffer, 4) || finish("cannot read");
	const auto dataOffset = get32bit(buffer);
	std::cout << "Data is fout at offset " << dataOffset << std::endl;

	//  DIB HEADER


	fin.read(buffer, 4) || finish("cannot read");
	const auto dibHeaderSize = get32bit(buffer);
	std::cout << "DIB header size is " << dibHeaderSize << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	const auto pixelWidth = get32bit(buffer);
	std::cout << "Width in pixels is " << pixelWidth << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto const pixelHeight = get32bit(buffer);
	std::cout << "Height in pixels is " << pixelHeight << std::endl;

	fin.read(buffer, 2) || finish("cannot read");
	auto const colourPlanes = get16bit(buffer); 
	(colourPlanes == 1) || finish("colour planes must be 1");

	
	auto const bitsPerPixel = get16bit(buffer); 
	std::cout << "Bits per pixel is " << bitsPerPixel << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const compressionType = get32bit(buffer);
	(compressionType == 0) || finish("only compression 0 is supported");
	std::cout << "The type of compression is " << compressionType << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const imageSize = get32bit(buffer);
	std::cout << "Image size is " << imageSize << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto const horizontalRes = get32bit(buffer);
	std::cout << "Horizontal resolution is " << horizontalRes << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const verticalRes = get32bit(buffer);
	std::cout << "Vertical resolution is " << verticalRes << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const numOfColours = std::max<uint32_t>(get32bit(buffer), static_cast<uint32_t>(1) << bitsPerPixel);
	std::cout << "Number of colours is " << numOfColours << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const numOfImportantColours = get32bit(buffer);
	std::cout << "Number of important colours is " << numOfImportantColours << std::endl;

	// DIB: COLOUR PROFILE


	fin.read(buffer, 4) || finish("cannot read");
	auto const maskRed = get32bit(buffer);
	std::cout << "Red mask is   " << std::hex << std::setw(8) << std::setfill('0') << maskRed << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const maskGreen = get32bit(buffer);
	std::cout << "Green mask is " << std::hex << std::setw(8) << std::setfill('0') << maskGreen << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const maskBlue = get32bit(buffer);
	std::cout << "Blue mask is  " << std::hex << std::setw(8) << std::setfill('0') << maskBlue << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto const maskAlpha = get32bit(buffer);
	std::cout << "Alpha mask is  " << std::hex << std::setw(8) << std::setfill('0') << maskAlpha << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto const colourSpace = get32bit(buffer);
	std::cout << "Colour space is " << colourSpace << ' ' << (char)buffer[3] << (char)buffer[2] << (char)buffer[1] << (char)buffer[0] << std::endl;
	(colourSpace == 0x73524742) || finish("only sRBG colour space is supported");
	std::cout << std::dec;


	fin.read(buffer, 4) || finish("cannot read");
	auto  endpRedX = DOT30(get32bit(buffer));
	std::cout << "X coordinate of red endpoint is   " << double(endpRedX) << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto  endpRedY = DOT30(get32bit(buffer));
	std::cout << "Y coordinate of red endpoint is   " << (double)endpRedY << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto  endpRedZ = DOT30(get32bit(buffer));
	std::cout << "Z coordinate of red endpoint is   " << (double)endpRedZ << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto  endpGreenX = DOT30(get32bit(buffer));
	std::cout << "X coordinate of green endpoint is " << (double)endpGreenX << std::endl;

	
	fin.read(buffer, 4) || finish("cannot read");
	auto  endpGreenY = DOT30(get32bit(buffer));
	std::cout << "Y coordinate of green endpoint is " << (double)endpGreenY << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto  endpGreenZ = DOT30(get32bit(buffer));
	std::cout << "Z coordinate of green endpoint is " << (double)endpGreenZ << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto  endpBlueX = DOT30(get32bit(buffer));
	std::cout << "X coordinate of blue endpoint is  " << (double)endpBlueX << std::endl;

	fin.read(buffer, 4) || finish("cannot read");
	auto  endpBlueY = DOT30(get32bit(buffer));
	std::cout << "Y coordinate of blue endpoint is  " << (double)endpBlueY << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto  endpBlueZ = DOT30(get32bit(buffer));
	std::cout << "Z coordinate of blue endpoint is  " << (double)endpBlueZ << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const gammaRed = get32bit(buffer);
	std::cout << "Gamma red coordinate scale value  " << gammaRed << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const gammaGreen = get32bit(buffer);
	std::cout << "Gamma green coordinate scale value " << gammaGreen << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const gammaBlue = get32bit(buffer);
	std::cout << "Gamma blue coordinate scale value  " << gammaBlue << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const intent = get32bit(buffer);
	std::cout << "The intent is " << intent << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const profileDataOffset = get32bit(buffer);
	std::cout << "Profile data offset " << profileDataOffset << std::endl;


	fin.read(buffer, 4) || finish("cannot read");
	auto const profileDataSize = get32bit(buffer);
	std::cout << "Profile data size   " << profileDataSize << std::endl;


	fin.read(buffer, 4) || finish("cannot read");

	//  DATA 
	buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;

	RGB factory(maskRed, maskGreen, maskBlue, maskAlpha);
	RGBAdata data(pixelHeight);

	auto currentOffset = dataOffset;
	for (auto& row : data)
	{
		row.resize(pixelWidth);
		{
			auto nextOffset = (currentOffset * 4 + 3) / 4;
			while (nextOffset > currentOffset && currentOffset < fileSize)
			{
				fin.read(buffer, 1) || finish("cannot read");
				++currentOffset;
			}
		}
		for (auto& pixel : row)
		{
			auto bytesToRead = (bitsPerPixel + 7) / 8;
			fin.read(buffer, bytesToRead) || finish("cannot read");
			factory.makeRGBAquad(pixel, get32bit(buffer));
			currentOffset += bytesToRead;
		}
	}
	std::cout << "The offset at the end is " << currentOffset << std::endl;

	fin.close();


	

	int8_t blurMatrix[3][3] =
	{ { 1,  1,  1},
		{ 1,  1,  1},
		{ 1,  1,  1}
	};

	auto blurredData = applyBlurMatrix(data, blurMatrix);

	
	std::ofstream fout("out.bmp", std::ios::binary);

	//  HEADER
	buffer[0] = 'B'; buffer[1] = 'M';
	fout.write(buffer, 2) || finish("cannot write");

	
	set32bit(fileSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(0, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(dataOffset, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	//  DIB HEADER

	
	set32bit(dibHeaderSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(pixelWidth, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(pixelHeight, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set16bit(colourPlanes, buffer);
	fout.write(buffer, 2) || finish("cannot write");

	
	set16bit(bitsPerPixel, buffer);
	fout.write(buffer, 2) || finish("cannot write");

	set32bit(compressionType, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(imageSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(horizontalRes, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(verticalRes, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(numOfColours, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(numOfImportantColours, buffer);
	fout.write(buffer, 4) || finish("cannot write");



	set32bit(maskRed, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(maskGreen, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(maskGreen, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(maskAlpha, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(colourSpace, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(endpRedX.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(endpRedY.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(endpRedZ.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(endpGreenX.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(endpGreenY.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(endpGreenZ.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(endpBlueX.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(endpBlueY.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(endpBlueZ.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	
	set32bit(gammaRed, buffer);
	fout.write(buffer, 4) || finish("cannot write");


	set32bit(gammaGreen, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(gammaBlue, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(intent, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(profileDataOffset, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(profileDataSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	set32bit(0, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	//  DATA 
	buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;

	auto currentWriteOffset = dataOffset;
	for (auto& row : blurredData)
	{
		{
			auto nextOffset = (currentWriteOffset * 4 + 3) / 4;
			while (nextOffset > currentWriteOffset && currentWriteOffset < fileSize)
			{
				fout.write(0, 1) || finish("cannot write");
				++currentWriteOffset;
			}
		}
		for (auto& pixel : row)
		{
			auto bytesToWrite = (bitsPerPixel + 7) / 8;
			uint32_t bytes = factory.saveRGBAquad(pixel);
			set32bit(bytes, buffer);
			fout.write(buffer, bytesToWrite) || finish("cannot write");
			currentWriteOffset += bytesToWrite;
		}
	}
	
	fout.close();

	return 0;
}