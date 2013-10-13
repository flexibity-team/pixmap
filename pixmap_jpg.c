// Copyright (c) 2012-2013 Bartosz Szczesny
// LICENSE: The MIT License (MIT)

#include <github.com/bszcz/pixmap/pixmap.h>
#include <github.com/bszcz/pixmap/pixmap_jpg.h>
#include <jpeglib.h>

int pixmap_write_jpg( const struct pixmap* img, const char* filename ) {
	int err = 0;

	FILE* file = fopen( filename, "wb" );
	if ( NULL == file ) {
		fprintf( stderr, "pixmap error: cannot open JPG file '%s'\n", filename );
		return 1;
	}

	j_compress_ptr jpg = calloc( 1, sizeof( struct jpeg_compress_struct ) );
	if ( NULL == jpg ) {
		fprintf( stderr, "pixmap error: cannot create JPG compress struct" );
		err = 1;
		goto fclose_jpg;
	}
	struct jpeg_error_mgr jerr;
	jpg->err = jpeg_std_error( &jerr );
	jpeg_create_compress( jpg );
	jpeg_stdio_dest( jpg, file );

	jpg->image_width = img->width;
	jpg->image_height = img->height;
	jpg->input_components = 3; // RGB
	jpg->in_color_space = JCS_RGB;
	jpeg_set_defaults( jpg );

	if ( 0 < img->quality || img->quality < 100 ) {
		jpeg_set_quality( jpg, img->quality, TRUE );
	} else {
		fprintf( stderr, "pixmap error: invalid quality value (%d), setting to default\n", img->quality );
		jpeg_set_quality( jpg, PIXMAP_GOOD_QUALITY, TRUE );
	}

	if ( PIXMAP_CHROMA_411 == img->chroma ) {
		jpg->comp_info[0].h_samp_factor = 2;
		jpg->comp_info[0].v_samp_factor = 2;
	} else if ( PIXMAP_CHROMA_422 == img->chroma ) {
		jpg->comp_info[0].h_samp_factor = 2;
		jpg->comp_info[0].v_samp_factor = 1;
	} else {
		if ( PIXMAP_CHROMA_444 != img->chroma ) {
			fprintf( stderr, "pixmap error: invalid chroma value (%d), setting to default\n", img->chroma );
		}
		jpg->comp_info[0].h_samp_factor = 1;
		jpg->comp_info[0].v_samp_factor = 1;
	}
	jpg->comp_info[1].h_samp_factor = 1;
	jpg->comp_info[1].v_samp_factor = 1;
	jpg->comp_info[2].h_samp_factor = 1;
	jpg->comp_info[2].v_samp_factor = 1;

	jpeg_start_compress( jpg, TRUE );
	JSAMPROW* rows = calloc( img->height, sizeof( JSAMPROW ) );
	if ( NULL == rows ) {
		fprintf( stderr, "pixmap error: cannot allocate memory\n" );
		err = 1;
		goto free_jpg;
	}
	for ( long h = 0; h < img->height; h++ ) {
		rows[ h ] = img->bytes + PIXMAP_COLORS*h*img->width;
	}
	if ( img->height != jpeg_write_scanlines( jpg, rows, img->height ) ) {
		fprintf( stderr, "pixmap error: cannot write scanlines\n" );
		err = 1;
	}

	free_jpg:
		jpeg_finish_compress( jpg );
		jpeg_destroy_compress( jpg );
		free( rows );
		free( jpg );
	fclose_jpg:
		if ( 0 != fclose( file ) ) {
			fprintf( stderr, "pixmap error: cannot close JPG file '%s'\n", filename );
			err = 1;
		}
	return err;
}
