/***************************************
 *
 * "ImageIO.h".h
 *
 * �摜�̓��o��
 *
 * Version    : 5
 * Month/Year : 05/2010
 * Author     : Shinya MAEDA, Kim Lab.
 ***************************************/
 /**************************************
  * �X�V���@2008/09/09
  * ��B���ȑ�w��DICOM��ǂ߂�悤�ɂ��܂����B
  * �w�b�_��#define NON_PRIFIX ���L�q���邱��
  * Preamble : �Ȃ�
  * Prefix   : �Ȃ�
  * Data Element : Group 0008 Element 0000�@(�J�n�ʒu)
  * Implicit VR �ɂ��L�q
 ****************************************/
 /**************************************
  * �X�V���@2008/09/19
  * �{�����[���f�[�^����Bitmap�t�@�C����
  * �V�[�P���V�����ɏo�͉\
 ****************************************/
 /**************************************
  * �X�V���@2008/10/21
  * WL,WW,GANMA��p�����Z�x�~���ϊ���ǉ�
 ****************************************/
 /**************************************
  * �X�V���@2008/10/24
  * VR��SQ���l����������`�����̏ꍇ��
  * ��O������ǉ�
 ****************************************/
 /**************************************
  * �X�V���@2011/03/02
  * prefix�̗L���Ɋւ�炸�ADICOM�摜�ǂݍ��݉\�ɕύX
  * �x�N�g���v�f�̃N���X�̒ǉ�
  * Short�^�ȊO��BMP�o�͉\��writeT�֐��̒ǉ�
 ****************************************/
#ifndef IMAGE_IO_H_
#define IMAGE_IO_H_

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cmath>
#include <float.h>
#include "direct.h"	//�t�H���_�쐬�p

 //���񏈗����s���ꍇ�̓R�����g�A�E�g���͂���
#define OPENMP

#ifdef OPENMP
#include<omp.h>
#endif

//��B���ȑ�w��DICOM�̂Ƃ���KYUSHU_DENT_UNIV��define����

//---------------------------------------------------------------------------------------------------
// �f�[�^�z��֌W
//  �e���v���[�g��p���Ă���
//---------------------------------------------------------------------------------------------------

template < class T > class DataArray;
template < class T > class DataArray2D;
template < class T > class DataArray3D;

//typedef unsigned char UChar;
//typedef unsigned int  UInt;
//typedef unsigned short UShort;
namespace ELEMENT {
	//
	// 2�����x�N�g���^ 
	//
	template < class T > class Vector2D
	{
	public:
		//�R���X�g���N�^
		Vector2D(T x = 0, T y = 0) : X(x), Y(y) {}
		//�R�s�[�R���X�g���N�^
		Vector2D(Vector2D & obj) : X(obj.X), Y(obj.Y) {}
		//�f�X�g���N�^
		~Vector2D() {}
		// �l�̐ݒ�
		void set(T data) { X = Y = data; }
		void set(T x, T y) {
			this->X = x;
			this->Y = y;
		}
		// �v�f
		T X;
		T Y;
		//�R�s�[������Z�q�͖�����
		Vector2D & operator=(Vector2D & obj) {
			this->X = obj.X;
			this->Y = obj.Y;
			return *this;
		}
	private:
	};
	//
	// 3�����x�N�g���^
	//
	template < class T > class Vector3D {
	public:
		// �R���X�g���N�^
		Vector3D(T x = 0, T y = 0, T z = 0) : X(x), Y(y), Z(z) {}
		// �R�s�[�R���X�g���N�^
		Vector3D(const Vector3D &obj) : X(obj.X), Y(obj.Y), Z(obj.Z) {}
		// �f�X�g���N�^
		~Vector3D() {}

		// �l�̐ݒ�
		void set(T data) { X = Y = Z = data; }
		void set(T x, T y, T z) {
			this->X = x;
			this->Y = y;
			this->Z = z;
		}
		// �t�@�C���o��(������)
		// �t�@�C������(������)
		// �v�f
		T X;
		T Y;
		T Z;

		// �R�s�[������Z�q
		Vector3D& operator=(const Vector3D &obj)
		{
			this->X = obj.X;
			this->Y = obj.Y;
			this->Z = obj.Z;
			return *this;
		}

	private:
	};
}

// �悭�g���摜�z���typedef���Ă���
typedef ELEMENT::Vector2D<bool> Bool2D;
typedef ELEMENT::Vector2D<char> Char2D;
typedef ELEMENT::Vector2D<unsigned char> UChar2D;
typedef ELEMENT::Vector2D<short> Short2D;
typedef ELEMENT::Vector2D<int> Int2D;
typedef ELEMENT::Vector2D<float> Float2D;
typedef ELEMENT::Vector2D<double> Double2D;
typedef ELEMENT::Vector3D<bool> Bool3D;
typedef ELEMENT::Vector3D<char> Char3D;
typedef ELEMENT::Vector3D<unsigned char> UChar3D;
typedef ELEMENT::Vector3D<short> Short3D;
typedef ELEMENT::Vector3D<int> Int3D;
typedef ELEMENT::Vector3D<float> Float3D;
typedef ELEMENT::Vector3D<double> Double3D;
//
// 1�����z��N���X
//
template < class T > class DataArray {
public:
	typedef T ValueType;	// �i�[����f�[�^�^

	int size;	// �v�f��
	double scale; // ���@
	T *data;	// �f�[�^�z��

	// �R���X�g���N�^
	DataArray() : size(0), scale(1.0f), data(0) {}
	DataArray(int size) : size(size), scale(1.0f), data(0) {
		// ���������蓖��
		data = new(std::nothrow) T[size];
		if (!data) {
			std::cerr << "Memory allocation error! DataArray::DataArray(int size)" << std::endl;
			if (getchar() == NULL) {
				return;
			}
			//std::exit(1);
			return;
		}
		// 0�ŏ�����
		std::memset(data, 0, sizeof(T) * size);
	}

	// �f�X�g���N�^
	virtual ~DataArray() { clear(); }

	// resize
	//   �z��̃��T�C�Y
	// 
	//  ����
	//   size : �z��̃T�C�Y
	void resize(int size) {
		// ���łɃ����������蓖�Ă���Ȃ�J������
		if (data != 0) { clear(); }
		// �������̊��蓖��
		data = new(std::nothrow) T[size];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray::resize(int size)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		// �T�C�Y�̐ݒ�
		this->size = size;
		this->scale = 1.0f;
		// �z���0�ŏ�����
		std::memset(data, 0, sizeof(T) * size);
	}

	// clear
	//  �f�[�^�̃��������J��
	void clear() {
		// �����������蓖�Ă��Ă��Ȃ��Ȃ�Ή������Ȃ�
		if (data == 0) { return; }
		// �������̊J��
		delete[] data;
		// �����o�ϐ��̏�����
		data = 0;
		size = 0;
	}

	// �R�s�[�R���X�g���N�^
	DataArray(const DataArray & obj) : size(obj.size), scale(obj.scale), data(0) {
		// �������̊��蓖��
		data = new(std::nothrow) T[size];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray::DataArray(const DataArray< T > & obj)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		// �f�[�^�̃R�s�[
		std::memcpy(data, obj.data, sizeof(T) * size);
	}

	//�f�[�^�̏�����
	void init() {
		// �z���0�ŏ�����
		std::memset(data, 0, sizeof(T) * size);
	}

	// �R�s�[������Z�q
	DataArray & operator=(const DataArray & rhs) {
		// �������g�ւ̑���̖h�~
		if (this == &rhs) {
			return *this;
		}
		// ���I�u�W�F�N�g�̌�n��
		clear();
		// �������̊m��
		resize(rhs.size);
		// �f�[�^�̃R�s�[
		std::memcpy(data, rhs.data, sizeof(T) * size);
		// ���@�̃Z�b�g
		setScale(rhs.scale);
		// ������Ԃ�
		return *this;
	}

	//���@�̃Z�b�g
	void setScale(double scale) {
		this->scale = scale;
	}

	//�R���\�[���\��
	inline void Print() {
		for (int j = 0; j < size; j++)
			std::cout << data[j] << "\n";
		std::cout << std::endl;
	}
private:
};

