//#include "stdafx.h"

/***************************************
 *
 * ImageIO.cpp
 *
 * 画像の入出力
 *
 * Version    : 4
 * Month/Year : 03/2008
 * Author     : Shinya MAEDA, Kim Lab.
 *
 ***************************************/
 /**************************************
  * 更新情報　2008/09/09
  * 九州歯科大学のDICOMを読めるようにしました。
  * ヘッダに#define NON_PRIFIX を記述すること
  * Preamble : なし
  * Prefix   : なし
  * Data Element : Group 0008 Element 0000　(開始位置)
  * Implicit VR による記述
 ****************************************/
 /**************************************
  * 更新情報　2008/09/19
  * ボリュームデータからBitmapファイルを
  * シーケンシャルに出力可能
 ****************************************/
 /**************************************
  * 更新情報　2008/10/21
  * WL,WW,GANMAを用いた濃度諧調変換を追加
 ****************************************/
 /**************************************
  * 更新情報　2008/10/24
  * VRのSQが値長さが未定義長さの場合の
  * 例外処理を追加
 ****************************************/
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <climits>
#include "ImageIO.h"

 // 転送構文
static const std::string ImplicitVRLittleEndianTF("1.2.840.10008.1.2");		// 暗黙的VRリトルエンディアン転送構文
static const std::string ExplicitVRLittleEndianTF("1.2.840.10008.1.2.1");	// 明示的VRリトルエンディアン転送構文
static const std::string ExplicitVRBigEndianTF("1.2.840.10008.1.2.2");		// 明示的VRビッグエンディアン転送構文

// DICOMファイルの読み込み
bool DicomReader::read(std::string fileName, ShortImage2D * dcm)
{
	// DICOMファイルメタ情報は Explicit VR Little Endian
	implicitVR_ = true;
	littleEndian_ = true;
	prefix = true;

	//画像情報の取得
	this->getInfo(fileName, &info);

	if (ifs_.is_open()) {
		ifs_.close();
	}

	ifs_.open(fileName.c_str(), std::ios_base::binary);
	if (!ifs_) {
		std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
		return false;
	}

	// ヘッダの空読み
	if (scan() == false) {
		std::cerr << "Failed to read file \"" << fileName << "\"" << std::endl;
		ifs_.close();
		sizeX_ = sizeY_ = 0;
		return false;
	}

	// 画像のメモリ割り当て
	dcm->resize(sizeX_, sizeY_);

	// 画像データの取得
	for (int y = 0; y < dcm->sizeY; y++) {
		ifs_.read((char *)dcm->data[y], dcm->sizeX * sizeof(short));
	}

	ifs_.close();
	sizeX_ = sizeY_ = 0;

	//画像情報の取得
	dcm->setScale(info.pixelSpacingX, info.pixelSpacingY);

	return true;
}

// タグの読み取り
unsigned int DicomReader::getTag()
{
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;

	ifs_.read((char *)&temp1, sizeof(unsigned short));
	ifs_.read((char *)&temp2, sizeof(unsigned short));

	return (temp1 << 16) | temp2;
}

