#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

extern "C" {
#include <jpeglib.h>
}

#define WIDTH 640
#define HEIGHT 480
#define SRC_YUYV "capture.yuy2"
#define DEST_JPG "capture.jpg"

typedef unsigned char uint8_t;


int yuv422_to_rgb(void* pYUV, void* pRGB, int width, int height)
{
	if (NULL == pYUV || NULL == pRGB)
	{
		return -1;
	}
	uint8_t* pYUVData = (uint8_t *)pYUV;
	uint8_t* pRGBData = (uint8_t *)pRGB;

	int Y1, U1, V1, Y2, R1, G1, B1, R2, G2, B2;
	int C1, D1, E1, C2;

	for (int i=0; i<height; ++i)
	{
		for (int j=0; j<width/2; ++j)
		{
			Y1 = *(pYUVData+i*width*2+j*4);
			U1 = *(pYUVData+i*width*2+j*4+1);
			Y2 = *(pYUVData+i*width*2+j*4+2);
			V1 = *(pYUVData+i*width*2+j*4+3);
			C1 = Y1-16;
			C2 = Y2-16;
			D1 = U1-128;
			E1 = V1-128;
			R1 = ((298*C1 + 409*E1 + 128)>>8>255 ? 255 : (298*C1 + 409*E1 + 128)>>8);
			G1 = ((298*C1 - 100*D1 - 208*E1 + 128)>>8>255 ? 255 : (298*C1 - 100*D1 - 208*E1 + 128)>>8);
			B1 = ((298*C1+516*D1 +128)>>8>255 ? 255 : (298*C1+516*D1 +128)>>8);
			R2 = ((298*C2 + 409*E1 + 128)>>8>255 ? 255 : (298*C2 + 409*E1 + 128)>>8);
			G2 = ((298*C2 - 100*D1 - 208*E1 + 128)>>8>255 ? 255 : (298*C2 - 100*D1 - 208*E1 + 128)>>8);
			B2 = ((298*C2 + 516*D1 +128)>>8>255 ? 255 : (298*C2 + 516*D1 +128)>>8);
			*pRGBData++ = B1<0 ? 0 : B1;
			*pRGBData++ = G1<0 ? 0 : G1;
			*pRGBData++ = R1<0 ? 0 : R1;
			*pRGBData++ = B2<0 ? 0 : B2;
			*pRGBData++ = G2<0 ? 0 : G2;
			*pRGBData++ = R2<0 ? 0 : R2;
		}
	}

	return 0;
}

int yuv2_to_rgb(void* pYUV, void* pRGB, int width, int height)
{
	if (NULL == pYUV || NULL == pRGB)
	{
		return -1;
	}
		uint8_t* pSrc = (uint8_t *)pYUV;
		uint8_t* pDest = (uint8_t *)pRGB;

		for(int index = 0;index < width * height ; index += 4)
		{
			//Y0 U0 Y1 V0
			uint8_t Y0 = *pSrc;
			uint8_t U = *(++pSrc);
			uint8_t Y1 = *(++pSrc);
			uint8_t V = *(++pSrc);
			++pSrc;

			uint8_t R,G,B;

			R = (Y0 + 1.14f*V);
			G=(Y0 - 0.39f*U-0.58f*V);  
			B=(Y0 +2.03f*U);
			if(R<0) R =0;
			if(R>255) R=255;
			if(G<0) G =0;
			if(G>255) G=255;
			if(B<0) B =0;
			if(B>255) B=255;

			*(pDest) =    (uint8_t)R;         
			*(++pDest) =  (uint8_t)G;
			*(++pDest) =  (uint8_t)B;
			++pDest;

			R = (Y1 + 1.14f*V);
			G=(Y1 - 0.39f*U-0.58f*V);  
			B=(Y1 +2.03f*U)   ;
			if(R<0) R =0;
			if(R>255) R=255;
			if(G<0) G =0;
			if(G>255) G=255;
			if(B<0) B =0;
			if(B>255) B=255;

			*(pDest) =  (uint8_t)R;         
			*(++pDest) =  (uint8_t)G;
			*(++pDest) =  (uint8_t)B;
			++pDest;
		}
}