typedef DataArray<bool> BoolArray;
typedef DataArray<char> CharArray;
typedef DataArray<unsigned char> UCharArray;
typedef DataArray<short> ShortArray;
typedef DataArray<unsigned short> UShortArray;
typedef DataArray<int> IntArray;
typedef DataArray<unsigned int> UIntArray;
typedef DataArray<float> FloatArray;
typedef DataArray<double> DoubleArray;

// 
// �Q�����f�[�^�i�[�p�N���X
//
template < class T > class DataArray2D {
public:
	typedef T ValueType;	// �摜�̃f�[�^�^

	int sizeX;	// X�����T�C�Y
	int sizeY;	// Y�����T�C�Y
	Double2D scale;	//�s�N�Z�����@
	T **data;	// �摜�f�[�^�̃|�C���^

	// �R���X�g���N�^
	DataArray2D() : sizeX(0), sizeY(0), scale(0.0f, 0.0f), data(0) {}
	DataArray2D(int sizeX, int sizeY) : sizeX(sizeX), sizeY(sizeY), scale(1.0f, 1.0f), data(0) {
		// �������̊��蓖��
		data = new(std::nothrow) T*[this->sizeY];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray2D::DataArray2D(int sizeX, int sizeY)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		for (int y = 0; y < this->sizeY; y++) {
			data[y] = new(std::nothrow) T[this->sizeX];
			if (data[y] == 0) {
				std::cerr << "Memory allocation error! DataArray2D::DataArray2D(int sizeX, int sizeY)" << std::endl;
				getchar();
				//std::exit(1);
				return;
			}
			// �z���0�ŏ�����
			std::memset(data[y], 0, sizeof(T) * this->sizeX);
		}
	}

	// �f�X�g���N�^
	virtual ~DataArray2D() { clear(); }

	// newImage
	//  �摜�f�[�^�̃��������m�ہD
	//
	// ����
	//  sizeX : X�����T�C�Y
	//  sizeY : Y�����T�C�Y
	//
	// �߂�l
	//  �����@0, ���s -1��Ԃ��D
	int newImage(int sizeX, int sizeY) {
		// ���łɃ����������蓖�Ă��Ă���Ȃ�ΊJ������
		if (data != 0) { clear(); }
		// �������̊��蓖��
		data = new(std::nothrow) T*[sizeY];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray2D::newImage(int sizeX, int sizeY)" << std::endl;
			if (getchar() != 0) {
				;
			}
			else {
				printf("oh my god\n");
			}
			//std::exit(1);
			return 1;
		}
		for (int y = 0; y < sizeY; y++) {
			data[y] = new(std::nothrow) T[sizeX];
			if (data[y] == 0) {
				std::cerr << "Memory allocation error! DataArray2D::newImage(int sizeX, int sizeY)" << std::endl;
				if (getchar() != 0) {
					;
				}
				else {
					printf("oh my god\n");
				}
				//std::exit(1);
				return 1;
			}
			// �z���0�ŏ�����
			std::memset(data[y], 0, sizeof(T) * sizeX);
		}
		// �T�C�Y�̐ݒ�
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		// �X�P�[���̐ݒ�
		this->scale.set(1.0f, 1.0f);
		return 0;
	}

	// deleteImage
	//  �摜�f�[�^�̃��������J���D
	void deleteImage() { clear(); }

	// resize
	//   �z��̃��T�C�Y
	// 
	//  ����
	//   sizeX : X�����T�C�Y
	//   sizeY : Y�����T�C�Y
	void resize(int sizeX, int sizeY) {
		// ���łɃ����������蓖�Ă��Ă���Ȃ�ΊJ������
		if (data != 0) { clear(); }
		// �������̊��蓖��
		data = new(std::nothrow) T*[sizeY];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray2D::resize(int sizeX, int sizeY)" << std::endl;
			if (getchar() == NULL) {
				return;
			}
			//std::exit(1);
			return;
		}
		for (int y = 0; y < sizeY; y++) {
			data[y] = new(std::nothrow) T[sizeX];
			if (data[y] == 0) {
				std::cerr << "Memory allocation error! DataArray2D::resize(int sizeX, int sizeY)" << std::endl;
				if (getchar() == NULL) {
					return;
				}
				//std::exit(1);
				return;
			}
			// �z���0�ŏ�����
			std::memset(data[y], 0, sizeof(T) * sizeX);
		}
		// �T�C�Y�̐ݒ�
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		// �X�P�[���̐ݒ�
		this->scale.set(1.0f, 1.0f);
	}

	// clear
	//  �f�[�^�̃��������J��
	void clear() {
		// �����������蓖�Ă��Ă��Ȃ��Ȃ�΁C�������Ȃ�
		if (data == 0) { return; }
		// �������̊J��
		for (int y = 0; y < sizeY; y++) {
			delete[] data[y];
		}
		delete[] data;
		// �����o�ϐ���������
		data = 0;
		sizeX = sizeY = 0;
	}


	// �R�s�[�R���X�g���N�^
	DataArray2D(const DataArray2D & obj) : sizeX(obj.sizeX), sizeY(obj.sizeY), scale(obj.scale.X, obj.scale.Y), data(0) {
		// �������̊��蓖��
		data = new T*[sizeY];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray2D< T >::DataArray2D(const DataArray2D< T > & obj)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		for (int y = 0; y < sizeY; y++) {
			data[y] = new(std::nothrow) T[sizeX];
			if (data[y] == 0) {
				std::cerr << "Memory allocation error! DataArray2D< T >::ShortImage2D(const DataArray2D< T > & obj)" << std::endl;
				getchar();
				//std::exit(1);
				return;
			}
		}
		// �f�[�^�̃R�s�[
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				data[y][x] = obj.data[y][x];
			}
		}
	}

	// �R�s�[������Z�q
	DataArray2D & operator=(const DataArray2D & rhs) {
		// �������g�ւ̑���̖h�~
		if (this == &rhs) { return *this; }
		// ���I�u�W�F�N�g�̌�n��
		clear();
		// �T�C�Y�̃R�s�[
		sizeX = rhs.sizeX;
		sizeY = rhs.sizeY;
		// ���@�̃R�s�[
		scale.X = rhs.scale.X;
		scale.Y = rhs.scale.Y;
		// �������̊��蓖��
		resize(rhs.sizeX, rhs.sizeY);
		// �f�[�^�̃R�s�[
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				data[y][x] = rhs.data[y][x];
			}
		}
		// �������A��
		return *this;
	}

	//�f�[�^�̏�����
	void init() {
		for (int y = 0; y < sizeY; y++) {
			// �z���0�ŏ�����
			std::memset(data[y], 0, sizeof(T) * sizeX);
		}
	}

	//�s�N�Z�����@�̃Z�b�g
	void setScale(const Double2D &scale) {
		this->scale = scale;
	}
	void setScale(double scaleX, double scaleY) {
		this->scale.X = scaleX;
		this->scale.Y = scaleY;
	}

	//�R���\�[���\��
	inline void Print() {
		for (int i = 0; i < sizeY; i++) {
			for (int j = 0; j < sizeX; j++)
				std::cout << data[i][j] << "\t";
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
private:
};

typedef DataArray2D<bool> BoolImage2D;
typedef DataArray2D<char> CharImage2D;
typedef DataArray2D<unsigned char> UCharImage2D;
typedef DataArray2D<short> ShortImage2D;	// 2����Dicom�摜�p
typedef DataArray2D<int> IntImage2D;
typedef DataArray2D<unsigned int> UIntImage2D;
typedef DataArray2D<float> FloatImage2D;
typedef DataArray2D<double> DoubleImage2D;


//
// 3�����f�[�^�i�[�p�N���X
//
template< class T > class DataArray3D {
public:
	typedef T ValueType;	// �摜�̃f�[�^�^

	int sizeX;	// X�����T�C�Y
	int sizeY;	// Y�����T�C�Y
	int sizeZ;	// Z�����T�C�Y
	Double3D scale;	// �{�N�Z�����@
	T ***data;	// �摜�f�[�^�̃|�C���^

	// �R���X�g���N�^
	DataArray3D() : sizeX(0), sizeY(0), sizeZ(0), scale(1.0f, 1.0f, 1.0f), data(0) {}
	DataArray3D(int sizeX, int sizeY, int sizeZ) : sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ), scale(1.0, 1.0, 1.0), data(0) {
		// �������̊��蓖��
		data = new(std::nothrow) T**[this->sizeZ];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray3D::DataArray3D(int sizeX, int sizeY, int sizeZ)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		for (int z = 0; z < this->sizeZ; z++) {
			data[z] = new(std::nothrow) T*[this->sizeY];
			if (data[z] == 0) {
				std::cerr << "Memory allocation error! DataArray3D::DataArray3D(int sizeX, int sizeY, int sizeZ)" << std::endl;
				getchar();
				//std::exit(1);
				return;
			}
			for (int y = 0; y < this->sizeY; y++) {
				data[z][y] = new(std::nothrow) T[this->sizeX];
				if (data[z][y] == 0) {
					std::cerr << "Memory allocation error! DataArray3D::DataArray3D(int sizeX, int sizeY, int sizeZ)" << std::endl;
					getchar();
					//std::exit(1);
					return;
				}
				// �z���0�ŏ�����
				std::memset(data[z][y], 0, sizeof(T) * this->sizeX);
			}
		}
	}

	// �f�X�g���N�^
	virtual ~DataArray3D() { clear(); }

	// newImage
	//  �摜�f�[�^�̃��������m�ہD
	//
	// ����
	//  sizeX : X�����T�C�Y
	//  sizeY : Y�����T�C�Y
	//  sizeZ : Z�����T�C�Y
	//
	// �߂�l
	//  �����@0, ���s -1��Ԃ��D
	int newImage(int sizeX, int sizeY, int sizeZ) {
		// ���łɃ����������蓖�Ă��Ă���Ȃ�ΊJ������
		if (data != 0) { deleteImage(); }
		// �������̊��蓖��
		data = new(std::nothrow) T**[sizeZ];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray3D::newImage(int sizeX, int sizeY, int sizeZ)" << std::endl;
			getchar();
			//std::exit(1);
			return 1;
		}
		for (int z = 0; z < sizeZ; z++) {
			data[z] = new(std::nothrow) T*[sizeY];
			if (data[z] == 0) {
				std::cerr << "Memory allocation error! DataArray3D::newImage(int sizeX, int sizeY, int sizeZ)" << std::endl;
				getchar();
				//std::exit(1);
				return 1;
			}
			for (int y = 0; y < sizeY; y++) {
				data[z][y] = new(std::nothrow) T[sizeX];
				if (data[z][y] == 0) {
					return -1;
				}
				// �z���0�ŏ�����
				std::memset(data[z][y], 0, sizeof(T) * sizeX);
			}
		}
		// �T�C�Y�̐ݒ�
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->sizeZ = sizeZ;
		this->scale.set(1.0f, 1.0f, 1.0f);
		return 0;
	}

	// deleteImage
	//  �摜�f�[�^�̃��������J���D
	void deleteImage() { clear(); }

	// resize
	//   �z��̃��T�C�Y
	// 
	//  ����
	//   sizeX : X�����T�C�Y
	//   sizeY : Y�����T�C�Y
	//   sizeZ : Z�����T�C�Y
	void resize(int sizeX, int sizeY, int sizeZ) {
		// ���łɃ����������蓖�Ă��Ă���Ȃ�ΊJ������
		if (data != 0) { clear(); }
		// �������̊��蓖��
		data = new(std::nothrow) T**[sizeZ];
		if (data == 0) {
			std::cerr << "Memory allocation error! ShortImage3D::resize(int sizeX, int sizeY, int sizeZ)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		for (int z = 0; z < sizeZ; z++) {
			data[z] = new(std::nothrow) T*[sizeY];
			if (data[z] == 0) {
				std::cerr << "Memory allocation error! ShortImage3D::resize(int sizeX, int sizeY, int sizeZ)" << std::endl;
				getchar();
				//std::exit(1);
				return;
			}
			for (int y = 0; y < sizeY; y++) {
				data[z][y] = new(std::nothrow) T[sizeX];
				if (data[z][y] == 0) {
					std::cerr << "Memory allocation error! ShortImage3D::resize(int sizeX, int sizeY, int sizeZ)" << std::endl;
					getchar();
					//std::exit(1);
					return;
				}
				// �z���0�ŏ�����
				std::memset(data[z][y], 0, sizeof(T) * sizeX);

			}
		}
		// �T�C�Y�̐ݒ�
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->sizeZ = sizeZ;
		this->scale.set(1.0f, 1.0f, 1.0f);
	}

	// clear
	//  �f�[�^�̃��������J��
	void clear() {
		// �����������蓖�Ă��Ă��Ȃ��Ȃ�΁C�������Ȃ�
		if (data == 0) { return; }
		// �������̊J��
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				delete[] data[z][y];
			}
			delete[] data[z];
		}
		delete[] data;
		// �����o�ϐ��̏�����
		data = 0;
		this->scale.set(1.0f, 1.0f, 1.0f);
		sizeX = sizeY = sizeZ = 0;
	}

	// �R�s�[�R���X�g���N�^
	DataArray3D(const DataArray3D & obj) : sizeX(obj.sizeX), sizeY(obj.sizeY), sizeZ(obj.sizeZ), scale(obj.scale), data(0) {
		// �������̊��蓖��
		data = new T**[sizeZ];
		if (data == 0) {
			std::cerr << "Memory allocation error! ShortImage3D::DataArray3D(const DataArray3D & obj)" << std::endl;
			getchar();
			//std::exit(1);			
			return;
		}
		for (int z = 0; z < sizeZ; z++) {
			data[z] = new(std::nothrow) T*[sizeY];
			if (data[z] == 0) {
				std::cerr << "Memory allocation error! ShortImage3D::DataArray3D(const DataArray3D & obj)" << std::endl;
				getchar();
				//std::exit(1);
				return;
			}
			for (int y = 0; y < sizeY; y++) {
				data[z][y] = new(std::nothrow) T[sizeX];
				if (data[z][y] == 0) {
					std::cerr << "Memory allocation error! ShortImage3D::DataArray3D(const DataArray3D & obj)" << std::endl;
					getchar();
					//std::exit(1);
					return;
				}
			}
		}
		// �f�[�^�̃R�s�[
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = obj.data[z][y][x];
				}
			}
		}
	}

	// �R�s�[������Z�q
	DataArray3D & operator=(const DataArray3D & rhs) {
		// �������g�ւ̑���̖h�~
		if (this == &rhs) { return *this; }
		// ���I�u�W�F�N�g�̌�n��
		clear();
		// �T�C�Y�̃R�s�[
		sizeX = rhs.sizeX;
		sizeY = rhs.sizeY;
		sizeZ = rhs.sizeZ;
		// �{�N�Z�����@�̃R�s�[
		scale = rhs.scale;
		// �������̊��蓖��
		resize(rhs.sizeX, rhs.sizeY, rhs.sizeZ);
		// �f�[�^�̃R�s�[
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = rhs.data[z][y][x];
				}
			}
		}
		// �������A��
		return *this;
	}

	//�f�[�^�̏�����
	void init() {
		for (int z = 0; z < this->sizeZ; z++) {
			for (int y = 0; y < this->sizeY; y++) {
				// �z���0�ŏ�����
				std::memset(data[z][y], 0, sizeof(T) * this->sizeX);
			}
		}
	}

	//�{�N�Z���X�P�[���̃Z�b�g
	void setScale(const Double3D &scale) {
		this->scale = scale;
	}

	//���@�ݒ�
	void setScale(double scaleX, double scaleY, double scaleZ) {
		this->scale.set(scaleX, scaleY, scaleZ);
	}