// 画像情報の取得
bool DicomReader::getInfo(std::string fileName, DicomImageInfo * info)
{
	// DICOMファイルメタ情報は Explicit VR Little Endian
	implicitVR_ = true;
	littleEndian_ = true;
	prefix = true;

	if (ifs_.is_open()) {
		ifs_.close();
	}

	ifs_.open(fileName.c_str(), std::ios_base::binary);
	if (!ifs_) {
		std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
		return false;
	}

	// ヘッダのスキャン
	if (scanDicomMetaInfo() == false) {
		prefix = false;
		ifs_.seekg(0, std::ifstream::beg);
	}

	unsigned int tag = 0;
	int valueLength = 0;
	std::string str;

	// 0x7fe0 0x0010　画像データのタグまでヘッダを読む
	while (ifs_.eof() == false && (tag = getTag()) != 0x7fe00010) {
		valueLength = getValueLength();
		//printf("tag : %0x  length : %d\n", tag, valueLength);
		char ch = 0;
		if (tag == 0x00180050) {	// 0x0018 0x0050 Thickness
			str = getStr(valueLength);
			info->thickness = static_cast<double>(std::atof((str.c_str())));
		}
		else if (tag == 0x00200011) {	// 0x0020 0x0011 Series Number
			str = getStr(valueLength);
			info->seriesNumber = static_cast<int>(std::atof((str.c_str())));
		}
		else if (tag == 0x00200013) {	// 0x0020 0x0013: Instance Number
			str = getStr(valueLength);
			info->instanceNumber = static_cast<int>(std::atof((str.c_str())));
		}
		else if (tag == 0x00280010) {	// 0x0028 0x0010: Rows
			ifs_.read((char *)&info->rows, valueLength);
		}
		else if (tag == 0x00280011) {	// 0x0028 0x0011: Columns
			ifs_.read((char *)&info->columns, valueLength);
		}
		else if (tag == 0x00280030) {	// 0x0028 0x0030: Pixel Spacing
			std::string pixelSpacing;
			for (int i = 0; i < valueLength; i++) {
				char ch;
				ifs_.read(&ch, sizeof(char));
				if (ch != '\0')
					pixelSpacing += ch;
			}
			std::size_t bsPos = pixelSpacing.find("\\", 0);
			str = pixelSpacing.substr(0, bsPos);
			info->pixelSpacingX = static_cast<double>(std::atof(str.c_str()));
			str = pixelSpacing.substr(bsPos + 1);
			info->pixelSpacingY = static_cast<double>(std::atof(str.c_str()));
		}
		else if (tag == 0x00281050) {	// 0x0028 0x1050 Window Center 
			str = getStr(valueLength);
			info->windowCenter = static_cast<int>(std::atof((str.c_str())));
		}
		else if (tag == 0x00281051) {	// 0x0028 0x1051 Window Width
			str = getStr(valueLength);
			info->windowWidth = static_cast<int>(std::atof((str.c_str())));
		}
		else {
			for (int i = 0; i < valueLength; i++) {
				ifs_.read(&ch, sizeof(char));
			}
		}
	}

	ifs_.close();

	return true;
}

// 転送構文の読み取り
// implicitVR_, littleEndian_のフラグを設定する
void DicomReader::setTransferSyntax(std::string transferSyntax)
{
	// VR, Endianの設定
	if (transferSyntax == ImplicitVRLittleEndianTF) {
		// Implicit VR Little Endian
		implicitVR_ = true;
		littleEndian_ = true;
	}
	else if (transferSyntax == ExplicitVRLittleEndianTF) {
		// Explict VR Little Endian
		implicitVR_ = false;
		littleEndian_ = true;
	}
	else if (transferSyntax == ExplicitVRBigEndianTF) {
		// Explict VR Big Endian
		implicitVR_ = false;
		littleEndian_ = false;
	}
	else {
		std::cerr << "This file's transfer syntax is not supported currently." << std::endl;
		implicitVR_ = true;
		littleEndian_ = true;
	}
}

