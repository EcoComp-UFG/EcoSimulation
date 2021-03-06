//@author: João Gabriel Silva Fernandes
#include <stdio.h>
#include "decompressData.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
int l2b_endian(int num){
	uint32_t b0,b1,b2,b3;
	uint32_t res;

	b0 = (num & 0x000000ff) << 24u;
	b1 = (num & 0x0000ff00) << 8u;
	b2 = (num & 0x00ff0000) >> 8u;
	b3 = (num & 0xff000000) >> 24u;

	return (b0 | b1 | b2 | b3);
}



 /******** retirado e adaptado de: https://zlib.net/zlib_how.html e https://zlib.net/manual.html ****************/

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int my_decompress_to_file(char source_str[], char dest_str[]){
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	FILE *source = fopen(source_str, "rb");
	FILE *dest = fopen(dest_str, "wb");

	if(!source || !dest){ 
		printf("erro ao abrir arquivos\n");
		return Z_ERRNO;
	}

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;
	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source))
		{
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;
		/* run inflate() on input until output buffer not full */
		do
		{
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR); /* state not clobbered */
			switch (ret)
			{
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR; /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);
	/* clean up and return */
	(void)inflateEnd(&strm);
	fclose(dest);
	fclose(source);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* Decompress from file source to stream_dest (byte array) until source ends or EOF.

   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. 

See header file por parameters details
*/
int my_decompress(FILE *source, byte **dest_stream, size_t *resulting_total_size){

	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];
	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	//printf("na func my my_decompress 125\n");
	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)     //if fread returns 0 -> 0 bytes found
			break;
		strm.next_in = in;
		 /* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);       //the actual decompressing work
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}

			have = CHUNK - strm.avail_out;
			*dest_stream = (byte *)realloc(*dest_stream, (*resulting_total_size)+have);
			if(dest_stream == NULL){
				perror("Realloc falhou em my_decompress");
				(void)inflateEnd(&strm);
				return Z_MEM_ERROR;
			}
			memcpy( *dest_stream+ *resulting_total_size, out, have );

			(*resulting_total_size) = *resulting_total_size + have;

		} while (strm.avail_out == 0);
		 /* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);
	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


/*
	source_stream 	-> array de bytes (unsigned char) fonte, dados a serem comprimidos
	dest 			-> ponteiro do arquivo destino, a ser gravado
	size 			-> quantidade de bytes a serem gravados
*/
int my_compress(byte *source_stream, FILE *dest, size_t size){


	/*
	ret will be used for zlib return codes. 
	flush will keep track of the current flushing state for deflate(), which is either no flushing,
	or flush to completion after the end of the input file is reached. 
	have is the amount of data returned from deflate(). 
	The strm structure is used to pass information to and from the zlib routines, and to maintain the deflate() state.
	*/
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char out[CHUNK];	//output buffer


		/* allocate deflate state */
	strm.zalloc = Z_NULL;		//Z_NULL to use the default memory allocation rotines 
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, COMPRESSION_LEVEL);

	if (ret != Z_OK)
		return ret;

	size_t total_bytes_writen =0;
	do{

		//se ainda tem mais de CHUNK bytes disponiveis no input para comprimir
		if( CHUNK < size-total_bytes_writen){
			strm.avail_in = CHUNK;
			flush =  Z_NO_FLUSH;
		}else{							//se for o ultimo CHUNK ( ou "semi-CHUNK") a ser escrito
			strm.avail_in = size - total_bytes_writen;
			flush =  Z_FINISH;
		}

		strm.next_in = &source_stream[total_bytes_writen];

		/* run deflate() on input until output buffer not full, finish
		   compression if all of source has been read in */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;

			ret = deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

			/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END);        /* stream will be complete */

		/* clean up and return */
	(void)deflateEnd(&strm);
	return Z_OK;
}