private:
};

typedef DataArray3D<bool> BoolImage3D;
typedef DataArray3D<char> CharImage3D;
typedef DataArray3D<unsigned char> UCharImage3D;
typedef DataArray3D<short> ShortImage3D;	// 3����Dicom�摜�p
typedef DataArray3D<int> IntImage3D;
typedef DataArray3D<unsigned int> UIntImage3D;
typedef DataArray3D<float> FloatImage3D;
typedef DataArray3D<double> DoubleImage3D;


////////////////////////////////////////////////////////////////////////////
//�����ʒu���킹�p

template< class T > class DataArray_verK_3D {
public:
	typedef T ValueType;	// �摜�̃f�[�^�^

	int sizeX;	// X�����T�C�Y
	int sizeY;	// Y�����T�C�Y
	int sizeZ;	// Z�����T�C�Y
	int centerX;
	int centerY;
	int centerZ;
	int VOISize;
	T ***data;	// �摜�f�[�^�̃|�C���^

	// �R���X�g���N�^
	DataArray_verK_3D() : sizeX(0), sizeY(0), sizeZ(0), data(0) {}
	DataArray_verK_3D(int sizeX, int sizeY, int sizeZ) : sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ), data(0) {
		// �������̊��蓖��
		data = new T**[this->sizeZ];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray3D::DataArray3D(int sizeX, int sizeY, int sizeZ)" << std::endl;
			std::exit(1);
		}
		for (int z = 0; z < this->sizeZ; z++) {
			data[z] = new T*[this->sizeY];
			if (data[z] == 0) {
				std::cerr << "Memory allocation error! DataArray3D::DataArray3D(int sizeX, int sizeY, int sizeZ)" << std::endl;
				std::exit(1);
			}
			for (int y = 0; y < this->sizeY; y++) {
				data[z][y] = new T[this->sizeX];
				if (data[z][y] == 0) {
					std::cerr << "Memory allocation error! DataArray3D::DataArray3D(int sizeX, int sizeY, int sizeZ)" << std::endl;
					std::exit(1);
				}
				// �z���0�ŏ�����
				std::memset(data[z][y], 0, sizeof(T) * this->sizeX);
			}
		}
	}

	// �f�X�g���N�^
	virtual ~DataArray_verK_3D() { clear(); }

	// newImage
	//  �摜�f�[�^�̃��������m�ہD
	//
	// ����
	//  sizeX : X�����T�C�Y
	//  sizeY : Y�����T�C�Y
	//  sizeZ : Z�����T�C�Y
	//
	// �߂�l
	//  �����@0, ���s -1��Ԃ��D
	int newImage(int sizeX, int sizeY, int sizeZ) {
		newImage(sizeX, sizeY, sizeZ, 0, 0, 0, 0);
		return 0;
	}

	// newImage
	//  �摜�f�[�^�̃��������m�ہD
	//
	// ����
	//  sizeX : X�����T�C�Y
	//  sizeY : Y�����T�C�Y
	//  sizeZ : Z�����T�C�Y
	//
	// �߂�l
	//  �����@0, ���s -1��Ԃ��D
	int newImage(int sizeX, int sizeY, int sizeZ, int centerX, int centerY, int centerZ, int VOISize) {
		// ���łɃ����������蓖�Ă��Ă���Ȃ�ΊJ������
		if (data != 0) { deleteImage(); }
		// �������̊��蓖��
		data = new T**[sizeZ];
		if (data == 0) {
			return -1;
		}
		for (int z = 0; z < sizeZ; z++) {
			data[z] = new T*[sizeY];
			if (data[z] == 0) {
				return -1;
			}
			for (int y = 0; y < sizeY; y++) {
				data[z][y] = new T[sizeX];
				if (data[z][y] == 0) {
					return -1;
				}
				// �z���0�ŏ�����
				std::memset(data[z][y], 0, sizeof(T) * sizeX);
			}
		}
		// �T�C�Y�̐ݒ�
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->sizeZ = sizeZ;
		this->centerX = centerX;
		this->centerY = centerY;
		this->centerZ = centerZ;

		return 0;
	}

	void VOIset(int centerX, int centerY, int centerZ, int VOISize) {
		this->centerX = centerX;
		this->centerY = centerY;
		this->centerZ = centerZ;
		this->VOISize = VOISize;
	}
	// deleteImage
	//  �摜�f�[�^�̃��������J���D
	void deleteImage() { clear(); }

	// resize
	//   �z��̃��T�C�Y
	// 
	//  ����
	//   sizeX : X�����T�C�Y
	//   sizeY : Y�����T�C�Y
	//   sizeZ : Z�����T�C�Y
	void resize(int sizeX, int sizeY, int sizeZ) {
		resize(sizeX, sizeY, sizeZ, 0, 0, 0, 0);
	}

	// resize
	//   �z��̃��T�C�Y
	// 
	//  ����
	//   sizeX : X�����T�C�Y
	//   sizeY : Y�����T�C�Y
	//   sizeZ : Z�����T�C�Y
	void resize(int sizeX, int sizeY, int sizeZ, int centerX, int centerY, int centerZ, int VOISize) {
		// ���łɃ����������蓖�Ă��Ă���Ȃ�ΊJ������
		if (data != 0) { clear(); }
		// �������̊��蓖��
		data = new T**[sizeZ];
		if (data == 0) {
			std::cerr << "Memory allocation error! ShortImage3D::resize(int sizeX, int sizeY, int sizeZ)" << std::endl;
			std::exit(1);
		}
		for (int z = 0; z < sizeZ; z++) {
			data[z] = new T*[sizeY];
			if (data[z] == 0) {
				std::cerr << "Memory allocation error! ShortImage3D::resize(int sizeX, int sizeY, int sizeZ)" << std::endl;
				std::exit(1);
			}
			for (int y = 0; y < sizeY; y++) {
				data[z][y] = new T[sizeX];
				if (data[z][y] == 0) {
					std::cerr << "Memory allocation error! ShortImage3D::resize(int sizeX, int sizeY, int sizeZ)" << std::endl;
					std::exit(1);
				}
				// �z���0�ŏ�����
				std::memset(data[z][y], 0, sizeof(T) * sizeX);

			}
		}
		// �T�C�Y�̐ݒ�
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->sizeZ = sizeZ;
		this->centerX = centerX;
		this->centerY = centerY;
		this->centerZ = centerZ;
		this->VOISize = VOISize;
	}

	// clear
	//  �f�[�^�̃��������J��
	void clear() {
		// �����������蓖�Ă��Ă��Ȃ��Ȃ�΁C�������Ȃ�
		if (data == 0) { return; }
		// �������̊J��
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				delete[] data[z][y];
			}
			delete[] data[z];
		}
		delete[] data;
		// �����o�ϐ��̏�����
		data = 0;
		sizeX = sizeY = sizeZ = 0;
	}

	// �R�s�[�R���X�g���N�^
	DataArray_verK_3D(const DataArray_verK_3D & obj) : sizeX(obj.sizeX), sizeY(obj.sizeY), sizeZ(obj.sizeZ), data(0) {
		// �������̊��蓖��
		data = new T**[sizeZ];
		if (data == 0) {
			std::cerr << "Memory allocation error! ShortImage3D::DataArray3D(const DataArray3D & obj)" << std::endl;
			std::exit(1);
		}
		for (int z = 0; z < sizeZ; z++) {
			data[z] = new T*[sizeY];
			if (data[z] == 0) {
				std::cerr << "Memory allocation error! ShortImage3D::DataArray3D(const DataArray3D & obj)" << std::endl;
				std::exit(1);
			}
			for (int y = 0; y < sizeY; y++) {
				data[z][y] = new T[sizeX];
				if (data[z][y] == 0) {
					std::cerr << "Memory allocation error! ShortImage3D::DataArray3D(const DataArray3D & obj)" << std::endl;
					std::exit(1);
				}
			}
		}
		// �f�[�^�̃R�s�[
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = obj.data[z][y][x];
				}
			}
		}
	}

	// �R�s�[������Z�q
	DataArray_verK_3D & operator=(const DataArray_verK_3D & rhs) {
		// �������g�ւ̑���̖h�~
		if (this == &rhs) { return *this; }
		// ���I�u�W�F�N�g�̌�n��
		clear();
		// �T�C�Y�̃R�s�[
		sizeX = rhs.sizeX;
		sizeY = rhs.sizeY;
		// �������̊��蓖��
		resize(rhs.sizeX, rhs.sizeY, rhs.sizeZ, rhs.centerX, rhs.centerY, rhs.centerZ, rhs.VOISize);
		// �f�[�^�̃R�s�[
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = rhs.data[z][y][x];
				}
			}
		}
		// �������A��
		return *this;
	}