// 値長さの取得
unsigned int DicomReader::getValueLength(int * byte)
{
	unsigned int valueLength = 0;
	unsigned short tmp = 0;
	int sum = 0;

	if (implicitVR_) {
		// Implicit VR Little Endian
		ifs_.read((char *)&valueLength, sizeof(unsigned int));
		sum += sizeof(unsigned int);
	}
	else {
		if (littleEndian_) {
			// Explicit VR Little Endian
			char ch = '0';
			std::string vr("");
			for (int i = 0; i < 2; i++) {
				ifs_.read(&ch, sizeof(char));
				sum += sizeof(char);
				vr += ch;
			}
			//std::cout<< vr : << vr << std::endl;
			if (vr != "OB" && vr != "OW" && vr != "SQ" && vr != "UN") {
				ifs_.read((char *)&valueLength, sizeof(unsigned short));
				sum += sizeof(unsigned short);
			}
			else if (vr == "SQ") {		//  追加
				ifs_.read((char *)&tmp, sizeof(unsigned short));
				sum += sizeof(unsigned short);
				ifs_.read((char *)&valueLength, sizeof(unsigned int));
				sum += sizeof(unsigned int);

				if (valueLength == -1) {	// 値長さが未定義長さ
					// データ要素タグを空読み
					ifs_.read((char *)&valueLength, sizeof(unsigned int));
					sum += sizeof(unsigned int);
					// シーケンス区切り項目のデータ数をvalueLengthとする
					valueLength = sizeof(unsigned int);
				}
			}
			else {
				ifs_.read((char *)&tmp, sizeof(unsigned short));
				sum += sizeof(unsigned short);
				ifs_.read((char *)&valueLength, sizeof(unsigned int));
				sum += sizeof(unsigned int);
			}
		}
		else {
			// Explicit VR Big Endian
			std::cerr << "Explicit VR Big Endian is not supported currently." << std::endl;
			valueLength = 0;
		}
	}

	*byte = sum;

	return valueLength;
}

// 文字列として読み込む
std::string DicomReader::getStr(int valueLength)
{
	std::string str;
	char ch;

	for (int i = 0; i < valueLength; i++) {
		ifs_.read(&ch, sizeof(char));
		if (ch != '\0') {
			str += ch;
		}
	}

	return str;
}

// DICOMファイルメタ情報をスキャンする
bool DicomReader::scanDicomMetaInfo()
{
	std::string str("");
	std::string DICM("DICM");

	// ファイルプリアンブルを空読み
	char ch = 0;
	while (ch != 'D' && !ifs_.eof()) {
		ifs_.read(&ch, sizeof(char));
	}
	str += ch;

	if (ifs_.eof()) {
		std::cerr << "Failed to read DICOM file." << std::endl;
		return false;
	}

	// DICOMプリフィックスを読む
	for (int i = 1; i < 4; i++) {
		ifs_.read(&ch, sizeof(char));
		str += ch;
	}

	if (str != DICM) {
		std::cerr << "This file is not DICOM format." << std::endl;
		return false;		// 追加 !!
	}

	// DICOMファイルメタ情報は Explicit VR Little Endian
	implicitVR_ = false;
	littleEndian_ = true;

	unsigned int tag = 0;
	int valueLength = 0;
	int groupLength = 0;

	// グループ長さの取得
	tag = getTag();
	valueLength = getValueLength();
	if (tag == 0x00020000) { // 0x0002 0x0000 グループ長さ
		ifs_.read((char *)&groupLength, valueLength);
	}

	int len = 0;
	std::string transferSyntax;
	while (len < groupLength) {
		tag = getTag();
		len += sizeof(tag);
		int byte = 0;
		if (tag == 0x00020010) {	// 0x0002 0x0010: TransferSyntaxUID
			valueLength = getValueLength(&byte);
			len += valueLength;
			len += byte;
			// 転送構文の読み取り
			for (int i = 0; i < valueLength; i++) {
				char ch = 0;
				ifs_.read(&ch, sizeof(char));
				if (ch != 0) {
					transferSyntax += ch;
				}
			}
		}
		else {
			valueLength = getValueLength(&byte);
			len += valueLength;
			len += byte;
			for (int i = 0; i < valueLength; i++) {
				char ch;
				ifs_.read(&ch, sizeof(char));
			}
		}
	}

	implicitVR_ = true; // デフォルトに戻す

	setTransferSyntax(transferSyntax);

	return true;
}

// DICOMヘッダ情報の読み取り
bool DicomReader::scan()
{
	//#ifndef NON_PRIFIX
	if (prefix)
		if (scanDicomMetaInfo() == false) return false;	//追加！！！
//#endif
	unsigned int tag = 0;
	int valueLength = 0;

	// 0x7fe0 0x0010　画像データのタグまでヘッダを読む
	while (ifs_.eof() == false && (tag = getTag()) != 0x7fe00010) {
		valueLength = getValueLength();
		char ch = 0;
		if (tag == 0x00280010) {			// 0x0028 0x0010: Rows
			ifs_.read((char *)&sizeY_, valueLength);
		}
		else if (tag == 0x00280011) {	// 0x0028 0x0011: Columns
			ifs_.read((char *)&sizeX_, valueLength);
		}
		else {
			for (int i = 0; i < valueLength; i++) {
				ifs_.read(&ch, sizeof(char));
			}
		}
	}

	valueLength = getValueLength();

	return true;
}

