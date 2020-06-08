#include <string>
#include "imageIO.h"

int main(void)
{
	BitmapReader bmpReader;										// Bitmap�t�@�C����ǂݍ��ނ��߂̃N���X
	RGBImage image;												// �摜��ێ����邽�߂̕ϐ�
	//color_image image;
	std::string inputFilename = "../data/color/Lenna.bmp";	    // �ǂݍ���Bitmap�t�@�C���ւ̃p�X
	bmpReader.read(inputFilename, &image);						// BitmapReader��Read���g�����Ƃŉ摜��ϐ��ɑ΂��ēǂݍ���

	RGBType pixelValue;											// �s�N�Z���l��ێ����邽�߂̕ϐ�
	//pixelValue = image.data[20][10];							// X : 10, Y : 20�̈ʒu�̃s�N�Z���l(RGBType�^)�����o��


	image.data[10][20].setElement(0, 255, 0);					// X : 20, Y : 10�̈ʒu�̃s�N�Z���l��r : 255, g : 100, b : 0���Z�b�g

	BitmapWriter bmpWriter;										// Bitmap�t�@�C�����������ނ��߂̃N���X
	std::string outputFilename = "original_ok.bmp";	            // �o�͉摜�̃p�X�Ɩ��O
	bmpWriter.write(outputFilename, &image);					// �r�b�g�}�b�v�֏����o��

	return 0;
}