private:
};


// �悭�g���摜�z���typedef���Ă���
typedef DataArray_verK_3D< short > ShortImage3DK; // �A�t�B���p�ɍ������`��

//�����ʒu���킹�p
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------
// Dicom�摜�ǂݍ��ݏ����֌W
//  ShorImage2D�݂̂ɑΉ�
//---------------------------------------------------------------------------------------------------

//
// DICOM�摜���
//
class DicomImageInfo {
public:
	DicomImageInfo() :
		thickness(0), seriesNumber(0), instanceNumber(0),
		rows(0), columns(0), pixelSpacingX(0), pixelSpacingY(0),
		windowCenter(0), windowWidth(0)
	{}
	void clear()
	{
		this->thickness = 0;
		this->seriesNumber = 0;
		this->instanceNumber = 0;
		this->rows = 0;
		this->columns = 0;
		this->pixelSpacingX = 0;
		this->pixelSpacingY = 0;
		this->windowCenter = 0;
		this->windowWidth = 0;
	}
	double thickness;		// 0x0018 0x0050 �X���C�X��
	int seriesNumber;		// 0x0020 0x0011 �V���[�Y�ԍ�
	int instanceNumber;		// 0x0020 0x0013 �摜�ԍ�
	unsigned short rows;	// 0x0028 0x0010 �摜��Y�T�C�Y
	unsigned short columns;	// 0x0028 0x0011 �摜��X�T�C�Y
	double pixelSpacingX;	// 0x0028 0x0030 ��f��X�T�C�Y
	double pixelSpacingY;	// 0x0028 0x0030 ��f��Y�T�C�Y
	int windowCenter;		// 0x0028 0x1050 �E�B���h�E���S 
	int windowWidth;		// 0x0028 0x1051 �E�B���h�E��
};