// Bitmap画像の読み込み
bool BitmapReader::read(std::string fileName, RGBImage *img)
{
	std::ifstream ifs(fileName.c_str(), std::ios_base::binary);

	if (!ifs) {
		std::cerr << "File open error! \"" << fileName << "\": BitmapReader::read" << std::endl;
		return false;
	}

	// ファイルヘッダの読み込み
	Bmp::BitmapFileHeader bmfh;
	ifs.read((char *)&bmfh.bfType, sizeof(bmfh.bfType));
	ifs.read((char *)&bmfh.bfSize, sizeof(bmfh.bfSize));
	ifs.read((char *)&bmfh.bfReserved1, sizeof(bmfh.bfReserved1));
	ifs.read((char *)&bmfh.bfReserved2, sizeof(bmfh.bfReserved2));
	ifs.read((char *)&bmfh.bfOffBits, sizeof(bmfh.bfOffBits));

	// 情報ヘッダの読み込み
	Bmp::BitmapInfoHeader bmih;
	ifs.read((char *)&bmih.biSize, sizeof(bmih.biSize));
	ifs.read((char *)&bmih.biWidth, sizeof(bmih.biWidth));
	ifs.read((char *)&bmih.biHeight, sizeof(bmih.biHeight));
	ifs.read((char *)&bmih.biPlanes, sizeof(bmih.biPlanes));
	ifs.read((char *)&bmih.biBitCount, sizeof(bmih.biBitCount));
	ifs.read((char *)&bmih.biCompression, sizeof(bmih.biCompression));
	ifs.read((char *)&bmih.biSizeImage, sizeof(bmih.biSizeImage));
	ifs.read((char *)&bmih.biXPelsPerMeter, sizeof(bmih.biXPelsPerMeter));
	ifs.read((char *)&bmih.biYPelsPerMeter, sizeof(bmih.biYPelsPerMeter));
	ifs.read((char *)&bmih.biClrUsed, sizeof(bmih.biClrUsed));
	ifs.read((char *)&bmih.biClrImportant, sizeof(bmih.biClrImportant));

	// 画像配列のサイズ設定
	if (bmih.biWidth != img->sizeX || bmih.biHeight != img->sizeY) {
		img->resize(bmih.biWidth, bmih.biHeight);
	}

	char pad;
	int padNum = img->sizeX % 4;	// X方向における詰め物の数

	// 画素データの読み込み
	for (int y = img->sizeY - 1; y >= 0; y--) {
		ifs.read((char *)img->data[y], 3 * img->sizeX);
		for (int p = 0; p < padNum; p++) {
			ifs.read((char *)&pad, 1);
		}
	}

	ifs.close();

	return true;
}

