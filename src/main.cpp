#include <string>
#include "imageIO.h"

int main(int argc, char *argv[])
{
	RGBImage image;												// 画像を保持するための変数
	BitmapReader bmpReader;										// Bitmapファイルを読み込むためのクラス
	std::string inputFilename = "../data/color/Lenna.bmp";	    // 読み込むBitmapファイルへのパス
	bmpReader.read(inputFilename, &image);						// BitmapReaderのReadを使うことで画像を変数に対して読み込む

	RGBType pixelValue;											// ピクセル値を保持するための変数
	//pixelValue = image.data[20][10];							// X : 10, Y : 20の位置のピクセル値(RGBType型)を取り出す

	image.data[10][20].setElement(0, 255, 0);

	for (int y = 0; y < image.sizeY / 2; ++y)
	{
		for (int x = 0; x < image.sizeX / 2; ++x)
		{
			image.data[y][x].setElement(0, 0, 255);				// X : x, Y : yの位置のピクセル値にr : 255, g : 100, b : 0をセット
		}
	}

	BitmapWriter bmpWriter;										// Bitmapファイルを書き込むためのクラス
	std::string outputFilename = "output.bmp";					// 出力画像のパスと名前
	bmpWriter.write(outputFilename, &image);					// ビットマップへ書き出し

	return 0;
}