//
// DICOM�̓ǂݍ���
//
class DicomReader {
public:
	// �摜���
	DicomImageInfo info;

	DicomReader() :
		ifs_(), prefix(NULL), implicitVR_(true), littleEndian_(true), sizeX_(0), sizeY_(0) {}

	// Dicom�摜�̓ǂݎ��
	bool read(std::string fileName, ShortImage2D * dcm);

	// Dicom�摜���̎擾
	bool getInfo(std::string fileName, DicomImageInfo * info);

private:
	std::ifstream ifs_;	// ���̓t�@�C���X�g���[��
	bool prefix;		//�v���t�B�N�X�����邩�ǂ���
	bool implicitVR_;	// �ÖٓIVR���ǂ����̃t���O
	bool littleEndian_;	// ���g���G���f�B�A�����ǂ����t���O
	int sizeX_, sizeY_; // �摜�T�C�Y

	// �^�O�̓ǂݎ��
	unsigned int getTag();

	// �]���\���̓ǂݎ��
	// implicitVR_, littleEndian_�̃t���O��ݒ肷��
	void setTransferSyntax(std::string transferSyntax);

	// �l�����̎擾
	unsigned int getValueLength() {
		int dummy = 0;
		return getValueLength(&dummy);
	}
	unsigned int getValueLength(int * byte);

	// ������Ƃ��ēǂݍ���
	std::string getStr(int valueLength);

	// DICOM�t�@�C�����^�����X�L��������
	bool scanDicomMetaInfo();

	// DICOM�f�[�^�̓ǂݎ��
	bool scan();

	// �R�s�[�R���X�g���N�^�C�R�s�[������Z�q�͖�����
	DicomReader(const DicomReader & obj);
	DicomReader & operator=(const DicomReader & rhs);
};


//---------------------------------------------------------------------------------------------------
// Bitmap�摜���o�͊֌W
//  24�r�b�g�t���J���[�ɂ̂ݑΉ��D
//  RGBConverter��DataArray2D��RGBImage�^�ɕϊ����Ă���o�͂��邱�ƁD
//  ShortImage2D�͕ϊ������Ƀ��m�N���摜�ŏo�͂��邱�Ƃ��\�D
//  �F�Â��������ꍇ��RGBConverter��RGBImage�ɕϊ����邱�ƁD
//---------------------------------------------------------------------------------------------------

//
// Bmp���O���
//  Bitmap�֌W�̏����ŗp������̂��܂Ƃ߂Ă���
//
namespace Bmp {
	// RGB�摜�̍ő�P�x�l
	const int RGBMaxIntensity = 255;

	// BitmapInfoHeader�\����
	struct BitmapInfoHeader {
		unsigned long biSize;			// ���w�b�_�T�C�Y�ibyte�j
		long biWidth;					// �摜�̕��ipixel�j
		long biHeight;					// �摜�̍����ipixel�j
		unsigned short biPlanes;		// �v���[�����i���1�j
		unsigned short biBitCount;		// �F�r�b�g��
		unsigned long biCompression;	// ���k�`��
		unsigned long biSizeImage;		// �摜�f�[�^�T�C�Y�ibyte�j
		long biXPelsPerMeter;			// �����𑜓x
		long biYPelsPerMeter;			// �����𑜓x
		unsigned long biClrUsed;		// �i�[�p���b�g��[�g�p�F��]
		unsigned long biClrImportant;	// �d�v�F��
	};

