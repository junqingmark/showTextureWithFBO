#include "imageReader.h"
#include <string>
#include <fstream>
#include <png.h>
#include "jpeglib.h"
#include <stdio.h>

imageReader* imageReader::readerInstance = NULL;

imageReader::imageReader()
{
	m_width = 0;
	m_height = 0;
	m_depth = 0;
}

imageReader::~imageReader()
{

}

imageReader* imageReader::createInstance()
{
	if (NULL == readerInstance)
	{
		readerInstance = new imageReader();
	}

	return readerInstance;
}

void imageReader::releaseInstance()
{
	if (readerInstance)
	{
		delete readerInstance;
		readerInstance = NULL;
	}
}

unsigned int imageReader::getImageWidth()
{
	return m_width;
}

unsigned int imageReader::getImageHeight()
{
	return m_height;
}

int imageReader::getImageDepth()
{
	return m_depth;
}

char* loadBMP(string& fileName, unsigned int& width, unsigned int& height, int& depth)
{
	char bmpHeader[54];
	ifstream ifs;
	ifs.open(fileName.c_str(), ios::in);
	if (ifs.is_open())
	{
		ifs.read(bmpHeader, 54);
	}
	else
	{
		cout << "Fail to open file: " << fileName << endl;
		return NULL;
	}

	if (bmpHeader[0] != 0x42 || bmpHeader[1] != 0x4d)
	{
		ifs.close();
		cout << "not a bmp file!" << endl;
		return NULL;
	}

	int bmpTotalSize = (unsigned char)bmpHeader[3] + (((unsigned char)bmpHeader[4]) << 8) + (((unsigned char)bmpHeader[5]) << 16) + (((unsigned char)bmpHeader[6]) << 24);
	cout << "The total size of the bmp file is " << bmpTotalSize << endl;

	width = (unsigned char)bmpHeader[18] + (((unsigned char)bmpHeader[19]) << 8) + (((unsigned char)bmpHeader[20]) << 16) + (((unsigned char)bmpHeader[21]) << 24);
	height = (unsigned char)bmpHeader[22] + (((unsigned char)bmpHeader[23]) << 8) + (((unsigned char)bmpHeader[24]) << 16) + (((unsigned char)bmpHeader[25])  << 24);

	int bitsPerPixel = bmpHeader[28];
	if (bitsPerPixel != 24 && bitsPerPixel != 32)
	{
		ifs.close();
		cout << "This is not a 24 bit or 32 bit bitmap!" << endl;
		return NULL;
	}

	//int bmpPixelSize = bmpHeader[34] + (bmpHeader[35] << 8) + (bmpHeader[36] << 16) + (bmpHeader[37] << 24);
	int bmpPixelSize = bitsPerPixel/8 * width * height;
	depth = bitsPerPixel;

	char* bmpPixel = new char[bmpPixelSize];
	int cur = ifs.tellg();

	cout << "The Pixel current position is " << cur << endl;
	ifs.read(bmpPixel, bmpPixelSize);

	int i;
	char tmp;
	if (24 == depth)
	{
		for (i = 0; i < bmpPixelSize; i += 3)
		{
			tmp = bmpPixel[i];
			bmpPixel[i] = bmpPixel[i + 2];
			bmpPixel[i + 2] = tmp;
		}
	}
	else if (32 == depth)
	{
		for (i = 0; i < bmpPixelSize; i += 4)
		{
			tmp = bmpPixel[i];
			bmpPixel[i] = bmpPixel[i + 2];
			bmpPixel[i + 2] = tmp;
		}
	}
	
	return bmpPixel;

	ifs.close();

}


char* loadPNG(string& fileName, unsigned int& width, unsigned int& height, int& depth)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers;
	int color_type, interlace_type;
	char buf[8];//png hender previous 8 bytes are fixed value
	FILE* fp = NULL;

	fp = fopen(fileName.c_str(), "rb");
	if (fp)
	{
		fread(buf, 1, 8, fp);
	}
	else
	{
		cout << "Fail to open file: " << fileName << endl;
		return NULL;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == png_ptr)
	{
		fclose(fp);
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (NULL == info_ptr)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return NULL;
	}
	
	
	if (png_sig_cmp((png_bytep)buf, 0, 8))
	{
		fclose(fp);
		cout << "not a png file!" << endl;
		return NULL;
	}

	rewind( fp );
	
	png_init_io(png_ptr, fp);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, NULL);
	color_type = png_get_color_type(png_ptr, info_ptr);
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	row_pointers = png_get_rows(png_ptr, info_ptr);

	char* pngPixel;

	unsigned long i, j, k;

	switch(color_type)
	{
		case PNG_COLOR_TYPE_RGB:
		{
			k = 0;
			depth = 24;
			pngPixel = new char[3 * width * height];
			cout << "pngPixel == " << pngPixel << endl;
			if (NULL == pngPixel)
			{
				cout << "Fail to Create pngPixel" << endl;
				return NULL;
			}
			for (i = 0; i < height; ++i)
			{
				for (j = 0; j < width * 3; j += 3)
				{
					pngPixel[k++] = row_pointers[i][j];
					pngPixel[k++] = row_pointers[i][j + 1];
					pngPixel[k++] = row_pointers[i][j + 2];
				}
			}
		}
		break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
		{
			k = 0;
			depth = 32;
			pngPixel = new char[4 * width * height];
			if (NULL == pngPixel)
			{
				cout << "Fail to Create pngPixel" << endl;
				return NULL;
			}
			for (i = 0; i < height; ++i)
			{
				for (j = 0; j < width * 4; j += 4)
				{
					pngPixel[k++] = row_pointers[i][j];
					pngPixel[k++] = row_pointers[i][j + 1];
					pngPixel[k++] = row_pointers[i][j + 2];
					pngPixel[k++] = row_pointers[i][j + 3];
				}
			}
		}
		break;

	}

	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return pngPixel;

}

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

void my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}



char* loadJPG(string& fileName, unsigned int& width, unsigned int& height, int& depth)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;
	char* jpgPixel;

	FILE* fp = NULL;

	fp = fopen(fileName.c_str(), "rb");
	if(NULL == fp)
	{
		cout << "Fail to open file: " << fileName << endl;
		return NULL;
	}

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		 jpeg_destroy_decompress(&cinfo);
		 fclose(fp);
		 return 0;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	width = cinfo.output_width;
	height = cinfo.output_height;

	if (cinfo.out_color_space == JCS_RGB)
	{
		depth = 24;

	}
	else if(cinfo.out_color_space == JCS_GRAYSCALE)
	{
		depth = 8;
	}

	int k =0;
	row_stride = cinfo.output_width * cinfo.output_components;
	jpgPixel = new char[row_stride * height];
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);
		int i;
		for ( i = 0; i < row_stride; ++i)
		{
			jpgPixel[k++] = buffer[0][i];
		}

	}

	(void) jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);
	fclose(fp);

	return jpgPixel;
}

char* imageReader::loadImage(string& fileName)
{
	if("" == fileName)
	{
		cout << "file name empty!" << endl;
	}

	if (fileName.find(".bmp") != string::npos)
	{
		return loadBMP(fileName, m_width, m_height, m_depth);
	}
	else if (fileName.find(".png") != string::npos)
	{
		return loadPNG(fileName, m_width, m_height, m_depth);
	}
	else if (fileName.find(".jpg") != string::npos || fileName.find(".jpeg") != string::npos)
	{
		return loadJPG(fileName, m_width, m_height, m_depth);
	}
}