// 画像の書き出し（RGBImage -> Bitmap）
bool BitmapWriter::write(std::string fileName, RGBImage * img)
{
	std::ofstream ofs(fileName.c_str(), std::ios_base::binary);

	if (!ofs) {
		std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
		return false;
	}

	// ヘッダ情報の設定
	Bmp::BitmapInfoHeader bmih;
	setBitmapInfoHeader(img->sizeX, img->sizeY, bmih);
	Bmp::BitmapFileHeader bmfh;
	setBitmapFileHeader(bmih.biSizeImage, bmfh);

	// ヘッダ情報の書き出し
	ofs.write((char *)&bmfh.bfType, sizeof(bmfh.bfType));
	ofs.write((char *)&bmfh.bfSize, sizeof(bmfh.bfSize));
	ofs.write((char *)&bmfh.bfReserved1, sizeof(bmfh.bfReserved1));
	ofs.write((char *)&bmfh.bfReserved2, sizeof(bmfh.bfReserved2));
	ofs.write((char *)&bmfh.bfOffBits, sizeof(bmfh.bfOffBits));
	ofs.write((char *)&bmih.biSize, sizeof(bmih.biSize));
	ofs.write((char *)&bmih.biWidth, sizeof(bmih.biWidth));
	ofs.write((char *)&bmih.biHeight, sizeof(bmih.biHeight));
	ofs.write((char *)&bmih.biPlanes, sizeof(bmih.biPlanes));
	ofs.write((char *)&bmih.biBitCount, sizeof(bmih.biBitCount));
	ofs.write((char *)&bmih.biCompression, sizeof(bmih.biCompression));
	ofs.write((char *)&bmih.biSizeImage, sizeof(bmih.biSizeImage));
	ofs.write((char *)&bmih.biXPelsPerMeter, sizeof(bmih.biXPelsPerMeter));
	ofs.write((char *)&bmih.biYPelsPerMeter, sizeof(bmih.biYPelsPerMeter));
	ofs.write((char *)&bmih.biClrUsed, sizeof(bmih.biClrUsed));
	ofs.write((char *)&bmih.biClrImportant, sizeof(bmih.biClrImportant));

	int sizeX = img->sizeX;
	int sizeY = img->sizeY;
	int padNum = sizeX % 4;
	unsigned char pad = '\0';
	int bufSize = (3 * sizeX + padNum) * sizeY;
	unsigned char * buf = new unsigned char[bufSize];
	unsigned int idx = 0;

	// バッファにRGB値を格納
	for (int y = sizeY - 1; y >= 0; y--) {
		for (int x = 0; x < sizeX; x++) {

			buf[idx++] = img->data[y][x].b;
			buf[idx++] = img->data[y][x].g;
			buf[idx++] = img->data[y][x].r;
		}
		std::memset(&buf[idx], pad, padNum);
		idx += padNum;
	}

	// 画像データの書き出し
	ofs.write((char *)buf, bufSize);

	delete[] buf;
	ofs.close();

	return true;
}

// Bitmap画像の書き出し
bool BitmapWriter::write(std::string dirName, RGBImage3D * img, int plane)
{
	RGBImage out;
	std::string fileName;
	std::ostringstream oss;
	//フォルダ作成
	if (!_mkdir(dirName.c_str()))
		std::cerr << "Make folder \"" << dirName << "\"" << std::endl;
	if (plane == Axial) {
		out.newImage(img->sizeX, img->sizeY);
		for (int k = 0; k < img->sizeZ; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeY; j++)
				for (int i = 0; i < img->sizeX; i++) {
					out.data[j][i].r = img->data[k][j][i].r;
					out.data[j][i].g = img->data[k][j][i].g;
					out.data[j][i].b = img->data[k][j][i].b;
				}
			if (!write(fileName, &out)) return 0;
			oss.str("");
		}
	}
	if (plane == Sagittal) {
		out.newImage(img->sizeY, img->sizeZ);
		for (int k = 0; k < img->sizeX; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeZ; j++)
				for (int i = 0; i < img->sizeY; i++)
				{
					out.data[j][i].r = img->data[j][i][k].r;
					out.data[j][i].g = img->data[j][i][k].g;
					out.data[j][i].b = img->data[j][i][k].b;
				}
			if (!write(fileName, &out)) return 0;
			oss.str("");
		}
	}
	if (plane == Coronal) {
		out.newImage(img->sizeX, img->sizeZ);
		for (int k = 0; k < img->sizeY; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeZ; j++)
				for (int i = 0; i < img->sizeX; i++)
				{
					out.data[j][i].r = img->data[j][k][i].r;
					out.data[j][i].g = img->data[j][k][i].g;
					out.data[j][i].b = img->data[j][k][i].b;
				}
			if (!write(fileName, &out)) return 0;
			oss.str("");
		}
	}

	return 1;
}