	// BitmapFileHeader�\����
	struct BitmapFileHeader {
		unsigned short bfType;		// �t�@�C���^�C�v�D'BM'
		unsigned long bfSize;		// �t�@�C���T�C�Y�ibyte�j
		unsigned short bfReserved1;	// �\��̈�1�D���0�D
		unsigned short bfReserved2;	// �\��̈�2�D���0�D
		unsigned long bfOffBits;	// �t�@�C���擪����摜�f�[�^�܂ł̃I�t�Z�b�g�ibyte�j
	};
};

//
// RGB�^
//
class RGBType {
public:
	// �R���X�g���N�^
	RGBType(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0) : r(r), g(g), b(b) {}
	// �l�̐ݒ�
	void setElement(unsigned char color) { r = g = b = color; }
	void setElement(unsigned char r, unsigned char g, unsigned char b) {
		this->r = r;
		this->g = g;
		this->b = b;
	}
	// RGB�l
	unsigned char b;
	unsigned char g;
	unsigned char r;

private:
};

//
// 2����RGB�摜�z��
//
typedef DataArray2D< RGBType > RGBImage;
typedef DataArray3D< RGBType > RGBImage3D;

//
// DataArray2D����RGBImage�ւ̕ϊ�
//
class RGBConverter {
public:
	RGBConverter() {}

	// DataArray2D����RGBImage�ւ̕ϊ�
	//  DataArray2D���̃f�[�^�l�̕ϊ��͈͂��w�肵�ĕϊ�����D
	// 
	// ����
	// �@min, max : �ϊ�����DataArray2D�̒l�͈̔͂̍ŏ��l�E�ő�l
	template< class T > void convert(DataArray2D< T > * from, RGBImage * to, T min, T max) {
		// �X�P�[�����O�̃p�����[�^�����߂�
		double slop = Bmp::RGBMaxIntensity / static_cast<double>(max - min);
		double intercept = -slop * min;
		// �摜�̃��T�C�Y
		if ((from->sizeX != to->sizeX) || (from->sizeY != to->sizeX)) {
			to->resize(from->sizeX, from->sizeY);
		}
		// �ϊ�
		for (int y = 0; y < from->sizeY; y++) {
			for (int x = 0; x < from->sizeX; x++) {
				double temp = slop * from->data[y][x] + intercept;
				unsigned char color = static_cast<unsigned char>(temp + ((temp >= 0.0) ? 0.5 : -0.5)); // �l�̌ܓ�
				if (color > Bmp::RGBMaxIntensity) {
					color = Bmp::RGBMaxIntensity;
				}
				else if (color < 0) {
					color = 0;
				}
				to->data[y][x].setElement(color);
			}
		}
	}

	// DataArray2D����RGBImage�ւ̕ϊ�
	//  from���̍ŏ��l�E�ő�l��ǂݎ���āC���͈̔͂�RGB�ɕϊ�����D
	template< class T > void convert(DataArray2D< T > * from, RGBImage * to) {
		T max = from->data[0][0], min = from->data[0][0];
		// �ő�E�ŏ��l��������
		for (int y = 0; y < from->sizeY; y++) {
			for (int x = 0; x < from->sizeX; x++) {
				T value = from->data[y][x];
				if (value < min) {
					min = value;
				}
				if (value > max) {
					max = value;
				}
			}
		}
		// �ϊ�
		convert(from, to, min, max);
	}

	// DataArray3D����RGBImage�ւ̕ϊ�
	template< class T > void convert(DataArray3D< T > * from, RGBImage * to, T max, T min) {

		DataArray2D<T> tmp(from->sizeX, from->sizeY);
		RGBImage tmp2(from->sizeX, from->sizeY);
		to->resize(from->sizeX, from->sizeY, from->sizeZ);
		for (int k = 0; k < from->sizeZ; k++) {
			for (int j = 0; j < from->sizeY; j++)
				for (int i = 0; i < from->sizeX; i++) tmp.data[j][i] = from->data[k][j][i];

			// �ϊ�
			convert(&tmp, &tmp2, min, max);
			for (int j = 0; j < from->sizeY; j++)
				for (int i = 0; i < from->sizeX; i++) to->data[k][j][i] = tmp2.data[j][i];
		}

	}

	// DataArray3D����RGBImage�ւ̕ϊ�
	//  from���̍ŏ��l�E�ő�l��ǂݎ���āC���͈̔͂�RGB�ɕϊ�����D
	template< class T > void convert(DataArray3D< T > * from, RGBImage * to) {
		T max = from->data[0][0][0], min = from->data[0][0][0];
		// �ő�E�ŏ��l��������		
		for (int z = 0; z < from->sizeZ; z++) {
			for (int y = 0; y < from->sizeY; y++) {
				for (int x = 0; x < from->sizeX; x++) {
					T value = from->data[z][y][x];
					if (value < min) {
						min = value;
					}
					if (value > max) {
						max = value;
					}
				}
			}
		}
		// �ϊ�
		convert(from, to, min, max);
	}



private:

	// �R�s�[�R���X�g���N�^�C�R�s�[������Z�q�͖�����
	RGBConverter(const RGBConverter & obj);
	RGBConverter & operator=(const RGBConverter & rhs);
};

//
// Bitmap�摜�̓ǂݍ���
//
class BitmapReader {
public:
	BitmapReader() {}

	// read
	//  Bitmap�摜�̓ǂݍ���
	//
	// ����
	//  fileName	: �摜�t�@�C����
	//  img			: �摜�z��
	//
	// �Ԃ�l
	//  �ǂݍ��ݐ�����true�C�ǂݍ��ݎ��s��false��Ԃ�
	bool read(std::string fileName, RGBImage *img);

private:
	// �R�s�[�R���X�g���N�^�C�R�s�[������Z�q�͖�����
	BitmapReader(const BitmapReader & obj);
	BitmapReader & operator=(const BitmapReader & rhs);
};


static const enum SELECT_PLANE
{
	Axial = 0,
	Sagittal,
	Coronal
}noname;


//
// Bitmap�摜�̏����o��
//
class BitmapWriter {
public:
	BitmapWriter() {}

	// write
	//  Bitmap�摜�̏����o���iRGBImage -> Bitmap�j
	// 
	// ����
	//  fileName	: �o�̓t�@�C����
	//  img			: �o�͂���摜
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	bool write(std::string fileName, RGBImage * img);
	bool write(std::string dirName, RGBImage3D * img, int plane = 0);

	// write
//  Bitmap�摜�̏����o���iShortImage2D -> Bitmap�j
// 
// ����
//	fileName	: �o�̓t�@�C����
//  img			: �o�͂���摜
//	wl			: window level
//	ww			: window width
//  ganma       : ganma�l
//
// �߂�l
//  �o�͐�����true�C���s��false��Ԃ�
	bool write(std::string fileName, ShortImage2D * img, short wl, short ww, double ganma);

	// write
	//  Bitmap�摜�̏����o���iShortImage3D -> Bitmap�j
	// 
	// ����
	//	dirName		: �o�̓f�B���N�g����
	//  img			: �o�͂���摜
	//	wl			: window level
	//	ww			: window width
	//  ganma       : ganma�l
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	bool write(std::string dirName, ShortImage3D * img, short wl, short ww, double ganma, int plane);

