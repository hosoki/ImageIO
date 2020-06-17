/***************************************
 *
 * "ImageIO.h".h
 *
 * 画像の入出力
 *
 * Version    : 5
 * Month/Year : 05/2010
 * Author     : Shinya MAEDA, Kim Lab.
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
 /**************************************
  * 更新情報　2011/03/02
  * prefixの有無に関わらず、DICOM画像読み込み可能に変更
  * ベクトル要素のクラスの追加
  * Short型以外もBMP出力可能なwriteT関数の追加
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
#include <climits>

//フォルダ作成用
#ifdef _WIN64
#include <direct.h>
#elif __linux
#include <sys/stat.h>
#endif // _WIN64

 //並列処理を行う場合はコメントアウトをはずす
#define OPENMP

#ifdef OPENMP
#include<omp.h>
#endif

//九州歯科大学のDICOMのときはKYUSHU_DENT_UNIVをdefineする

//---------------------------------------------------------------------------------------------------
// データ配列関係
//  テンプレートを用いている
//---------------------------------------------------------------------------------------------------

template < class T > class DataArray;
template < class T > class DataArray2D;
template < class T > class DataArray3D;

//typedef unsigned char UChar;
//typedef unsigned int  UInt;
//typedef unsigned short UShort;
namespace ELEMENT {
	//
	// 2次元ベクトル型 
	//
	template < class T > class Vector2D
	{
	public:
		//コンストラクタ
		Vector2D(T x = 0, T y = 0) : X(x), Y(y) {}
		//コピーコンストラクタ
		Vector2D(Vector2D & obj) : X(obj.X), Y(obj.Y) {}
		//デストラクタ
		~Vector2D() {}
		// 値の設定
		void set(T data) { X = Y = data; }
		void set(T x, T y) {
			this->X = x;
			this->Y = y;
		}
		// 要素
		T X;
		T Y;
		//コピー代入演算子は未実装
		Vector2D & operator=(Vector2D & obj) {
			this->X = obj.X;
			this->Y = obj.Y;
			return *this;
		}
	private:
	};
	//
	// 3次元ベクトル型
	//
	template < class T > class Vector3D {
	public:
		// コンストラクタ
		Vector3D(T x = 0, T y = 0, T z = 0) : X(x), Y(y), Z(z) {}
		// コピーコンストラクタ
		Vector3D(const Vector3D &obj) : X(obj.X), Y(obj.Y), Z(obj.Z) {}
		// デストラクタ
		~Vector3D() {}

		// 値の設定
		void set(T data) { X = Y = Z = data; }
		void set(T x, T y, T z) {
			this->X = x;
			this->Y = y;
			this->Z = z;
		}
		// ファイル出力(未実装)
		// ファイル入力(未実装)
		// 要素
		T X;
		T Y;
		T Z;

		// コピー代入演算子
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

// よく使う画像配列はtypedefしておく
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
// 1次元配列クラス
//
template < class T > class DataArray {
public:
	typedef T ValueType;	// 格納するデータ型

	int size;	// 要素数
	double scale; // 寸法
	T *data;	// データ配列

	// コンストラクタ
	DataArray() : size(0), scale(1.0f), data(0) {}
	DataArray(int size) : size(size), scale(1.0f), data(0) {
		// メモリ割り当て
		data = new(std::nothrow) T[size];
		if (!data) {
			std::cerr << "Memory allocation error! DataArray::DataArray(int size)" << std::endl;
			if (getchar() == NULL) {
				return;
			}
			//std::exit(1);
			return;
		}
		// 0で初期化
		std::memset(data, 0, sizeof(T) * size);
	}

	// デストラクタ
	virtual ~DataArray() { clear(); }

	// resize
	//   配列のリサイズ
	// 
	//  引数
	//   size : 配列のサイズ
	void resize(int size) {
		// すでにメモリが割り当てられるなら開放する
		if (data != 0) { clear(); }
		// メモリの割り当て
		data = new(std::nothrow) T[size];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray::resize(int size)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		// サイズの設定
		this->size = size;
		this->scale = 1.0f;
		// 配列を0で初期化
		std::memset(data, 0, sizeof(T) * size);
	}

	// clear
	//  データのメモリを開放
	void clear() {
		// メモリが割り当てられていないならば何もしない
		if (data == 0) { return; }
		// メモリの開放
		delete[] data;
		// メンバ変数の初期化
		data = 0;
		size = 0;
	}

	// コピーコンストラクタ
	DataArray(const DataArray & obj) : size(obj.size), scale(obj.scale), data(0) {
		// メモリの割り当て
		data = new(std::nothrow) T[size];
		if (data == 0) {
			std::cerr << "Memory allocation error! DataArray::DataArray(const DataArray< T > & obj)" << std::endl;
			getchar();
			//std::exit(1);
			return;
		}
		// データのコピー
		std::memcpy(data, obj.data, sizeof(T) * size);
	}

	//データの初期化
	void init() {
		// 配列を0で初期化
		std::memset(data, 0, sizeof(T) * size);
	}

	// コピー代入演算子
	DataArray & operator=(const DataArray & rhs) {
		// 自分自身への代入の防止
		if (this == &rhs) {
			return *this;
		}
		// 自オブジェクトの後始末
		clear();
		// メモリの確保
		resize(rhs.size);
		// データのコピー
		std::memcpy(data, rhs.data, sizeof(T) * size);
		// 寸法のセット
		setScale(rhs.scale);
		// 自分を返す
		return *this;
	}

	//寸法のセット
	void setScale(double scale) {
		this->scale = scale;
	}

	//コンソール表示
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
// ２次元データ格納用クラス
//
template < class T > class DataArray2D {
public:
	typedef T ValueType;	// 画像のデータ型

	int sizeX;	// X方向サイズ
	int sizeY;	// Y方向サイズ
	Double2D scale;	//ピクセル寸法
	T **data;	// 画像データのポインタ

	// コンストラクタ
	DataArray2D() : sizeX(0), sizeY(0), scale(0.0f, 0.0f), data(0) {}
	DataArray2D(int sizeX, int sizeY) : sizeX(sizeX), sizeY(sizeY), scale(1.0f, 1.0f), data(0) {
		// メモリの割り当て
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
			// 配列を0で初期化
			std::memset(data[y], 0, sizeof(T) * this->sizeX);
		}
	}

	// デストラクタ
	virtual ~DataArray2D() { clear(); }

	// newImage
	//  画像データのメモリを確保．
	//
	// 引数
	//  sizeX : X方向サイズ
	//  sizeY : Y方向サイズ
	//
	// 戻り値
	//  成功　0, 失敗 -1を返す．
	int newImage(int sizeX, int sizeY) {
		// すでにメモリが割り当てられているならば開放する
		if (data != 0) { clear(); }
		// メモリの割り当て
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
			// 配列を0で初期化
			std::memset(data[y], 0, sizeof(T) * sizeX);
		}
		// サイズの設定
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		// スケールの設定
		this->scale.set(1.0f, 1.0f);
		return 0;
	}

	// deleteImage
	//  画像データのメモリを開放．
	void deleteImage() { clear(); }

	// resize
	//   配列のリサイズ
	// 
	//  引数
	//   sizeX : X方向サイズ
	//   sizeY : Y方向サイズ
	void resize(int sizeX, int sizeY) {
		// すでにメモリが割り当てられているならば開放する
		if (data != 0) { clear(); }
		// メモリの割り当て
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
			// 配列を0で初期化
			std::memset(data[y], 0, sizeof(T) * sizeX);
		}
		// サイズの設定
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		// スケールの設定
		this->scale.set(1.0f, 1.0f);
	}

	// clear
	//  データのメモリを開放
	void clear() {
		// メモリが割り当てられていないならば，何もしない
		if (data == 0) { return; }
		// メモリの開放
		for (int y = 0; y < sizeY; y++) {
			delete[] data[y];
		}
		delete[] data;
		// メンバ変数を初期化
		data = 0;
		sizeX = sizeY = 0;
	}


	// コピーコンストラクタ
	DataArray2D(const DataArray2D & obj) : sizeX(obj.sizeX), sizeY(obj.sizeY), scale(obj.scale.X, obj.scale.Y), data(0) {
		// メモリの割り当て
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
		// データのコピー
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				data[y][x] = obj.data[y][x];
			}
		}
	}

	// コピー代入演算子
	DataArray2D & operator=(const DataArray2D & rhs) {
		// 自分自身への代入の防止
		if (this == &rhs) { return *this; }
		// 自オブジェクトの後始末
		clear();
		// サイズのコピー
		sizeX = rhs.sizeX;
		sizeY = rhs.sizeY;
		// 寸法のコピー
		scale.X = rhs.scale.X;
		scale.Y = rhs.scale.Y;
		// メモリの割り当て
		resize(rhs.sizeX, rhs.sizeY);
		// データのコピー
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				data[y][x] = rhs.data[y][x];
			}
		}
		// 自分を帰す
		return *this;
	}

	//データの初期化
	void init() {
		for (int y = 0; y < sizeY; y++) {
			// 配列を0で初期化
			std::memset(data[y], 0, sizeof(T) * sizeX);
		}
	}

	//ピクセル寸法のセット
	void setScale(const Double2D &scale) {
		this->scale = scale;
	}
	void setScale(double scaleX, double scaleY) {
		this->scale.X = scaleX;
		this->scale.Y = scaleY;
	}

	//コンソール表示
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
typedef DataArray2D<short> ShortImage2D;	// 2次元Dicom画像用
typedef DataArray2D<int> IntImage2D;
typedef DataArray2D<unsigned int> UIntImage2D;
typedef DataArray2D<float> FloatImage2D;
typedef DataArray2D<double> DoubleImage2D;


//
// 3次元データ格納用クラス
//
template< class T > class DataArray3D {
public:
	typedef T ValueType;	// 画像のデータ型

	int sizeX;	// X方向サイズ
	int sizeY;	// Y方向サイズ
	int sizeZ;	// Z方向サイズ
	Double3D scale;	// ボクセル寸法
	T ***data;	// 画像データのポインタ

	// コンストラクタ
	DataArray3D() : sizeX(0), sizeY(0), sizeZ(0), scale(1.0f, 1.0f, 1.0f), data(0) {}
	DataArray3D(int sizeX, int sizeY, int sizeZ) : sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ), scale(1.0, 1.0, 1.0), data(0) {
		// メモリの割り当て
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
				// 配列を0で初期化
				std::memset(data[z][y], 0, sizeof(T) * this->sizeX);
			}
		}
	}

	// デストラクタ
	virtual ~DataArray3D() { clear(); }

	// newImage
	//  画像データのメモリを確保．
	//
	// 引数
	//  sizeX : X方向サイズ
	//  sizeY : Y方向サイズ
	//  sizeZ : Z方向サイズ
	//
	// 戻り値
	//  成功　0, 失敗 -1を返す．
	int newImage(int sizeX, int sizeY, int sizeZ) {
		// すでにメモリが割り当てられているならば開放する
		if (data != 0) { deleteImage(); }
		// メモリの割り当て
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
				// 配列を0で初期化
				std::memset(data[z][y], 0, sizeof(T) * sizeX);
			}
		}
		// サイズの設定
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->sizeZ = sizeZ;
		this->scale.set(1.0f, 1.0f, 1.0f);
		return 0;
	}

	// deleteImage
	//  画像データのメモリを開放．
	void deleteImage() { clear(); }

	// resize
	//   配列のリサイズ
	// 
	//  引数
	//   sizeX : X方向サイズ
	//   sizeY : Y方向サイズ
	//   sizeZ : Z方向サイズ
	void resize(int sizeX, int sizeY, int sizeZ) {
		// すでにメモリが割り当てられているならば開放する
		if (data != 0) { clear(); }
		// メモリの割り当て
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
				// 配列を0で初期化
				std::memset(data[z][y], 0, sizeof(T) * sizeX);

			}
		}
		// サイズの設定
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->sizeZ = sizeZ;
		this->scale.set(1.0f, 1.0f, 1.0f);
	}

	// clear
	//  データのメモリを開放
	void clear() {
		// メモリが割り当てられていないならば，何もしない
		if (data == 0) { return; }
		// メモリの開放
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				delete[] data[z][y];
			}
			delete[] data[z];
		}
		delete[] data;
		// メンバ変数の初期化
		data = 0;
		this->scale.set(1.0f, 1.0f, 1.0f);
		sizeX = sizeY = sizeZ = 0;
	}

	// コピーコンストラクタ
	DataArray3D(const DataArray3D & obj) : sizeX(obj.sizeX), sizeY(obj.sizeY), sizeZ(obj.sizeZ), scale(obj.scale), data(0) {
		// メモリの割り当て
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
		// データのコピー
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = obj.data[z][y][x];
				}
			}
		}
	}

	// コピー代入演算子
	DataArray3D & operator=(const DataArray3D & rhs) {
		// 自分自身への代入の防止
		if (this == &rhs) { return *this; }
		// 自オブジェクトの後始末
		clear();
		// サイズのコピー
		sizeX = rhs.sizeX;
		sizeY = rhs.sizeY;
		sizeZ = rhs.sizeZ;
		// ボクセル寸法のコピー
		scale = rhs.scale;
		// メモリの割り当て
		resize(rhs.sizeX, rhs.sizeY, rhs.sizeZ);
		// データのコピー
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = rhs.data[z][y][x];
				}
			}
		}
		// 自分を帰す
		return *this;
	}

	//データの初期化
	void init() {
		for (int z = 0; z < this->sizeZ; z++) {
			for (int y = 0; y < this->sizeY; y++) {
				// 配列を0で初期化
				std::memset(data[z][y], 0, sizeof(T) * this->sizeX);
			}
		}
	}

	//ボクセルスケールのセット
	void setScale(const Double3D &scale) {
		this->scale = scale;
	}

	//寸法設定
	void setScale(double scaleX, double scaleY, double scaleZ) {
		this->scale.set(scaleX, scaleY, scaleZ);
	}
private:
};

typedef DataArray3D<bool> BoolImage3D;
typedef DataArray3D<char> CharImage3D;
typedef DataArray3D<unsigned char> UCharImage3D;
typedef DataArray3D<short> ShortImage3D;	// 3次元Dicom画像用
typedef DataArray3D<int> IntImage3D;
typedef DataArray3D<unsigned int> UIntImage3D;
typedef DataArray3D<float> FloatImage3D;
typedef DataArray3D<double> DoubleImage3D;


////////////////////////////////////////////////////////////////////////////
//頭部位置合わせ用

template< class T > class DataArray_verK_3D {
public:
	typedef T ValueType;	// 画像のデータ型

	int sizeX;	// X方向サイズ
	int sizeY;	// Y方向サイズ
	int sizeZ;	// Z方向サイズ
	int centerX;
	int centerY;
	int centerZ;
	int VOISize;
	T ***data;	// 画像データのポインタ

	// コンストラクタ
	DataArray_verK_3D() : sizeX(0), sizeY(0), sizeZ(0), data(0) {}
	DataArray_verK_3D(int sizeX, int sizeY, int sizeZ) : sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ), data(0) {
		// メモリの割り当て
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
				// 配列を0で初期化
				std::memset(data[z][y], 0, sizeof(T) * this->sizeX);
			}
		}
	}

	// デストラクタ
	virtual ~DataArray_verK_3D() { clear(); }

	// newImage
	//  画像データのメモリを確保．
	//
	// 引数
	//  sizeX : X方向サイズ
	//  sizeY : Y方向サイズ
	//  sizeZ : Z方向サイズ
	//
	// 戻り値
	//  成功　0, 失敗 -1を返す．
	int newImage(int sizeX, int sizeY, int sizeZ) {
		newImage(sizeX, sizeY, sizeZ, 0, 0, 0, 0);
		return 0;
	}

	// newImage
	//  画像データのメモリを確保．
	//
	// 引数
	//  sizeX : X方向サイズ
	//  sizeY : Y方向サイズ
	//  sizeZ : Z方向サイズ
	//
	// 戻り値
	//  成功　0, 失敗 -1を返す．
	int newImage(int sizeX, int sizeY, int sizeZ, int centerX, int centerY, int centerZ, int VOISize) {
		// すでにメモリが割り当てられているならば開放する
		if (data != 0) { deleteImage(); }
		// メモリの割り当て
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
				// 配列を0で初期化
				std::memset(data[z][y], 0, sizeof(T) * sizeX);
			}
		}
		// サイズの設定
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
	//  画像データのメモリを開放．
	void deleteImage() { clear(); }

	// resize
	//   配列のリサイズ
	// 
	//  引数
	//   sizeX : X方向サイズ
	//   sizeY : Y方向サイズ
	//   sizeZ : Z方向サイズ
	void resize(int sizeX, int sizeY, int sizeZ) {
		resize(sizeX, sizeY, sizeZ, 0, 0, 0, 0);
	}

	// resize
	//   配列のリサイズ
	// 
	//  引数
	//   sizeX : X方向サイズ
	//   sizeY : Y方向サイズ
	//   sizeZ : Z方向サイズ
	void resize(int sizeX, int sizeY, int sizeZ, int centerX, int centerY, int centerZ, int VOISize) {
		// すでにメモリが割り当てられているならば開放する
		if (data != 0) { clear(); }
		// メモリの割り当て
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
				// 配列を0で初期化
				std::memset(data[z][y], 0, sizeof(T) * sizeX);

			}
		}
		// サイズの設定
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->sizeZ = sizeZ;
		this->centerX = centerX;
		this->centerY = centerY;
		this->centerZ = centerZ;
		this->VOISize = VOISize;
	}

	// clear
	//  データのメモリを開放
	void clear() {
		// メモリが割り当てられていないならば，何もしない
		if (data == 0) { return; }
		// メモリの開放
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				delete[] data[z][y];
			}
			delete[] data[z];
		}
		delete[] data;
		// メンバ変数の初期化
		data = 0;
		sizeX = sizeY = sizeZ = 0;
	}

	// コピーコンストラクタ
	DataArray_verK_3D(const DataArray_verK_3D & obj) : sizeX(obj.sizeX), sizeY(obj.sizeY), sizeZ(obj.sizeZ), data(0) {
		// メモリの割り当て
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
		// データのコピー
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = obj.data[z][y][x];
				}
			}
		}
	}

	// コピー代入演算子
	DataArray_verK_3D & operator=(const DataArray_verK_3D & rhs) {
		// 自分自身への代入の防止
		if (this == &rhs) { return *this; }
		// 自オブジェクトの後始末
		clear();
		// サイズのコピー
		sizeX = rhs.sizeX;
		sizeY = rhs.sizeY;
		// メモリの割り当て
		resize(rhs.sizeX, rhs.sizeY, rhs.sizeZ, rhs.centerX, rhs.centerY, rhs.centerZ, rhs.VOISize);
		// データのコピー
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					data[z][y][x] = rhs.data[z][y][x];
				}
			}
		}
		// 自分を帰す
		return *this;
	}
private:
};


// よく使う画像配列はtypedefしておく
typedef DataArray_verK_3D< short > ShortImage3DK; // アフィン用に作ったよ～ん

//頭部位置合わせ用
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------
// Dicom画像読み込み処理関係
//  ShorImage2Dのみに対応
//---------------------------------------------------------------------------------------------------

//
// DICOM画像情報
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
	double thickness;		// 0x0018 0x0050 スライス厚
	int seriesNumber;		// 0x0020 0x0011 シリーズ番号
	int instanceNumber;		// 0x0020 0x0013 画像番号
	unsigned short rows;	// 0x0028 0x0010 画像のYサイズ
	unsigned short columns;	// 0x0028 0x0011 画像のXサイズ
	double pixelSpacingX;	// 0x0028 0x0030 画素のXサイズ
	double pixelSpacingY;	// 0x0028 0x0030 画素のYサイズ
	int windowCenter;		// 0x0028 0x1050 ウィンドウ中心 
	int windowWidth;		// 0x0028 0x1051 ウィンドウ幅
};

//
// DICOMの読み込み
//
class DicomReader {
public:
	// 画像情報
	DicomImageInfo info;

	DicomReader() :
		ifs_(), prefix(NULL), implicitVR_(true), littleEndian_(true), sizeX_(0), sizeY_(0) {}

	// Dicom画像の読み取り
	bool read(std::string fileName, ShortImage2D * dcm);

	// Dicom画像情報の取得
	bool getInfo(std::string fileName, DicomImageInfo * info);

private:
	std::ifstream ifs_;	// 入力ファイルストリーム
	bool prefix;		//プリフィクスがあるかどうか
	bool implicitVR_;	// 暗黙的VRかどうかのフラグ
	bool littleEndian_;	// リトルエンディアンかどうかフラグ
	int sizeX_, sizeY_; // 画像サイズ

	// タグの読み取り
	unsigned int getTag();

	// 転送構文の読み取り
	// implicitVR_, littleEndian_のフラグを設定する
	void setTransferSyntax(std::string transferSyntax);

	// 値長さの取得
	unsigned int getValueLength() {
		int dummy = 0;
		return getValueLength(&dummy);
	}
	unsigned int getValueLength(int * byte);

	// 文字列として読み込む
	std::string getStr(int valueLength);

	// DICOMファイルメタ情報をスキャンする
	bool scanDicomMetaInfo();

	// DICOMデータの読み取り
	bool scan();

	// コピーコンストラクタ，コピー代入演算子は未実装
	DicomReader(const DicomReader & obj);
	DicomReader & operator=(const DicomReader & rhs);
};


//---------------------------------------------------------------------------------------------------
// Bitmap画像入出力関係
//  24ビットフルカラーにのみ対応．
//  RGBConverterでDataArray2DをRGBImage型に変換してから出力すること．
//  ShortImage2Dは変換せずにモノクロ画像で出力することも可能．
//  色づけしたい場合はRGBConverterでRGBImageに変換すること．
//---------------------------------------------------------------------------------------------------

//
// Bmp名前空間
//  Bitmap関係の処理で用いるものをまとめている
//
namespace Bmp {
	// RGB画像の最大輝度値
	const int RGBMaxIntensity = 255;

	// BitmapInfoHeader構造体
	struct BitmapInfoHeader {
		unsigned biSize;				// 情報ヘッダサイズ（byte）
		unsigned biWidth;				// 画像の幅（pixel）
		unsigned biHeight;				// 画像の高さ（pixel）
		unsigned short biPlanes;		// プレーン数（常に1）
		unsigned short biBitCount;		// 色ビット数
		unsigned biCompression;			// 圧縮形式
		unsigned biSizeImage;			// 画像データサイズ（byte）
		unsigned biXPelsPerMeter;		// 水平解像度
		unsigned biYPelsPerMeter;		// 垂直解像度
		unsigned biClrUsed;				// 格納パレット数[使用色数]
		unsigned biClrImportant;		// 重要色数
	};

	// BitmapFileHeader構造体
	struct BitmapFileHeader {
		unsigned short bfType;		// ファイルタイプ．'BM'
		unsigned bfSize;			// ファイルサイズ（byte）
		unsigned short bfReserved1;	// 予約領域1．常に0．
		unsigned short bfReserved2;	// 予約領域2．常に0．
		unsigned bfOffBits;			// ファイル先頭から画像データまでのオフセット（byte）
	};
};

//
// RGB型
//
class RGBType {
public:
	// コンストラクタ
	RGBType(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0) : r(r), g(g), b(b) {}
	// 値の設定
	void setElement(unsigned char color) { r = g = b = color; }
	void setElement(unsigned char r, unsigned char g, unsigned char b) {
		this->r = r;
		this->g = g;
		this->b = b;
	}
	// RGB値
	unsigned char b;
	unsigned char g;
	unsigned char r;

private:
};

//
// 2次元RGB画像配列
//
typedef DataArray2D< RGBType > RGBImage;
typedef DataArray3D< RGBType > RGBImage3D;

//
// DataArray2DからRGBImageへの変換
//
class RGBConverter {
public:
	RGBConverter() {}

	// DataArray2DからRGBImageへの変換
	//  DataArray2D内のデータ値の変換範囲を指定して変換する．
	// 
	// 引数
	// 　min, max : 変換するDataArray2Dの値の範囲の最小値・最大値
	template< class T > void convert(DataArray2D< T > * from, RGBImage * to, T min, T max) {
		// スケーリングのパラメータを求める
		double slop = Bmp::RGBMaxIntensity / static_cast<double>(max - min);
		double intercept = -slop * min;
		// 画像のリサイズ
		if ((from->sizeX != to->sizeX) || (from->sizeY != to->sizeX)) {
			to->resize(from->sizeX, from->sizeY);
		}
		// 変換
		for (int y = 0; y < from->sizeY; y++) {
			for (int x = 0; x < from->sizeX; x++) {
				double temp = slop * from->data[y][x] + intercept;
				unsigned char color = static_cast<unsigned char>(temp + ((temp >= 0.0) ? 0.5 : -0.5)); // 四捨五入
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

	// DataArray2DからRGBImageへの変換
	//  from内の最小値・最大値を読み取って，その範囲でRGBに変換する．
	template< class T > void convert(DataArray2D< T > * from, RGBImage * to) {
		T max = from->data[0][0], min = from->data[0][0];
		// 最大・最小値を見つける
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
		// 変換
		convert(from, to, min, max);
	}

	// DataArray3DからRGBImageへの変換
	template< class T > void convert(DataArray3D< T > * from, RGBImage * to, T max, T min) {

		DataArray2D<T> tmp(from->sizeX, from->sizeY);
		RGBImage tmp2(from->sizeX, from->sizeY);
		to->resize(from->sizeX, from->sizeY, from->sizeZ);
		for (int k = 0; k < from->sizeZ; k++) {
			for (int j = 0; j < from->sizeY; j++)
				for (int i = 0; i < from->sizeX; i++) tmp.data[j][i] = from->data[k][j][i];

			// 変換
			convert(&tmp, &tmp2, min, max);
			for (int j = 0; j < from->sizeY; j++)
				for (int i = 0; i < from->sizeX; i++) to->data[j][i] = tmp2.data[j][i];
				//for (int i = 0; i < from->sizeX; i++) to->data[k][j][i] = tmp2.data[j][i];
		}

	}

	// DataArray3DからRGBImageへの変換
	//  from内の最小値・最大値を読み取って，その範囲でRGBに変換する．
	template< class T > void convert(DataArray3D< T > * from, RGBImage * to) {
		T max = from->data[0][0][0], min = from->data[0][0][0];
		// 最大・最小値を見つける		
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
		// 変換
		convert(from, to, min, max);
	}



private:

	// コピーコンストラクタ，コピー代入演算子は未実装
	RGBConverter(const RGBConverter & obj);
	RGBConverter & operator=(const RGBConverter & rhs);
};

//
// Bitmap画像の読み込み
//
class BitmapReader {
public:
	BitmapReader() {}

	// read
	//  Bitmap画像の読み込み
	//
	// 引数
	//  fileName	: 画像ファイル名
	//  img			: 画像配列
	//
	// 返り値
	//  読み込み成功でtrue，読み込み失敗でfalseを返す
	bool read(std::string fileName, RGBImage *img);

private:
	// コピーコンストラクタ，コピー代入演算子は未実装
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
// Bitmap画像の書き出し
//
class BitmapWriter {
public:
	BitmapWriter() {}

	// write
	//  Bitmap画像の書き出し（RGBImage -> Bitmap）
	// 
	// 引数
	//  fileName	: 出力ファイル名
	//  img			: 出力する画像
	//
	// 戻り値
	//  出力成功でtrue，失敗でfalseを返す
	bool write(std::string fileName, RGBImage * img);
	bool write(std::string dirName, RGBImage3D * img, int plane = 0);

	// write
//  Bitmap画像の書き出し（ShortImage2D -> Bitmap）
// 
// 引数
//	fileName	: 出力ファイル名
//  img			: 出力する画像
//	wl			: window level
//	ww			: window width
//  ganma       : ganma値
//
// 戻り値
//  出力成功でtrue，失敗でfalseを返す
	bool write(std::string fileName, ShortImage2D * img, short wl, short ww, double ganma);

	// write
	//  Bitmap画像の書き出し（ShortImage3D -> Bitmap）
	// 
	// 引数
	//	dirName		: 出力ディレクトリ名
	//  img			: 出力する画像
	//	wl			: window level
	//	ww			: window width
	//  ganma       : ganma値
	//
	// 戻り値
	//  出力成功でtrue，失敗でfalseを返す
	bool write(std::string dirName, ShortImage3D * img, short wl, short ww, double ganma, int plane);

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
	bool write(std::string dirName, ShortImage3D * img, double min, double max, int plane = 0);
	bool write(std::string dirName, ShortImage3D * img, int plane = 0)
	{
		ShortImage2D out;
		std::string fileName;
		std::ostringstream oss;

		//フォルダ作成
#ifdef _WIN64
		if (!_mkdir(dirName.c_str()))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

#elif __linux
		if (!mkdir(dirName.c_str(), 0777))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

#endif // _WIN64

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
	template <class T> bool writeT(std::string fileName, DataArray2D<T> * img, double min, double max)
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
	//  Bitmap画像の書き出し（ShortImage2D -> Bitmap）
	// 
	// 引数
	//  fileName	: 出力ファイル名
	//  img			: 出力する画像
	//
	// 戻り値
	//  出力成功でtrue，失敗でfalseを返す
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
	template <class T> bool writeT(std::string dirName, DataArray3D<T> * img, double min, double max, int plane = 0)
	{
		DataArray2D<T> out;
		std::string fileName;
		std::ostringstream oss;

		//フォルダ作成
#ifdef _WIN64
		if (!_mkdir(dirName.c_str()))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

#elif __linux
		if (!mkdir(dirName.c_str(), 0777))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

#endif // _WIN64

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
	//  Bitmap画像の書き出し（ShortImage3D -> Bitmap）
	// 
	// 引数
	//	dirName		: 出力ディレクトリ名
	//  img			: 出力する画像
	//
	// 戻り値
	//  出力成功でtrue，失敗でfalseを返す
	template <class T> bool writeT(std::string dirName, DataArray3D<T> * img, int plane = 0)
	{
		DataArray2D<T> out;
		std::string fileName;
		std::ostringstream oss;

		//フォルダ作成
#ifdef _WIN64
		if (!_mkdir(dirName.c_str()))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

#elif __linux
		if (!mkdir(dirName.c_str(), 0777))
			std::cerr << "Make folder \"" << dirName << "\"" << std::endl;

#endif // _WIN64

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

	// 24ビットフルカラーBitmapのBitmapInfoHeaderの設定
	void setBitmapInfoHeader(long width, long height, Bmp::BitmapInfoHeader & bmih);

	// 24ビットフルカラーBitmapのBitmapFileHeaderの設定
	void setBitmapFileHeader(unsigned long biSizeImage, Bmp::BitmapFileHeader & bmfh);

	// コピーコンストラクタ，コピー代入演算子は未実装
	BitmapWriter(const BitmapWriter & obj);
	BitmapWriter & operator=(const BitmapWriter & rhs);
};


//---------------------------------------------------------------------------------------------------
// Rawデータ入出力関係
//---------------------------------------------------------------------------------------------------
//
// Rawデータの読み込み
//
class RawReader {
public:
	RawReader() {}

	// read
	//  2次元Rawデータの読み込み．
	//  この関数内でメモリを割り当てる．
	//
	// 引数
	//  fileName	: ファイル名
	//  raw			: 画像データ
	//  sizeX		: X方向サイズ
	//  sizeY		: Y方向サイズ
	//
	// 戻り値
	//  成功：true，失敗：false
	template< class T > bool read(std::string fileName, DataArray2D< T > * raw, int sizeX, int sizeY) {
		// ファイルを開く
		std::ifstream ifs(fileName.c_str(), std::ios_base::binary);
		if (!ifs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			//std::exit(1);
			return false;
		}
		// 画像データ配列の設定
		raw->resize(sizeX, sizeY);
		// 画像データの取得
		for (int y = 0; y < sizeY; y++) {
			ifs.read((char *)raw->data[y], sizeof(T) * raw->sizeX);
		}
		// ファイルを閉じる
		ifs.close();

		return true;
	}

	// read
	//  3次元Rawデータの読み込み．
	//  x, yサイズを指定すれば，z方向のサイズは決定できるので，z方向サイズの指定は不要．
	//  この関数内でメモリを割り当てる．
	//
	// 引数
	//  fileName	: ファイル名
	//  raw			: 画像データ
	//  sizeX		: X方向サイズ
	//  sizeY		: Y方向サイズ
	//
	// 戻り値
	//  成功：true，失敗：false
	template< class T > bool read(std::string fileName, DataArray3D<T> * raw, int sizeX, int sizeY) {
		// ファイルを開く
		std::ifstream ifs(fileName.c_str(), std::ios_base::binary);
		if (!ifs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			//std::exit(1);
			return false;
		}
		// Z方向サイズの決定
		ifs.seekg(0, std::ifstream::end);
		int sizeZ = ifs.tellg() / (sizeX * sizeY * sizeof(T));
		ifs.seekg(0, std::ifstream::beg);
		// 画像データ配列の設定
		raw->resize(sizeX, sizeY, sizeZ);
		// 画像データの取得
		for (int z = 0; z < sizeZ; z++) {
			for (int y = 0; y < sizeY; y++) {
				ifs.read((char *)raw->data[z][y], sizeof(T) * sizeX);
			}
		}
		// ファイルを閉じる
		ifs.close();

		return true;
	}

private:

	// コピーコンストラクタ，コピー代入演算子は未実装
	RawReader(const RawReader & obj);
	RawReader & operator=(const RawReader & rhs);
};

//
// Rawデータの書き出し
//
class RawWriter {
public:
	RawWriter() {}

	// write
	//  ２次元Rawデータの書き出し．
	//  Rawデータは画素値以外の情報は持たないので，XY方向のサイズを把握しておくこと．
	//
	// 引数
	//  fileName : ファイル名
	//  raw      : 画像データ
	// 
	// 戻り値
	//  成功：true，失敗：false
	template< class T > bool write(std::string fileName, DataArray2D< T > * raw) {
		// ファイルを開く
		std::ofstream ofs(fileName.c_str(), std::ios_base::binary);
		if (!ofs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			return false;
		}
		// データの読み込み
		for (int y = 0; y < raw->sizeY; y++) {
			ofs.write((char *)raw->data[y], sizeof(T) * raw->sizeX);
		}
		// ファイルを閉じる
		ofs.close();

		return true;
	}

	// write
	//  ３次元Rawデータの書き出し．
	//  Rawデータは画素値以外の情報は持たないので，XY方向のサイズを把握しておくこと．
	//
	// 引数
	//  fileName : ファイル名
	//  raw      : 画像データ
	// 
	// 戻り値
	//  成功：true，失敗：false
	template< class T > bool write(std::string fileName, DataArray3D< T > * raw) {
		// ファイルを開く
		std::ofstream ofs(fileName.c_str(), std::ios_base::binary);
		if (!ofs) {
			std::cerr << "Failed to open file \"" << fileName << "\"" << std::endl;
			return false;
		}
		// データの読み込み
		for (int z = 0; z < raw->sizeZ; z++) {
			for (int y = 0; y < raw->sizeY; y++) {
				ofs.write((char *)raw->data[z][y], sizeof(T) * raw->sizeX);
			}
		}
		// ファイルを閉じる
		ofs.close();

		return true;
	}


	// convert
	//  ３次元Rawデータを２次元シーケンシャルファイルに書き出し．
	//  Rawデータは画素値以外の情報は持たないので，XY方向のサイズを把握しておくこと．
	//
	// 引数
	//  dirName  : フォルダ名名
	//  raw      : 画像データ
	// 
	// 戻り値
	//  成功：true，失敗：false
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

	// コピーコンストラクタ，コピー代入演算子は未実装
	RawWriter(const RawWriter & obj);
	RawWriter & operator=(const RawWriter & rhs);
};

#endif // IMAGE_IO_H_