// Bitmap画像の書き出し
bool BitmapWriter::write(std::string fileName, ShortImage2D * img, short wl, short ww, double ganma = 1.0)
{
	std::ofstream ofs(fileName.c_str(), std::ios_base::binary);

	if (!ofs) {
		std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
		return false;
	}

	// ヘッダ情報の設定
	Bmp::BitmapInfoHeader bmih;
	setBitmapInfoHeader(img->sizeX, img->sizeY, bmih);
	Bmp::BitmapFileHeader bmfh;
	setBitmapFileHeader(bmih.biSizeImage, bmfh);

	// ヘッダ情報の書き出し
	ofs.write((char *)&bmfh.bfType, sizeof(bmfh.bfType));
	ofs.write((char *)&bmfh.bfSize, sizeof(bmfh.bfSize));
	ofs.write((char *)&bmfh.bfReserved1, sizeof(bmfh.bfReserved1));
	ofs.write((char *)&bmfh.bfReserved2, sizeof(bmfh.bfReserved2));
	ofs.write((char *)&bmfh.bfOffBits, sizeof(bmfh.bfOffBits));
	ofs.write((char *)&bmih.biSize, sizeof(bmih.biSize));
	ofs.write((char *)&bmih.biWidth, sizeof(bmih.biWidth));
	ofs.write((char *)&bmih.biHeight, sizeof(bmih.biHeight));
	ofs.write((char *)&bmih.biPlanes, sizeof(bmih.biPlanes));
	ofs.write((char *)&bmih.biBitCount, sizeof(bmih.biBitCount));
	ofs.write((char *)&bmih.biCompression, sizeof(bmih.biCompression));
	ofs.write((char *)&bmih.biSizeImage, sizeof(bmih.biSizeImage));
	ofs.write((char *)&bmih.biXPelsPerMeter, sizeof(bmih.biXPelsPerMeter));
	ofs.write((char *)&bmih.biYPelsPerMeter, sizeof(bmih.biYPelsPerMeter));
	ofs.write((char *)&bmih.biClrUsed, sizeof(bmih.biClrUsed));
	ofs.write((char *)&bmih.biClrImportant, sizeof(bmih.biClrImportant));

	int sizeX = img->sizeX;
	int sizeY = img->sizeY;
	int padNum = sizeX % 4;
	unsigned char pad = '\0';
	int bufSize = (3 * sizeX + padNum) * sizeY;
	unsigned char * buf = new unsigned char[bufSize];
	unsigned int idx = 0;

	// WW,WLに関するLUT
	DataArray <unsigned char> lut(ww);

	for (int i = 0; i < ww; i++) {
		int value = int((double)i / (double)ww * Bmp::RGBMaxIntensity);

		if (value > Bmp::RGBMaxIntensity) value = Bmp::RGBMaxIntensity;
		if (value < 0)					 value = 0;

		lut.data[i] = (unsigned char)value;
	}

	// ガンマ補正
	for (int i = 0; i < ww; i++) {

		int value = int(Bmp::RGBMaxIntensity * pow(((double)lut.data[i] / Bmp::RGBMaxIntensity), 1.0 / ganma));

		if (value > Bmp::RGBMaxIntensity) value = Bmp::RGBMaxIntensity;
		if (value < 0)					 value = 0;

		lut.data[i] = (unsigned char)value;
	}

	// バッファにRGB値を格納
	for (int y = sizeY - 1; y >= 0; y--) {
		for (int x = 0; x < sizeX; x++) {
			short value = img->data[y][x];

			unsigned char gray;

			if (value <= wl - ww / 2) gray = lut.data[0];
			else if (value >= wl + ww / 2) gray = lut.data[ww - 1];
			else						 gray = lut.data[value - (wl - ww / 2)];

			std::memset(&buf[idx], gray, 3);
			idx += 3;
		}
		std::memset(&buf[idx], pad, padNum);
		idx += padNum;
	}

	// 画像データの書き出し
	ofs.write((char *)buf, bufSize);

	delete[] buf;
	ofs.close();

	return true;
}