	// write
	//  Bitmap�摜�̏����o���iShortImage2D -> Bitmap�j
	// 
	// ����
	//  fileName	: �o�̓t�@�C����
	//  img			: �o�͂���摜
	//  min			: �摜�������f�l�͈̔͂̍ŏ��l
	//  max			: �摜�������f�l�͈̔͂̍ő�l
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	bool write(std::string fileName, ShortImage2D * img, double min, double max);
	bool write(std::string fileName, ShortImage2D * img)
	{
		short min = SHRT_MAX, max = SHRT_MIN;

		int sizeX = img->sizeX, sizeY = img->sizeY;
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				short value = img->data[y][x];
				if (value < min) {
					min = value;
				}
				if (value > max) {
					max = value;
				}
			}
		}
		return write(fileName, img, min, max);
	}



	// write
	//  Bitmap�摜�̏����o���iShortImage3D -> Bitmap�j
	// 
	// ����
	//	dirName		: �o�̓f�B���N�g����
	//  img			: �o�͂���摜
	//	min			: �摜�������f�l�͈̔͂̍ŏ��l
	//	max			: �摜�������f�l�͈̔͂̍ő�l
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	// Bitmap�摜�̏����o��
	bool write(std::string dirName, ShortImage3D * img, double min, double max, int plane = 0);
	bool write(std::string dirName, ShortImage3D * img, int plane = 0)
	{
		ShortImage2D out;
		std::string fileName;
		std::ostringstream oss;

		//�t�H���_�쐬
		if (!_mkdir(dirName.c_str()))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

		if (plane == Axial) {
			out.newImage(img->sizeX, img->sizeY);
			for (int k = 0; k < img->sizeZ; k++) {
				oss << k;
				if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
				else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
				else			 fileName = dirName + "\\" + oss.str() + ".bmp";
				double min = DBL_MAX;
				double max = -DBL_MAX;
				for (int j = 0; j < img->sizeY; j++)
					for (int i = 0; i < img->sizeX; i++)
					{
						out.data[j][i] = img->data[k][j][i];
						if (min > out.data[j][i]) min = out.data[j][i];
						if (max < out.data[j][i]) max = out.data[j][i];
					}
				if (!write(fileName, &out, min, max)) return 0;
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
				double min = DBL_MAX;
				double max = -DBL_MAX;
				for (int j = 0; j < img->sizeZ; j++)
					for (int i = 0; i < img->sizeY; i++)
					{
						out.data[j][i] = img->data[j][i][k];
						if (min > out.data[j][i]) min = out.data[j][i];
						if (max < out.data[j][i]) max = out.data[j][i];
					}
				if (!write(fileName, &out, min, max)) return 0;
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
				double min = DBL_MAX;
				double max = -DBL_MAX;
				for (int j = 0; j < img->sizeZ; j++)
					for (int i = 0; i < img->sizeX; i++)
					{
						out.data[j][i] = img->data[j][k][i];
						if (min > out.data[j][i]) min = out.data[j][i];
						if (max < out.data[j][i]) max = out.data[j][i];
					}
				if (!write(fileName, &out, min, max)) return 0;
				oss.str("");
			}
		}
		return 1;
	}


	// write
	//  Bitmap�摜�̏����o���iShortImage2D -> Bitmap�j
	// 
	// ����
	//  fileName	: �o�̓t�@�C����
	//  img			: �o�͂���摜
	//  min			: �摜�������f�l�͈̔͂̍ŏ��l
	//  max			: �摜�������f�l�͈̔͂̍ő�l
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	template <class T> bool writeT(std::string fileName, DataArray2D<T> * img, double min, double max)
	{
		std::ofstream ofs(fileName.c_str(), std::ios_base::binary);

		if (!ofs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			return false;
		}

		// �w�b�_���̐ݒ�
		Bmp::BitmapInfoHeader bmih;
		setBitmapInfoHeader(img->sizeX, img->sizeY, bmih);
		Bmp::BitmapFileHeader bmfh;
		setBitmapFileHeader(bmih.biSizeImage, bmfh);

		// �w�b�_���̏����o��
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

		// �Z�x�ϊ��̃p�����[�^
		double gradient = Bmp::RGBMaxIntensity / (max - min);
		double intercept = -gradient * min;

		// �o�b�t�@��RGB�l���i�[
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

		// �摜�f�[�^�̏����o��
		ofs.write((char *)buf, bufSize);

		delete[] buf;	buf = NULL;
		ofs.close();

		return true;
	}

	// write
	//  Bitmap�摜�̏����o���iShortImage2D -> Bitmap�j
	// 
	// ����
	//  fileName	: �o�̓t�@�C����
	//  img			: �o�͂���摜
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	template <class T> bool writeT(std::string fileName, DataArray2D<T> * img)
	{
		double min = DBL_MAX, max = -DBL_MAX;

		int sizeX = img->sizeX, sizeY = img->sizeY;
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				double value = static_cast<double>(img->data[y][x]);
				if (value < min) {
					min = value;
				}
				if (value > max) {
					max = value;
				}
			}
		}
		return writeT(fileName, img, min, max);
	}

	// write
	//  Bitmap�摜�̏����o���iShortImage3D -> Bitmap�j
	// 
	// ����
	//	dirName		: �o�̓f�B���N�g����
	//  img			: �o�͂���摜
	//	min			: �摜�������f�l�͈̔͂̍ŏ��l
	//	max			: �摜�������f�l�͈̔͂̍ő�l
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	// Bitmap�摜�̏����o��
	template <class T> bool writeT(std::string dirName, DataArray3D<T> * img, double min, double max, int plane = 0)
	{
		DataArray2D<T> out;
		std::string fileName;
		std::ostringstream oss;

		//�t�H���_�쐬
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


	// write
	//  Bitmap�摜�̏����o���iShortImage3D -> Bitmap�j
	// 
	// ����
	//	dirName		: �o�̓f�B���N�g����
	//  img			: �o�͂���摜
	//
	// �߂�l
	//  �o�͐�����true�C���s��false��Ԃ�
	template <class T> bool writeT(std::string dirName, DataArray3D<T> * img, int plane = 0)
	{
		DataArray2D<T> out;
		std::string fileName;
		std::ostringstream oss;

		//�t�H���_�쐬
		if (!_mkdir(dirName.c_str()))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

		if (plane == Axial) {
			out.newImage(img->sizeX, img->sizeY);
			for (int k = 0; k < img->sizeZ; k++) {
				oss << k;
				if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".bmp";
				else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".bmp";
				else			 fileName = dirName + "\\" + oss.str() + ".bmp";
				double min = DBL_MAX;
				double max = -DBL_MAX;
				for (int j = 0; j < img->sizeY; j++)
					for (int i = 0; i < img->sizeX; i++)
					{
						out.data[j][i] = img->data[k][j][i];
						if (min > out.data[j][i]) min = out.data[j][i];
						if (max < out.data[j][i]) max = out.data[j][i];
					}
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
				double min = DBL_MAX;
				double max = -DBL_MAX;
				for (int j = 0; j < img->sizeZ; j++)
					for (int i = 0; i < img->sizeY; i++)
					{
						out.data[j][i] = img->data[j][i][k];
						if (min > out.data[j][i]) min = out.data[j][i];
						if (max < out.data[j][i]) max = out.data[j][i];
					}
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
				double min = DBL_MAX;
				double max = -DBL_MAX;
				for (int j = 0; j < img->sizeZ; j++)
					for (int i = 0; i < img->sizeX; i++)
					{
						out.data[j][i] = img->data[j][k][i];
						if (min > out.data[j][i]) min = out.data[j][i];
						if (max < out.data[j][i]) max = out.data[j][i];
					}
				if (!writeT(fileName, &out, min, max)) return 0;
				oss.str("");
			}
		}
		return 1;
	}

private:

	// 24�r�b�g�t���J���[Bitmap��BitmapInfoHeader�̐ݒ�
	void setBitmapInfoHeader(long width, long height, Bmp::BitmapInfoHeader & bmih);

	// 24�r�b�g�t���J���[Bitmap��BitmapFileHeader�̐ݒ�
	void setBitmapFileHeader(unsigned long biSizeImage, Bmp::BitmapFileHeader & bmfh);

	// �R�s�[�R���X�g���N�^�C�R�s�[������Z�q�͖�����
	BitmapWriter(const BitmapWriter & obj);
	BitmapWriter & operator=(const BitmapWriter & rhs);
};