int rgb24_to_jpeg(uint8_t *rgb24data, int width, int height)
{

	//rgb24data 是整个RGB888图片的数据

	struct jpeg_compress_struct jcs;

	// 声明错误处理器，并赋值给jcs.err域
	struct jpeg_error_mgr jem;
	jcs.err = jpeg_std_error(&jem);

	jpeg_create_compress(&jcs);

	//２、指定压缩后的图像所存放的目标文件，注意，目标文件应以二进制模式打开

	FILE *jpgFile=fopen(DEST_JPG, "wb");
	if (jpgFile==NULL){
		return 0;
	}
	jpeg_stdio_dest(&jcs, jpgFile);

	//３、设置压缩参数，主要参数有

	jcs.image_width = width;      // 为图的宽和高，单位为像素
	jcs.image_height = height;
	jcs.input_components = 3;     //色彩通道数// 在此为1,表示灰度图， 如果是彩色位图，则为3
	jcs.in_color_space = JCS_RGB; //色彩空间//JCS_GRAYSCALE表示灰度图， JCS_RGB 表示彩色图像

	jpeg_set_defaults(&jcs);
	jpeg_set_quality (&jcs, 100, 1);//压缩质量

	//４、上面的工作准备完成后，就可以压缩了，压缩过程非常简单，首先调用 jpeg_start_compress，
	//然后可以对每一行进行压缩，也可以对若干行进行压缩，甚至可以对整个的图像进行一次压缩，如下：

	jpeg_start_compress(&jcs, 1);

	JSAMPROW row_pointer[1]; // 一行位图
	int row_stride;          // 每一行的字节数

	row_stride = jcs.image_width*3;  // 如果不是索引图，此处需要乘以3

	// 对每一行进行压缩
	while (jcs.next_scanline < height) {
		row_pointer[0] = & rgb24data[jcs.next_scanline * row_stride];
		jpeg_write_scanlines(&jcs, row_pointer, 1);
	}

	jpeg_finish_compress(&jcs);//压缩完成后，记得要调用jpeg_finish_compress函数

	//５、最后就是释放压缩工作过程中所申请的资源了，主要就是jpeg压缩对象，由于在本例中我是直接用的局部变量，
	//所以只需调用jpeg_destroy_compress这个函数即可，如下：

	jpeg_destroy_compress(&jcs);
	return 0;

}

//not good
int yuv422_to_jpeg(uint8_t *data, int image_width, int image_height, FILE *fp, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];  /* pointer to JSAMPLE row[s] */
	int row_stride;    /* physical row width in image buffer */
	JSAMPIMAGE  buffer;
	int band,i,buf_width[3],buf_height[3], mem_size, max_line, counter;
	uint8_t *yuv[3];
	uint8_t *pSrc, *pDst;

	yuv[0] = data;
	yuv[1] = yuv[0] + (image_width * image_height);
	yuv[2] = yuv[1] + (image_width * image_height) /2;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = image_width;  /* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;    /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;  /* colorspace of input image */

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE );

	cinfo.raw_data_in = TRUE;
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 1;

	jpeg_start_compress(&cinfo, TRUE);

	buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY));
	for(band=0; band <3; band++)
	{
		buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
		buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
		buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, buf_width[band], buf_height[band]);
	}
	max_line = cinfo.max_v_samp_factor*DCTSIZE;
	for(counter=0; cinfo.next_scanline < cinfo.image_height; counter++)
	{
		//buffer image copy.
		for(band=0; band <3; band++)
		{
			mem_size = buf_width[band];
			pDst = (uint8_t *) buffer[band][0];
			pSrc = (uint8_t *) yuv[band] + counter*buf_height[band] * buf_width[band];

			for(i=0; i <buf_height[band]; i++)
			{
				memcpy(pDst, pSrc, mem_size);
				pSrc += buf_width[band];
				pDst += buf_width[band];
			}
		}
		jpeg_write_raw_data(&cinfo, buffer, max_line);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	return 0;
}

int main()
{
	int YUVbufferSize = WIDTH * HEIGHT * 2;
	int RGBbufferSize = WIDTH * HEIGHT * 3;
	uint8_t *YUVbuffer = (uint8_t *)calloc(1, YUVbufferSize);
	uint8_t *RGBbuffer = (uint8_t *)calloc(1, RGBbufferSize);

	FILE *fpIn = fopen(SRC_YUYV, "rb");
	if(fpIn == NULL){
		printf("open source file failed!\n");
		return -1;
	}

	FILE *fpOut = fopen(DEST_JPG, "wb");
	if(fpIn == NULL){
		printf("open dest file failed!\n");
		return -1;
	}

	size_t bytes = fread(YUVbuffer, 1, YUVbufferSize, fpIn);
	printf("read %d bytes to buffer\n", bytes);

	//yuv422_to_jpeg(buffer, WIDTH, HEIGHT, fpOut, 80);

	yuv422_to_rgb(YUVbuffer, RGBbuffer, WIDTH, HEIGHT);

	fwrite(RGBbuffer, 1, RGBbufferSize, fpOut);
	
	rgb24_to_jpeg(RGBbuffer, WIDTH, HEIGHT);
	fclose(fpIn);
	fclose(fpOut);
	free(YUVbuffer);
	free(RGBbuffer);
	YUVbuffer = NULL;
	RGBbuffer = NULL;
}