// write
//  Bitmap画像の書き出し（ShortImage2D -> Bitmap）
// 
// 引数
//  fileName	: 出力ファイル名
//  img			: 出力する画像
//  min			: 画像化する画素値の範囲の最小値
//  max			: 画像化する画素値の範囲の最大値
//
// 戻り値
//  出力成功でtrue，失敗でfalseを返す
bool BitmapWriter::write(std::string fileName, ShortImage2D * img, double min, double max)
{
	std::ofstream ofs(fileName.c_str(), std::ios_base::binary);

	if (!ofs) {
		std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
		return false;
	}

	// ヘッダ情報の設定
	Bmp::BitmapInfoHeader bmih;
	setBitmapInfoHeader(img->sizeX, img->sizeY, bmih);
	Bmp::BitmapFileHeader bmfh;
	setBitmapFileHeader(bmih.biSizeImage, bmfh);

	// ヘッダ情報の書き出し
	ofs.write((char *)&bmfh.bfType, sizeof(bmfh.bfType));
	ofs.write((char *)&bmfh.bfSize, sizeof(bmfh.bfSize));
	ofs.write((char *)&bmfh.bfReserved1, sizeof(bmfh.bfReserved1));
	ofs.write((char *)&bmfh.bfReserved2, sizeof(bmfh.bfReserved2));
	ofs.write((char *)&bmfh.bfOffBits, sizeof(bmfh.bfOffBits));
	ofs.write((char *)&bmih.biSize, sizeof(bmih.biSize));
	ofs.write((char *)&bmih.biWidth, sizeof(bmih.biWidth));
	ofs.write((char *)&bmih.biHeight, sizeof(bmih.biHeight));
	ofs.write((char *)&bmih.biPlanes, sizeof(bmih.biPlanes));
	ofs.write((char *)&bmih.biBitCount, sizeof(bmih.biBitCount));
	ofs.write((char *)&bmih.biCompression, sizeof(bmih.biCompression));
	ofs.write((char *)&bmih.biSizeImage, sizeof(bmih.biSizeImage));
	ofs.write((char *)&bmih.biXPelsPerMeter, sizeof(bmih.biXPelsPerMeter));
	ofs.write((char *)&bmih.biYPelsPerMeter, sizeof(bmih.biYPelsPerMeter));
	ofs.write((char *)&bmih.biClrUsed, sizeof(bmih.biClrUsed));
	ofs.write((char *)&bmih.biClrImportant, sizeof(bmih.biClrImportant));

	int sizeX = img->sizeX;
	int sizeY = img->sizeY;
	int padNum = sizeX % 4;
	unsigned char pad = '\0';
	int bufSize = (3 * sizeX + padNum) * sizeY;
	unsigned char * buf = new unsigned char[bufSize];
	unsigned int idx = 0;

	// 濃度変換のパラメータ
	double gradient = Bmp::RGBMaxIntensity / (max - min);
	double intercept = -gradient * min;

	// バッファにRGB値を格納
	for (int y = sizeY - 1; y >= 0; y--) {
		for (int x = 0; x < sizeX; x++) {
			double value = static_cast<double>(img->data[y][x]);
			if (value < min) {
				value = min;
			}
			if (value > max) {
				value = max;
			}
			unsigned char gray =
				static_cast<unsigned char>(gradient * value + intercept + 0.5);
			std::memset(&buf[idx], gray, 3);
			idx += 3;
		}
		std::memset(&buf[idx], pad, padNum);
		idx += padNum;
	}

	// 画像データの書き出し
	ofs.write((char *)buf, bufSize);

	delete[] buf;	buf = NULL;
	ofs.close();

	return true;
}

