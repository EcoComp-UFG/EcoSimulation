//@author: João Gabriel Silva Fernandes
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <zlib.h>
#include <string.h>


#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif



#define CHUNK 256000		//size of buffer in bytes

#define COMPRESSION_LEVEL 9	//de -1 a 9 -> -1 = Z_DEFAULT_COMPRESSION ( que é igual a 6)

int my_decompress(FILE *source, FILE *dest);


int l2b_endian(int num){
	uint32_t b0,b1,b2,b3;
	uint32_t res;

	b0 = (num & 0x000000ff) << 24u;
	b1 = (num & 0x0000ff00) << 8u;
	b2 = (num & 0x00ff0000) >> 8u;
	b3 = (num & 0xff000000) >> 24u;

	res = b0 | b1 | b2 | b3;

	return res;
}

int main(){

	FILE *source, *dest;

	source = fopen("./TestePrimes.StreamZip", "rb");	//abre o arquivo fonte( comprimido)
	dest = fopen("./saidaDecompressed.txt","wb");		//abre o arquivo destino, onde vao ser descomprimidos os dados

	my_decompress(source,dest);	//realiza a decompressão

	fclose(dest);
	fclose(source);

	/* aqui termina a decompressão*/

	/* aqui inicia a leitura do arquivo resultante, apenas para imprimir o resultado na tela*/
	source = fopen("./saidaDecompressed.txt", "rb");
	char str[6458633];			//buffer de bytes
	int size=0;					//quantidade de bytes que o arquivo tem


	//lê byte por byte do arquivo e armazena em str
	while( fread(str+size, 1, 1, source) != 0 ){
		size++;
	}
	fclose(source);


	int *numeros = (int *)str;	//converte o tipo para inteiro 32bits

	for(int i=0;i< 1020;i++){
		printf("%i  ", numeros[i] );
	}

	return 0;
}


//******** retirado de: https://zlib.net/zlib_how.html e https://zlib.net/manual.html ****************/

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int my_decompress(FILE *source, FILE *dest){
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
    /* decompress until deflate stream ends or end of file */
    do {
    	strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
         /* run inflate() on input until output buffer not full */
        do {
        	strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
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
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
         /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}



int my_compress(FILE *source, FILE *dest){


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
    unsigned char in[CHUNK];	//input buffer
    unsigned char out[CHUNK];	//output buffer


        /* allocate deflate state */
    strm.zalloc = Z_NULL;		//Z_NULL to use the default memory allocation rotines 
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, COMPRESSION_LEVEL);
    if (ret != Z_OK)
        return ret;


	do{

		strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

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