//---------------------------------------------------------------------------------------------------
// Raw�f�[�^���o�͊֌W
//---------------------------------------------------------------------------------------------------
//
// Raw�f�[�^�̓ǂݍ���
//
class RawReader {
public:
	RawReader() {}

	// read
	//  2����Raw�f�[�^�̓ǂݍ��݁D
	//  ���̊֐����Ń����������蓖�Ă�D
	//
	// ����
	//  fileName	: �t�@�C����
	//  raw			: �摜�f�[�^
	//  sizeX		: X�����T�C�Y
	//  sizeY		: Y�����T�C�Y
	//
	// �߂�l
	//  �����Ftrue�C���s�Ffalse
	template< class T > bool read(std::string fileName, DataArray2D< T > * raw, int sizeX, int sizeY) {
		// �t�@�C�����J��
		std::ifstream ifs(fileName.c_str(), std::ios_base::binary);
		if (!ifs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			//std::exit(1);
			return false;
		}
		// �摜�f�[�^�z��̐ݒ�
		raw->resize(sizeX, sizeY);
		// �摜�f�[�^�̎擾
		for (int y = 0; y < sizeY; y++) {
			ifs.read((char *)raw->data[y], sizeof(T) * raw->sizeX);
		}
		// �t�@�C�������
		ifs.close();

		return true;
	}

	// read
	//  3����Raw�f�[�^�̓ǂݍ��݁D
	//  x, y�T�C�Y���w�肷��΁Cz�����̃T�C�Y�͌���ł���̂ŁCz�����T�C�Y�̎w��͕s�v�D
	//  ���̊֐����Ń����������蓖�Ă�D
	//
	// ����
	//  fileName	: �t�@�C����
	//  raw			: �摜�f�[�^
	//  sizeX		: X�����T�C�Y
	//  sizeY		: Y�����T�C�Y
	//
	// �߂�l
	//  �����Ftrue�C���s�Ffalse
	template< class T > bool read(std::string fileName, DataArray3D<T> * raw, int sizeX, int sizeY) {
		// �t�@�C�����J��
		std::ifstream ifs(fileName.c_str(), std::ios_base::binary);
		if (!ifs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			//std::exit(1);
			return false;
		}
		// Z�����T�C�Y�̌���
		ifs.seekg(0, std::ifstream::end);
		int sizeZ = ifs.tellg() / (sizeX * sizeY * sizeof(T));
		ifs.seekg(0, std::ifstream::beg);
		// �摜�f�[�^�z��̐ݒ�
		raw->resize(sizeX, sizeY, sizeZ);
		// �摜�f�[�^�̎擾
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				ifs.read((char *)raw->data[z][y], sizeof(T) * sizeX);
			}
		}
		// �t�@�C�������
		ifs.close();

		return true;
	}

private:

	// �R�s�[�R���X�g���N�^�C�R�s�[������Z�q�͖�����
	RawReader(const RawReader & obj);
	RawReader & operator=(const RawReader & rhs);
};

//
// Raw�f�[�^�̏����o��
//
class RawWriter {
public:
	RawWriter() {}

	// write
	//  �Q����Raw�f�[�^�̏����o���D
	//  Raw�f�[�^�͉�f�l�ȊO�̏��͎����Ȃ��̂ŁCXY�����̃T�C�Y��c�����Ă������ƁD
	//
	// ����
	//  fileName : �t�@�C����
	//  raw      : �摜�f�[�^
	// 
	// �߂�l
	//  �����Ftrue�C���s�Ffalse
	template< class T > bool write(std::string fileName, DataArray2D< T > * raw) {
		// �t�@�C�����J��
		std::ofstream ofs(fileName.c_str(), std::ios_base::binary);
		if (!ofs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			return false;
		}
		// �f�[�^�̓ǂݍ���
		for (int y = 0; y < raw->sizeY; y++) {
			ofs.write((char *)raw->data[y], sizeof(T) * raw->sizeX);
		}
		// �t�@�C�������
		ofs.close();

		return true;
	}

	// write
	//  �R����Raw�f�[�^�̏����o���D
	//  Raw�f�[�^�͉�f�l�ȊO�̏��͎����Ȃ��̂ŁCXY�����̃T�C�Y��c�����Ă������ƁD
	//
	// ����
	//  fileName : �t�@�C����
	//  raw      : �摜�f�[�^
	// 
	// �߂�l
	//  �����Ftrue�C���s�Ffalse
	template< class T > bool write(std::string fileName, DataArray3D< T > * raw) {
		// �t�@�C�����J��
		std::ofstream ofs(fileName.c_str(), std::ios_base::binary);
		if (!ofs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			return false;
		}
		// �f�[�^�̓ǂݍ���
		for (int z = 0; z < raw->sizeZ; z++) {
			for (int y = 0; y < raw->sizeY; y++) {
				ofs.write((char *)raw->data[z][y], sizeof(T) * raw->sizeX);
			}
		}
		// �t�@�C�������
		ofs.close();

		return true;
	}


	// convert
	//  �R����Raw�f�[�^���Q�����V�[�P���V�����t�@�C���ɏ����o���D
	//  Raw�f�[�^�͉�f�l�ȊO�̏��͎����Ȃ��̂ŁCXY�����̃T�C�Y��c�����Ă������ƁD
	//
	// ����
	//  dirName  : �t�H���_����
	//  raw      : �摜�f�[�^
	// 
	// �߂�l
	//  �����Ftrue�C���s�Ffalse
	template< class T > bool convert(std::string dirName, DataArray3D <T> * raw)
	{
		DataArray2D <T> out;
		std::string fileName;
		std::ostringstream oss;

		out.newImage(raw->sizeX, raw->sizeY);

		for (int k = 0; k < raw->sizeZ; k++) {

			oss << k;
			if (k < 10)		 fileName = dirName + "\\" + "00" + oss.str() + ".dat";
			else if (k < 100) fileName = dirName + "\\" + "0" + oss.str() + ".dat";
			else			 fileName = dirName + "\\" + oss.str() + ".dat";

			for (int j = 0; j < raw->sizeY; j++)
				for (int i = 0; i < raw->sizeX; i++) out.data[j][i] = raw->data[k][j][i];

			if (!write(fileName, &out)) return 0;
			oss.str("");
		}

		return 1;
	}

private:

	// �R�s�[�R���X�g���N�^�C�R�s�[������Z�q�͖�����
	RawWriter(const RawWriter & obj);
	RawWriter & operator=(const RawWriter & rhs);
};

#endif // IMAGE_IO_H_