// write
//  Bitmap画像の書き出し（ShortImage3D -> Bitmap）
// 
// 引数
//	dirName		: 出力ディレクトリ名
//  img			: 出力する画像
//	min			: 画像化する画素値の範囲の最小値
//	max			: 画像化する画素値の範囲の最大値
//
// 戻り値
//  出力成功でtrue，失敗でfalseを返す
// Bitmap画像の書き出し
bool BitmapWriter::write(std::string dirName, ShortImage3D * img, double min, double max, int plane)
{
	ShortImage2D out;
	std::string fileName;
	std::ostringstream oss;

	//フォルダ作成
	if (!_mkdir(dirName.c_str()))
		std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

	if (plane == Axial) {
		out.newImage(img->sizeX, img->sizeY);
		for (int k = 0; k < img->sizeZ; k++) {
			oss << k;
			/*if(k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if(k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			*/ fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeY; j++)
				for (int i = 0; i < img->sizeX; i++) out.data[j][i] = img->data[k][j][i];
			if (!writeT(fileName, &out, min, max)) return 0;
			oss.str("");
		}
	}
	if (plane == Sagittal) {
		out.newImage(img->sizeY, img->sizeZ);
		for (int k = 0; k < img->sizeX; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeZ; j++)
				for (int i = 0; i < img->sizeY; i++) out.data[j][i] = img->data[j][i][k];
			if (!writeT(fileName, &out, min, max)) return 0;
			oss.str("");
		}
	}
	if (plane == Coronal) {
		out.newImage(img->sizeX, img->sizeZ);
		for (int k = 0; k < img->sizeY; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeZ; j++)
				for (int i = 0; i < img->sizeX; i++) out.data[j][i] = img->data[j][k][i];
			if (!writeT(fileName, &out, min, max)) return 0;
			oss.str("");
		}
	}

	return 1;
}

// Bitmap画像の書き出し
bool BitmapWriter::write(std::string dirName, ShortImage3D * img, short wl, short ww, double ganma = 1.0, int plane = 0)
{
	ShortImage2D out;
	std::string fileName;
	std::ostringstream oss;
	//フォルダ作成
	if (!_mkdir(dirName.c_str()))
		std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

	if (plane == Axial) {
		out.newImage(img->sizeX, img->sizeY);
		for (int k = 0; k < img->sizeZ; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeY; j++)
				for (int i = 0; i < img->sizeX; i++) out.data[j][i] = img->data[k][j][i];
			if (!write(fileName, &out, wl, ww, ganma)) return 0;
			oss.str("");
		}
	}
	if (plane == Sagittal) {
		out.newImage(img->sizeY, img->sizeZ);
		for (int k = 0; k < img->sizeX; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeZ; j++)
				for (int i = 0; i < img->sizeY; i++) out.data[j][i] = img->data[j][i][k];
			if (!write(fileName, &out, wl, ww, ganma)) return 0;
			oss.str("");
		}
	}
	if (plane == Coronal) {
		out.newImage(img->sizeX, img->sizeZ);
		for (int k = 0; k < img->sizeY; k++) {
			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
			else			 fileName = dirName + "\\" + oss.str() + ".bmp";
			for (int j = 0; j < img->sizeZ; j++)
				for (int i = 0; i < img->sizeX; i++) out.data[j][i] = img->data[j][k][i];
			if (!write(fileName, &out, wl, ww, ganma)) return 0;
			oss.str("");
		}
	}
	return 1;
}

// 24ビットフルカラーBitmapのBitmapInfoHeaderの設定
void BitmapWriter::setBitmapInfoHeader(long width, long height, Bmp::BitmapInfoHeader & bmih)
{
	bmih.biSize = 0x28;
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biPlanes = 0x01;
	bmih.biBitCount = 24;
	bmih.biCompression = 0x00;
	bmih.biSizeImage = height * (width * 3 + width % 4);
	bmih.biXPelsPerMeter = 0x00;
	bmih.biYPelsPerMeter = 0x00;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0x00;
}

// 24ビットフルカラーBitmapのBitmapFileHeaderの設定
void BitmapWriter::setBitmapFileHeader(unsigned long biSizeImage, Bmp::BitmapFileHeader & bmfh)
{
	bmfh.bfType = 'B' + ('M' << 8);
	bmfh.bfOffBits = 54;
	bmfh.bfSize = 54 + biSizeImage;
	bmfh.bfReserved1 = 0x00;
	bmfh.bfReserved2 = 0x00;
}

